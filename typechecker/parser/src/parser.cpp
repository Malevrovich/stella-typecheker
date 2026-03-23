#include "stella/parser.hpp"

#include <any>
#include <format>
#include <unordered_set>

#include <ANTLRFileStream.h>
#include <ANTLRInputStream.h>
#include <antlr4-runtime.h>

#include <loguru.hpp>

#include "stella/ast/asc.hpp"
#include "stella/ast/ast.hpp"
#include "stella/ast/let.hpp"
#include "stella/ast/list.hpp"
#include "stella/ast/record.hpp"
#include "stella/ast/tuple.hpp"
#include "stella/utils.hpp"

#include "StellaLexer.h"
#include "StellaParser.h"
#include "StellaParserBaseVisitor.h"

namespace stella {

namespace {

class CharStreamSourceInfo : public ast::SourceInfo {
public:
    CharStreamSourceInfo(std::shared_ptr<antlr4::CharStream> stream, antlr4::ParserRuleContext* ctx)
        : stream_{std::move(stream)},
          start_line_{ctx->getStart()->getLine()},
          start_column_{ctx->getStart()->getCharPositionInLine()},
          interval_{ctx->getStart()->getStartIndex(), ctx->getStop()->getStopIndex()} {}

    std::string GetLocation() const override {
        return std::format("{}:{}:{}", stream_->getSourceName(), start_line_, start_column_);
    }

    std::string ToString() const override { return stream_->getText(interval_); }

private:
    std::shared_ptr<antlr4::CharStream> stream_;
    std::size_t start_line_;
    std::size_t start_column_;
    antlr4::misc::Interval interval_;
};

std::shared_ptr<ast::SourceInfo> GetSourceInfo(antlr4::ParserRuleContext* ctx,
                                               std::shared_ptr<antlr4::CharStream> stream) {
    if (!ctx) {
        return nullptr;
    }

    return std::make_shared<CharStreamSourceInfo>(std::move(stream), ctx);
}

template <typename T>
auto try_any_cast(const std::any& operand) -> T {
    try {
        return std::any_cast<T>(operand);
    } catch (const std::bad_any_cast&) {
        DLOG_F(ERROR, "Bad any_cast in AST builder. Expected %s, got %s at: %s",
               tryDemangle(typeid(T).name()).c_str(), tryDemangle(operand.type().name()).c_str(),
               loguru::stacktrace().c_str());
        throw std::runtime_error("Unexpected type in AST builder");
    }
}

class ASTBuilder : public antlr4_stella::StellaParserBaseVisitor {
public:
    ASTBuilder(std::shared_ptr<antlr4::CharStream> stream)
        : stream_(std::move(stream)) {}

private:
    std::shared_ptr<antlr4::CharStream> stream_;

    template <typename T>
    std::shared_ptr<const ast::Type> type(const std::shared_ptr<T>& t) {
        return std::static_pointer_cast<const ast::Type>(t);
    }

    template <typename T, typename... Args>
    std::shared_ptr<const T> make_node(antlr4::ParserRuleContext* ctx, Args&&... args) {
        return std::make_shared<T>(GetSourceInfo(ctx, stream_), std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    std::shared_ptr<const ast::NodeExpr> make_expr(antlr4::ParserRuleContext* ctx, Args&&... args) {
        return std::make_shared<T>(GetSourceInfo(ctx, stream_), std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    std::shared_ptr<const ast::NodeDecl> make_decl(antlr4::ParserRuleContext* ctx, Args&&... args) {
        return std::make_shared<T>(GetSourceInfo(ctx, stream_), std::forward<Args>(args)...);
    }

    antlrcpp::Any
    visitStart_Program(antlr4_stella::StellaParser::Start_ProgramContext* ctx) override {
        return visit(ctx->program());
    }

    antlrcpp::Any visitProgram(antlr4_stella::StellaParser::ProgramContext* ctx) override {
        std::vector<std::shared_ptr<const ast::NodeDecl>> decls;
        for (auto decl_ctx : ctx->decl()) {
            decls.push_back(try_any_cast<std::shared_ptr<const ast::NodeDecl>>(visit(decl_ctx)));
        }

        return make_node<ast::NodeProgram>(ctx, std::move(decls));
    }

    antlrcpp::Any visitTypeNat(antlr4_stella::StellaParser::TypeNatContext* ctx) override {
        return type(std::make_shared<const ast::TypeNat>());
    }

    antlrcpp::Any visitTypeBool(antlr4_stella::StellaParser::TypeBoolContext* ctx) override {
        return type(std::make_shared<const ast::TypeBool>());
    }

    antlrcpp::Any visitTypeFun(antlr4_stella::StellaParser::TypeFunContext* ctx) override {
        if (ctx->paramTypes.size() > 1) {
            throw std::runtime_error(
                "Multiple parameter types in function type are not supported yet");
        }

        auto param_type = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->paramTypes[0]));
        auto return_type = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->returnType));

        return type(std::make_shared<const ast::TypeFun>(param_type, return_type));
    }

