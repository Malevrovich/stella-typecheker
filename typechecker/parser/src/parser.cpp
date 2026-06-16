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
#include "stella/ast/auto.hpp"
#include "stella/ast/cast.hpp"
#include "stella/ast/exception.hpp"
#include "stella/ast/generic.hpp"
#include "stella/ast/let.hpp"
#include "stella/ast/list.hpp"
#include "stella/ast/panic.hpp"
#include "stella/ast/record.hpp"
#include "stella/ast/reference.hpp"
#include "stella/ast/sequence.hpp"
#include "stella/ast/sum.hpp"
#include "stella/ast/top_bottom.hpp"
#include "stella/ast/tuple.hpp"
#include "stella/ast/variant.hpp"
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
        std::unordered_set<std::string> extensions;
        for (auto ext_ctx : ctx->extensions) {
            auto* an_ext = dynamic_cast<antlr4_stella::StellaParser::AnExtensionContext*>(ext_ctx);
            if (an_ext) {
                for (auto* name_node : an_ext->ExtensionName()) {
                    extensions.insert(name_node->getText());
                }
            }
        }

        std::vector<std::shared_ptr<const ast::NodeDecl>> decls;
        for (auto decl_ctx : ctx->decl()) {
            decls.push_back(try_any_cast<std::shared_ptr<const ast::NodeDecl>>(visit(decl_ctx)));
        }

        return make_node<ast::NodeProgram>(ctx, std::move(decls), std::move(extensions));
    }

    antlrcpp::Any visitTypeNat(antlr4_stella::StellaParser::TypeNatContext* ctx) override {
        return type(std::make_shared<const ast::TypeNat>(nullptr, GetSourceInfo(ctx, stream_)));
    }

    antlrcpp::Any visitTypeBool(antlr4_stella::StellaParser::TypeBoolContext* ctx) override {
        return type(std::make_shared<const ast::TypeBool>(nullptr, GetSourceInfo(ctx, stream_)));
    }

    antlrcpp::Any visitTypeFun(antlr4_stella::StellaParser::TypeFunContext* ctx) override {
        if (ctx->paramTypes.size() > 1) {
            throw std::runtime_error(
                "Multiple parameter types in function type are not supported yet");
        }

        auto param_type = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->paramTypes[0]));
        auto return_type = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->returnType));

        return type(std::make_shared<const ast::TypeFun>(param_type, return_type, nullptr, GetSourceInfo(ctx, stream_)));
    }

    antlrcpp::Any visitTypeForAll(antlr4_stella::StellaParser::TypeForAllContext* ctx) override {
        std::vector<std::string> type_params;
        for (auto* ident : ctx->types) {
            type_params.push_back(ident->getText());
        }

        auto body = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->type_));

        return type(std::make_shared<const ast::TypeForAll>(std::move(type_params), body, nullptr, GetSourceInfo(ctx, stream_)));
    }

    antlrcpp::Any visitTypeUnit(antlr4_stella::StellaParser::TypeUnitContext* ctx) override {
        return type(std::make_shared<const ast::TypeUnit>(nullptr, GetSourceInfo(ctx, stream_)));
    }

    antlrcpp::Any visitTypeTop(antlr4_stella::StellaParser::TypeTopContext* ctx) override {
        return type(std::make_shared<const ast::TypeTop>(nullptr, GetSourceInfo(ctx, stream_)));
    }

    antlrcpp::Any visitTypeBottom(antlr4_stella::StellaParser::TypeBottomContext* ctx) override {
        return type(std::make_shared<const ast::TypeBottom>(nullptr, GetSourceInfo(ctx, stream_)));
    }

    antlrcpp::Any visitTypeAuto(antlr4_stella::StellaParser::TypeAutoContext* ctx) override {
        return type(std::make_shared<const ast::TypeAuto>(nullptr, GetSourceInfo(ctx, stream_)));
    }

    antlrcpp::Any visitTypeVar(antlr4_stella::StellaParser::TypeVarContext* ctx) override {
        return type(std::make_shared<const ast::TypeVar>(ctx->name->getText(), nullptr, GetSourceInfo(ctx, stream_)));
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
    visitTypeAbstraction(antlr4_stella::StellaParser::TypeAbstractionContext* ctx) override {
        std::vector<std::string> type_params;
        for (auto* ident : ctx->generics) {
            type_params.push_back(ident->getText());
        }

        auto body = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->expr_));

        return make_expr<ast::NodeExprTypeAbstraction>(ctx, std::move(type_params), body);
    }

    antlrcpp::Any
    visitTypeApplication(antlr4_stella::StellaParser::TypeApplicationContext* ctx) override {
        auto function = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->fun));

        std::vector<std::shared_ptr<const ast::Type>> type_args;
        for (auto* type_ctx : ctx->types) {
            type_args.push_back(try_any_cast<std::shared_ptr<const ast::Type>>(visit(type_ctx)));
        }

        return make_expr<ast::NodeExprTypeApplication>(ctx, function, std::move(type_args));
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

        std::vector<std::shared_ptr<const ast::NodeDecl>> local_decls;
        for (auto* local_ctx : ctx->localDecls) {
            local_decls.push_back(
                try_any_cast<std::shared_ptr<const ast::NodeDecl>>(visit(local_ctx)));
        }

        auto abstr = make_node<ast::NodeExprAbstraction>(ctx, param, body);

        return make_decl<ast::NodeDeclFun>(ctx, ctx->name->getText(), return_type, abstr,
                                           std::move(local_decls));
    }

    antlrcpp::Any
    visitDeclFunGeneric(antlr4_stella::StellaParser::DeclFunGenericContext* ctx) override {
        if (ctx->paramDecls.size() > 1) {
            throw std::runtime_error(
                "Multiple parameters in function abstraction are not supported yet");
        }

        std::vector<std::string> type_params;
        for (auto* ident : ctx->generics) {
            type_params.push_back(ident->getText());
        }

        auto param =
            try_any_cast<std::shared_ptr<const ast::NodeParamDecl>>(visit(ctx->paramDecls[0]));
        auto return_type = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->returnType));
        auto body = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->returnExpr));

        std::vector<std::shared_ptr<const ast::NodeDecl>> local_decls;
        for (auto* local_ctx : ctx->localDecls) {
            local_decls.push_back(
                try_any_cast<std::shared_ptr<const ast::NodeDecl>>(visit(local_ctx)));
        }

        auto abstr = make_node<ast::NodeExprAbstraction>(ctx, param, body);

        return make_decl<ast::NodeDeclFunGeneric>(ctx, ctx->name->getText(), std::move(type_params),
                                                  return_type, abstr, std::move(local_decls));
    }

    antlrcpp::Any visitTypeList(antlr4_stella::StellaParser::TypeListContext* ctx) override {
        auto element_type = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->type_));
        return type(std::make_shared<const ast::TypeList>(element_type, nullptr, GetSourceInfo(ctx, stream_)));
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
        return type(std::make_shared<const ast::TypeTuple>(std::move(element_types), nullptr, GetSourceInfo(ctx, stream_)));
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
            std::make_shared<const ast::TypeRecord>(std::move(fields), std::move(duplicate_label), nullptr, GetSourceInfo(ctx, stream_)));
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

    antlrcpp::Any visitTypeParens(antlr4_stella::StellaParser::TypeParensContext* ctx) override {
        return visit(ctx->type_);
    }

    antlrcpp::Any visitTypeSum(antlr4_stella::StellaParser::TypeSumContext* ctx) override {
        auto left = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->left));
        auto right = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->right));
        return type(std::make_shared<const ast::TypeSum>(left, right, nullptr, GetSourceInfo(ctx, stream_)));
    }

    antlrcpp::Any visitInl(antlr4_stella::StellaParser::InlContext* ctx) override {
        auto expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->expr_));
        return make_expr<ast::NodeExprInl>(ctx, expr);
    }

    antlrcpp::Any visitInr(antlr4_stella::StellaParser::InrContext* ctx) override {
        auto expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->expr_));
        return make_expr<ast::NodeExprInr>(ctx, expr);
    }

    antlrcpp::Any visitPatternInl(antlr4_stella::StellaParser::PatternInlContext* ctx) override {
        auto pattern = try_any_cast<std::shared_ptr<const ast::NodePattern>>(visit(ctx->pattern_));
        return std::static_pointer_cast<const ast::NodePattern>(
            make_node<ast::NodePatternInl>(ctx, pattern));
    }

    antlrcpp::Any visitPatternInr(antlr4_stella::StellaParser::PatternInrContext* ctx) override {
        auto pattern = try_any_cast<std::shared_ptr<const ast::NodePattern>>(visit(ctx->pattern_));
        return std::static_pointer_cast<const ast::NodePattern>(
            make_node<ast::NodePatternInr>(ctx, pattern));
    }

    antlrcpp::Any visitMatch(antlr4_stella::StellaParser::MatchContext* ctx) override {
        auto expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->expr_));

        std::vector<std::shared_ptr<const ast::NodeMatchCase>> cases;
        for (auto case_ctx : ctx->cases) {
            auto pattern =
                try_any_cast<std::shared_ptr<const ast::NodePattern>>(visit(case_ctx->pattern_));
            auto case_expr =
                try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(case_ctx->expr_));
            cases.push_back(make_node<ast::NodeMatchCase>(case_ctx, pattern, case_expr));
        }

        return make_expr<ast::NodeExprMatch>(ctx, expr, std::move(cases));
    }

    antlrcpp::Any visitTypeVariant(antlr4_stella::StellaParser::TypeVariantContext* ctx) override {
        std::vector<ast::TypeVariant::Field> fields;
        std::unordered_set<std::string> seen_labels;
        std::optional<std::string> duplicate_label;
        for (auto field_ctx : ctx->fieldTypes) {
            const std::string label = field_ctx->label->getText();
            if (!seen_labels.insert(label).second && !duplicate_label) {
                duplicate_label = label;
            }
            std::optional<std::shared_ptr<const ast::Type>> field_type;
            if (field_ctx->type_) {
                field_type =
                    try_any_cast<std::shared_ptr<const ast::Type>>(visit(field_ctx->type_));
            }
            fields.push_back({label, std::move(field_type)});
        }
        return type(std::make_shared<const ast::TypeVariant>(std::move(fields),
                                                             std::move(duplicate_label), nullptr, GetSourceInfo(ctx, stream_)));
    }

    antlrcpp::Any visitVariant(antlr4_stella::StellaParser::VariantContext* ctx) override {
        std::optional<std::shared_ptr<const ast::NodeExpr>> expr;
        if (ctx->rhs) {
            expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->rhs));
        }
        return make_expr<ast::NodeExprVariant>(ctx, ctx->label->getText(), std::move(expr));
    }

    antlrcpp::Any
    visitPatternVariant(antlr4_stella::StellaParser::PatternVariantContext* ctx) override {
        std::optional<std::shared_ptr<const ast::NodePattern>> pattern;
        if (ctx->pattern_) {
            pattern = try_any_cast<std::shared_ptr<const ast::NodePattern>>(visit(ctx->pattern_));
        }
        return std::static_pointer_cast<const ast::NodePattern>(
            make_node<ast::NodePatternVariant>(ctx, ctx->label->getText(), std::move(pattern)));
    }

    antlrcpp::Any visitTypeAsc(antlr4_stella::StellaParser::TypeAscContext* ctx) override {
        auto expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->expr_));
        auto asc_type = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->type_));

        return make_expr<ast::NodeExprTypeAsc>(ctx, expr, asc_type);
    }

    antlrcpp::Any visitTypeCast(antlr4_stella::StellaParser::TypeCastContext* ctx) override {
        auto expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->expr_));
        auto cast_type = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->type_));

        return make_expr<ast::NodeExprTypeCast>(ctx, expr, cast_type);
    }

    antlrcpp::Any visitPatternVar(antlr4_stella::StellaParser::PatternVarContext* ctx) override {
        return std::static_pointer_cast<const ast::NodePattern>(
            make_node<ast::NodePatternVar>(ctx, ctx->name->getText()));
    }

    antlrcpp::Any
    visitPatternCastAs(antlr4_stella::StellaParser::PatternCastAsContext* ctx) override {
        auto inner_pattern =
            try_any_cast<std::shared_ptr<const ast::NodePattern>>(visit(ctx->pattern_));
        auto cast_type = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->type_));
        return std::static_pointer_cast<const ast::NodePattern>(
            make_node<ast::NodePatternCastAs>(ctx, inner_pattern, cast_type));
    }

    antlrcpp::Any visitLet(antlr4_stella::StellaParser::LetContext* ctx) override {
        if (ctx->patternBindings.size() > 1) {
            throw std::runtime_error("Multiple pattern bindings in let are not supported yet");
        }

        auto binding = ctx->patternBindings[0];
        auto pattern_base =
            try_any_cast<std::shared_ptr<const ast::NodePattern>>(visit(binding->pat));
        auto pattern = std::dynamic_pointer_cast<const ast::NodePatternVar>(pattern_base);
        if (!pattern) {
            throw std::runtime_error("Only PatternVar is supported in let bindings");
        }
        auto init = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(binding->rhs));
        auto body = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->body));

        return make_expr<ast::NodeExprLet>(ctx, pattern, init, body);
    }

    antlrcpp::Any visitPanic(antlr4_stella::StellaParser::PanicContext* ctx) override {
        return make_expr<ast::NodeExprPanic>(ctx);
    }

    antlrcpp::Any visitSequence(antlr4_stella::StellaParser::SequenceContext* ctx) override {
        auto expr1 = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->expr1));
        auto expr2 = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->expr2));
        return make_expr<ast::NodeExprSequence>(ctx, expr1, expr2);
    }

    antlrcpp::Any
    visitDeclExceptionType(antlr4_stella::StellaParser::DeclExceptionTypeContext* ctx) override {
        auto exception_type =
            try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->exceptionType));
        return make_decl<ast::NodeDeclExceptionType>(ctx, exception_type);
    }

    antlrcpp::Any visitDeclExceptionVariant(
        antlr4_stella::StellaParser::DeclExceptionVariantContext* ctx) override {
        auto variant_type = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->variantType));
        return make_decl<ast::NodeDeclExceptionVariant>(ctx, ctx->name->getText(), variant_type);
    }

    antlrcpp::Any visitThrow(antlr4_stella::StellaParser::ThrowContext* ctx) override {
        auto expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->expr_));
        return make_expr<ast::NodeExprThrow>(ctx, expr);
    }

    antlrcpp::Any visitTryWith(antlr4_stella::StellaParser::TryWithContext* ctx) override {
        auto try_expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->tryExpr));
        auto fallback_expr =
            try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->fallbackExpr));
        return make_expr<ast::NodeExprTryWith>(ctx, try_expr, fallback_expr);
    }

    antlrcpp::Any visitTryCatch(antlr4_stella::StellaParser::TryCatchContext* ctx) override {
        auto try_expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->tryExpr));
        auto pattern = try_any_cast<std::shared_ptr<const ast::NodePattern>>(visit(ctx->pat));
        auto fallback_expr =
            try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->fallbackExpr));
        return make_expr<ast::NodeExprTryCatch>(ctx, try_expr, pattern, fallback_expr);
    }

    antlrcpp::Any visitTryCastAs(antlr4_stella::StellaParser::TryCastAsContext* ctx) override {
        auto try_expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->tryExpr));
        auto cast_type = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->type_));
        auto pattern = try_any_cast<std::shared_ptr<const ast::NodePattern>>(visit(ctx->pattern_));
        auto success_expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->expr_));
        auto fallback_expr =
            try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->fallbackExpr));
        return make_expr<ast::NodeExprTryCastAs>(ctx, try_expr, cast_type, pattern, success_expr,
                                                 fallback_expr);
    }

    antlrcpp::Any visitTypeRef(antlr4_stella::StellaParser::TypeRefContext* ctx) override {
        auto inner_type = try_any_cast<std::shared_ptr<const ast::Type>>(visit(ctx->type_));
        return type(std::make_shared<const ast::TypeRef>(inner_type, nullptr, GetSourceInfo(ctx, stream_)));
    }

    antlrcpp::Any visitRef(antlr4_stella::StellaParser::RefContext* ctx) override {
        auto expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->expr_));
        return make_expr<ast::NodeExprRef>(ctx, expr);
    }

    antlrcpp::Any visitDeref(antlr4_stella::StellaParser::DerefContext* ctx) override {
        auto expr = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->expr_));
        return make_expr<ast::NodeExprDeref>(ctx, expr);
    }

    antlrcpp::Any visitAssign(antlr4_stella::StellaParser::AssignContext* ctx) override {
        auto lhs = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->lhs));
        auto rhs = try_any_cast<std::shared_ptr<const ast::NodeExpr>>(visit(ctx->rhs));
        return make_expr<ast::NodeExprAssign>(ctx, lhs, rhs);
    }

    antlrcpp::Any visitConstMemory(antlr4_stella::StellaParser::ConstMemoryContext* ctx) override {
        return make_expr<ast::NodeExprConstMemory>(ctx, ctx->mem->getText());
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