#include "stella/parser.hpp"

#include <any>

#include <antlr4-runtime.h>
#include <loguru.hpp>

#include "stella/ast.hpp"

#include "StellaLexer.h"
#include "StellaParser.h"
#include "StellaParserBaseVisitor.h"
#include "stella/base.hpp"

namespace {

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

template <typename T>
std::shared_ptr<stella::ast::Type> type(const std::shared_ptr<T>& t) {
    return std::static_pointer_cast<stella::ast::Type>(t);
}

template <typename T>
std::shared_ptr<stella::ast::NodeExpr> expr(const std::shared_ptr<T>& t) {
    return std::static_pointer_cast<stella::ast::NodeExpr>(t);
}

template <typename T>
std::shared_ptr<stella::ast::NodeDecl> decl(const std::shared_ptr<T>& t) {
    return std::static_pointer_cast<stella::ast::NodeDecl>(t);
}

class ASTBuilder : public antlr4_stella::StellaParserBaseVisitor {
public:
    antlrcpp::Any
    visitStart_Program(antlr4_stella::StellaParser::Start_ProgramContext* ctx) override {
        return visit(ctx->program());
    }

    antlrcpp::Any visitProgram(antlr4_stella::StellaParser::ProgramContext* ctx) override {
        std::vector<std::shared_ptr<stella::ast::NodeDecl>> decls;
        for (auto decl_ctx : ctx->decl()) {
            decls.push_back(try_any_cast<std::shared_ptr<stella::ast::NodeDecl>>(visit(decl_ctx)));
        }

        return std::make_shared<stella::ast::NodeProgram>(std::move(decls));
    }

    antlrcpp::Any visitTypeNat(antlr4_stella::StellaParser::TypeNatContext* ctx) override {
        return type(std::make_shared<stella::ast::TypeNat>());
    }

    antlrcpp::Any visitTypeBool(antlr4_stella::StellaParser::TypeBoolContext* ctx) override {
        return type(std::make_shared<stella::ast::TypeBool>());
    }

    antlrcpp::Any visitTypeFun(antlr4_stella::StellaParser::TypeFunContext* ctx) override {
        if (ctx->paramTypes.size() > 1) {
            throw std::runtime_error(
                "Multiple parameter types in function type are not supported yet");
        }

        auto param_type =
            try_any_cast<std::shared_ptr<stella::ast::Type>>(visit(ctx->paramTypes[0]));
        auto return_type = try_any_cast<std::shared_ptr<stella::ast::Type>>(visit(ctx->returnType));

        return type(std::make_shared<stella::ast::TypeFun>(param_type, return_type));
    }

    antlrcpp::Any visitConstTrue(antlr4_stella::StellaParser::ConstTrueContext* ctx) override {
        return expr(std::make_shared<stella::ast::NodeExprConstTrue>());
    }

    antlrcpp::Any visitConstFalse(antlr4_stella::StellaParser::ConstFalseContext* ctx) override {
        return expr(std::make_shared<stella::ast::NodeExprConstFalse>());
    }

    antlrcpp::Any visitConstInt(antlr4_stella::StellaParser::ConstIntContext* ctx) override {
        int value = std::stoi(ctx->n->getText());
        return expr(std::make_shared<stella::ast::NodeExprConstInt>(value));
    }

    antlrcpp::Any visitVar(antlr4_stella::StellaParser::VarContext* ctx) override {
        return expr(std::make_shared<stella::ast::NodeExprVar>(ctx->name->getText()));
    }

    antlrcpp::Any visitIf(antlr4_stella::StellaParser::IfContext* ctx) override {
        auto condition =
            try_any_cast<std::shared_ptr<stella::ast::NodeExpr>>(visit(ctx->condition));
        auto then_expr = try_any_cast<std::shared_ptr<stella::ast::NodeExpr>>(visit(ctx->thenExpr));
        auto else_expr = try_any_cast<std::shared_ptr<stella::ast::NodeExpr>>(visit(ctx->elseExpr));

        return expr(std::make_shared<stella::ast::NodeExprIf>(condition, then_expr, else_expr));
    }

