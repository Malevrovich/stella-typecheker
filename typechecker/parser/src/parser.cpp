#include "stella/parser.hpp"

#include <ANTLRFileStream.h>
#include <ANTLRInputStream.h>
#include <any>
#include <format>

#include <antlr4-runtime.h>
#include <loguru.hpp>
#include <misc/Interval.h>
#include <support/Any.h>

#include "stella/ast/ast.hpp"

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
        DLOG_F(ERROR, "Bad any_cast in AST builder. Expected %s, got %s at: %s", typeid(T).name(),
               operand.type().name(), loguru::stacktrace().c_str());
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

    antlrcpp::Any visitConstTrue(antlr4_stella::StellaParser::ConstTrueContext* ctx) override {
        return make_expr<ast::NodeExprConstTrue>(ctx);
    }

    antlrcpp::Any visitConstFalse(antlr4_stella::StellaParser::ConstFalseContext* ctx) override {
        return make_expr<ast::NodeExprConstFalse>(ctx);
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