    antlrcpp::Any visitTypeUnit(antlr4_stella::StellaParser::TypeUnitContext* ctx) override {
        return type(std::make_shared<const ast::TypeUnit>());
    }

    antlrcpp::Any visitConstTrue(antlr4_stella::StellaParser::ConstTrueContext* ctx) override {
        return make_expr<ast::NodeExprConstTrue>(ctx);
    }

    antlrcpp::Any visitConstFalse(antlr4_stella::StellaParser::ConstFalseContext* ctx) override {
        return make_expr<ast::NodeExprConstFalse>(ctx);
    }

    antlrcpp::Any visitConstUnit(antlr4_stella::StellaParser::ConstUnitContext* ctx) override {
        return make_expr<ast::NodeExprConstUnit>(ctx);
    }

    antlrcpp::Any visitConstInt(antlr4_stella::StellaParser::ConstIntContext* ctx) override {
        int value = std::stoi(ctx->n->getText());
        return make_expr<ast::NodeExprConstInt>(ctx, value);
    }

    antlrcpp::Any visitVar(antlr4_stella::StellaParser::VarContext* ctx) override {
        return make_expr<ast::NodeExprVar>(ctx, ctx->name->getText());
    }

    antlrcpp::Any visitIf(antlr4_stella::StellaParser::IfContext* ctx) override {
        auto condition = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->condition));
        auto then_expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->thenExpr));
        auto else_expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->elseExpr));

        return make_expr<ast::NodeExprIf>(ctx, condition, then_expr, else_expr);
    }

    antlrcpp::Any visitSucc(antlr4_stella::StellaParser::SuccContext* ctx) override {
        auto operand = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->n));
        return make_expr<ast::NodeExprSucc>(ctx, operand);
    }

    antlrcpp::Any visitPred(antlr4_stella::StellaParser::PredContext* ctx) override {
        auto operand = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->n));
        return make_expr<ast::NodeExprPred>(ctx, operand);
    }

    antlrcpp::Any visitIsZero(antlr4_stella::StellaParser::IsZeroContext* ctx) override {
        auto operand = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->n));
        return make_expr<ast::NodeExprIsZero>(ctx, operand);
    }

    antlrcpp::Any visitFix(antlr4_stella::StellaParser::FixContext* ctx) override {
        auto expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->expr_));
        return make_expr<ast::NodeExprFix>(ctx, expr);
    }

    antlrcpp::Any visitNatRec(antlr4_stella::StellaParser::NatRecContext* ctx) override {
        auto n = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->n));
        auto initial = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->initial));
        auto step = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->step));
        return make_expr<ast::NodeExprNatRec>(ctx, n, initial, step);
    }

    antlrcpp::Any visitApplication(antlr4_stella::StellaParser::ApplicationContext* ctx) override {
        if (ctx->args.size() > 1) {
            throw std::runtime_error(
                "Multiple arguments in function application are not supported yet");
        }

        auto fun = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->fun));
        auto arg = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->args[0]));

        return make_expr<ast::NodeExprApplication>(ctx, fun, arg);
    }

    antlrcpp::Any visitAbstraction(antlr4_stella::StellaParser::AbstractionContext* ctx) override {
        if (ctx->paramDecls.size() > 1) {
            throw std::runtime_error(
                "Multiple parameters in function abstraction are not supported yet");
        }

        auto param =
            try_any_cast<std::shared_ptr<const ast::NodeParamDecl>>(visit(ctx->paramDecls[0]));
        auto body = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->returnExpr));

        return make_expr<ast::NodeExprAbstraction>(ctx, param, body);
    }

    antlrcpp::Any
    visitParenthesisedExpr(antlr4_stella::StellaParser::ParenthesisedExprContext* ctx) override {
        return visit(ctx->expr_);
    }

    std::any visitParamDecl(antlr4_stella::StellaParser::ParamDeclContext* ctx) override {
        auto type = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->paramType));
        return make_node<ast::NodeParamDecl>(ctx, ctx->name->getText(), type);
    }

    antlrcpp::Any visitDeclFun(antlr4_stella::StellaParser::DeclFunContext* ctx) override {
        if (ctx->paramDecls.size() > 1) {
            throw std::runtime_error(
                "Multiple parameters in function abstraction are not supported yet");
        }

        auto param =
            try_any_cast<std::shared_ptr<const ast::NodeParamDecl>>(visit(ctx->paramDecls[0]));
        auto return_type = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->returnType));
        auto body = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->returnExpr));

        auto abstr = make_node<ast::NodeExprAbstraction>(ctx, param, body);

        return make_decl<ast::NodeDeclFun>(ctx, ctx->name->getText(), return_type, abstr);
    }

    antlrcpp::Any visitTypeList(antlr4_stella::StellaParser::TypeListContext* ctx) override {
        auto element_type = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->type_));
        return type(std::make_shared<const ast::TypeList>(element_type));
    }

    antlrcpp::Any visitList(antlr4_stella::StellaParser::ListContext* ctx) override {
        std::vector<std::shared_ptr<const ast::NodeExpr>> elements;
        for (auto expr_ctx : ctx->exprs) {
            elements.push_back(try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(expr_ctx)));
        }
        return make_expr<ast::NodeExprList>(ctx, std::move(elements));
    }

    antlrcpp::Any visitConsList(antlr4_stella::StellaParser::ConsListContext* ctx) override {
        auto head = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->head));
        auto tail = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->tail));
        return make_expr<ast::NodeExprConsList>(ctx, head, tail);
    }

    antlrcpp::Any visitHead(antlr4_stella::StellaParser::HeadContext* ctx) override {
        auto list = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->list));
        return make_expr<ast::NodeExprHead>(ctx, list);
    }

    antlrcpp::Any visitTail(antlr4_stella::StellaParser::TailContext* ctx) override {
        auto list = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->list));
        return make_expr<ast::NodeExprTail>(ctx, list);
    }

    antlrcpp::Any visitIsEmpty(antlr4_stella::StellaParser::IsEmptyContext* ctx) override {
        auto list = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->list));
        return make_expr<ast::NodeExprIsEmpty>(ctx, list);
    }

    antlrcpp::Any visitTypeTuple(antlr4_stella::StellaParser::TypeTupleContext* ctx) override {
        std::vector<std::shared_ptr<const ast::Type>> element_types;
        for (auto type_ctx : ctx->types) {
            element_types.push_back(
                try_any_cast<std::shared_ptr<const ast::Type>>(visit(type_ctx)));
        }
        return type(std::make_shared<const ast::TypeTuple>(std::move(element_types)));
    }

    antlrcpp::Any visitTuple(antlr4_stella::StellaParser::TupleContext* ctx) override {
        std::vector<std::shared_ptr<const ast::NodeExpr>> elements;
        for (auto expr_ctx : ctx->exprs) {
            elements.push_back(try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(expr_ctx)));
        }
        return make_expr<ast::NodeExprTuple>(ctx, std::move(elements));
    }

    antlrcpp::Any visitDotTuple(antlr4_stella::StellaParser::DotTupleContext* ctx) override {
        auto expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->expr_));
        int index = std::stoi(ctx->index->getText());
        return make_expr<ast::NodeExprDotTuple>(ctx, expr, index);
    }

    antlrcpp::Any visitTypeRecord(antlr4_stella::StellaParser::TypeRecordContext* ctx) override {
        std::vector<ast::TypeRecord::Field> fields;
        std::unordered_set<std::string> seen_labels;
        std::optional<std::string> duplicate_label;
        for (auto field_ctx : ctx->fieldTypes) {
            const std::string label = field_ctx->label->getText();
            if (!seen_labels.insert(label).second && !duplicate_label) {
                duplicate_label = label;
            }
            auto field_type =
                try_any_cast<std::shared_ptr<const ast::Type>>(visit(field_ctx->type_));
            fields.push_back({label, std::move(field_type)});
        }
        return type(
            std::make_shared<const ast::TypeRecord>(std::move(fields), std::move(duplicate_label)));
    }

    antlrcpp::Any visitRecord(antlr4_stella::StellaParser::RecordContext* ctx) override {
        std::vector<ast::NodeExprRecord::Field> fields;
        for (auto binding_ctx : ctx->bindings) {
            auto expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(binding_ctx->rhs));
            fields.push_back({binding_ctx->name->getText(), std::move(expr)});
        }
        return make_expr<ast::NodeExprRecord>(ctx, std::move(fields));
    }

    antlrcpp::Any visitDotRecord(antlr4_stella::StellaParser::DotRecordContext* ctx) override {
        auto expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->expr_));
        return make_expr<ast::NodeExprDotRecord>(ctx, expr, ctx->label->getText());
    }

    antlrcpp::Any visitTypeAsc(antlr4_stella::StellaParser::TypeAscContext* ctx) override {
        auto expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->expr_));
        auto asc_type = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->type_));

        return make_expr<ast::NodeExprTypeAsc>(ctx, expr, asc_type);
    }

    antlrcpp::Any visitPatternVar(antlr4_stella::StellaParser::PatternVarContext* ctx) override {
        return make_node<ast::NodePatternVar>(ctx, ctx->name->getText());
    }

    antlrcpp::Any visitLet(antlr4_stella::StellaParser::LetContext* ctx) override {
        if (ctx->patternBindings.size() > 1) {
            throw std::runtime_error("Multiple pattern bindings in let are not supported yet");
        }

        auto binding = ctx->patternBindings[0];
        auto pattern =
            try_any_cast<std::shared_ptr<const ast::NodePatternVar>>(visit(binding->pat));
        auto init = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(binding->rhs));
        auto body = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->body));

        return make_expr<ast::NodeExprLet>(ctx, pattern, init, body);
    }

    std::any visitTerminatingSemicolon(
        antlr4_stella::StellaParser::TerminatingSemicolonContext* ctx) override {
        return visit(ctx->expr_);
    }

    antlrcpp::Any visitChildren(antlr4::tree::ParseTree* node) override {
        DLOG_S(FATAL) << "Parsing error occured at node: " << node->getText();
        throw std::runtime_error("Unexpected node in AST builder: " + node->getText());
    }
};

std::shared_ptr<const ast::NodeProgram>
ParseProgram(std::shared_ptr<antlr4::ANTLRInputStream> stream) {
    antlr4_stella::StellaLexer lexer(stream.get());

    antlr4::CommonTokenStream tokens(&lexer);

    antlr4_stella::StellaParser parser(&tokens);

    antlr4_stella::StellaParser::Start_ProgramContext* tree = parser.start_Program();

    ASTBuilder builder{stream};
    auto result = builder.visit(tree);

    return try_any_cast<std::shared_ptr<const ast::NodeProgram>>(result);
}

} // namespace

std::shared_ptr<const ast::NodeProgram> ParseProgramText(std::string_view input) {
    return ParseProgram(std::make_shared<antlr4::ANTLRInputStream>(input));
}

std::shared_ptr<const ast::NodeProgram> ParseProgramFile(const std::string& filename) {
    auto file_stream = std::make_shared<antlr4::ANTLRFileStream>();
    file_stream->loadFromFile(filename);
    return ParseProgram(file_stream);
}

} // namespace stella