    antlrcpp::Any visitSucc(antlr4_stella::StellaParser::SuccContext* ctx) override {
        auto operand = try_any_cast<std::shared_ptr<stella::ast::NodeExpr>>(visit(ctx->n));
        return expr(std::make_shared<stella::ast::NodeExprSucc>(operand));
    }

    antlrcpp::Any visitPred(antlr4_stella::StellaParser::PredContext* ctx) override {
        auto operand = try_any_cast<std::shared_ptr<stella::ast::NodeExpr>>(visit(ctx->n));
        return expr(std::make_shared<stella::ast::NodeExprPred>(operand));
    }

    antlrcpp::Any visitIsZero(antlr4_stella::StellaParser::IsZeroContext* ctx) override {
        auto operand = try_any_cast<std::shared_ptr<stella::ast::NodeExpr>>(visit(ctx->n));
        return expr(std::make_shared<stella::ast::NodeExprIsZero>(operand));
    }

    antlrcpp::Any visitApplication(antlr4_stella::StellaParser::ApplicationContext* ctx) override {
        if (ctx->args.size() > 1) {
            throw std::runtime_error(
                "Multiple arguments in function application are not supported yet");
        }

        auto fun = try_any_cast<std::shared_ptr<stella::ast::NodeExpr>>(visit(ctx->fun));
        auto arg = try_any_cast<std::shared_ptr<stella::ast::NodeExpr>>(visit(ctx->args[0]));

        return expr(std::make_shared<stella::ast::NodeExprApplication>(fun, arg));
    }

    antlrcpp::Any visitAbstraction(antlr4_stella::StellaParser::AbstractionContext* ctx) override {
        if (ctx->paramDecls.size() > 1) {
            throw std::runtime_error(
                "Multiple parameters in function abstraction are not supported yet");
        }

        auto param =
            try_any_cast<std::shared_ptr<stella::ast::NodeParamDecl>>(visit(ctx->paramDecls[0]));
        auto body = try_any_cast<std::shared_ptr<stella::ast::NodeExpr>>(visit(ctx->returnExpr));

        return expr(std::make_shared<stella::ast::NodeExprAbstraction>(param, body));
    }

    antlrcpp::Any
    visitParenthesisedExpr(antlr4_stella::StellaParser::ParenthesisedExprContext* ctx) override {
        return visit(ctx->expr_);
    }

    std::any visitParamDecl(antlr4_stella::StellaParser::ParamDeclContext* ctx) override {
        auto type = try_any_cast<std::shared_ptr<stella::ast::Type>>(visit(ctx->paramType));
        return std::make_shared<stella::ast::NodeParamDecl>(ctx->name->getText(), type);
    }

    antlrcpp::Any visitDeclFun(antlr4_stella::StellaParser::DeclFunContext* ctx) override {
        if (ctx->paramDecls.size() > 1) {
            throw std::runtime_error(
                "Multiple parameters in function abstraction are not supported yet");
        }

        auto param =
            try_any_cast<std::shared_ptr<stella::ast::NodeParamDecl>>(visit(ctx->paramDecls[0]));
        auto return_type = try_any_cast<std::shared_ptr<stella::ast::Type>>(visit(ctx->returnType));
        auto body = try_any_cast<std::shared_ptr<stella::ast::NodeExpr>>(visit(ctx->returnExpr));

        auto abstr = std::make_shared<stella::ast::NodeExprAbstraction>(param, body);

        return decl(
            std::make_shared<stella::ast::NodeDeclFun>(ctx->name->getText(), return_type, abstr));
    }

    antlrcpp::Any visitChildren(antlr4::tree::ParseTree* node) override {
        throw std::runtime_error("Unexpected node in AST builder: " + node->getText());
    }
};

} // namespace

std::shared_ptr<stella::ast::NodeProgram> stella::ParseProgram(std::string_view input) {
    antlr4::ANTLRInputStream stream(input);

    antlr4_stella::StellaLexer lexer(&stream);

    antlr4::CommonTokenStream tokens(&lexer);

    antlr4_stella::StellaParser parser(&tokens);

    antlr4_stella::StellaParser::Start_ProgramContext* tree = parser.start_Program();

    ASTBuilder builder;
    auto result = builder.visit(tree);

    return try_any_cast<std::shared_ptr<stella::ast::NodeProgram>>(result);
}
