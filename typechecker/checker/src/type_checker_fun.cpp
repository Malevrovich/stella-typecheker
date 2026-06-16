#include "stella/ast/fun.hpp"
#include "stella/typecheck/type_checker.hpp"

#include <memory>

#include <loguru.hpp>

#include "stella/ast/ast.hpp"
#include "stella/ast/base.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"
#include "stella/typecheck/name_context.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitDeclFun(const ast::NodeDeclFun& node) {
    SetProvisionalType(node, {ast::TypeFun::MakeSentinel()});

    Visit(*node.GetReturnType());

    const auto& abstraction = *node.GetAbstraction();
    const auto& param = abstraction.GetParam();
    Visit(*param->GetType());
    Visit(*param);

    // Pre-set a DeducedType on this DeclFun from the declared param/return types so that
    // recursive references to the function (e.g. `return main`) find a valid type and do
    // not cause infinite recursion. The final SetDeducedType below will overwrite this.
    {
        auto param_type = types_storage_.get<DeducedType>(param.get()).type;
        auto return_type = node.GetReturnType();
        types_storage_.set<DeducedType>(
            &node, {CreateType<ast::TypeFun>(param_type, return_type)});
    }

    auto name_guard = name_context_.Push(std::string{param->GetName()}, *param);

    const bool was_in_function_body = in_function_body_;
    in_function_body_ = true;

    // Visit local declarations (e.g. exception type declarations) before the body.
    for (const auto& local_decl : node.GetLocalDecls()) {
        Visit(*local_decl);
    }

    const auto& body = abstraction.GetBody();
    ExpectType(*body, ExpectedType::EqualsTo(node.GetReturnType()));
    Visit(*body);

    in_function_body_ = was_in_function_body;

    auto param_type = types_storage_.get<DeducedType>(param.get()).type;
    auto body_type = types_storage_.get<DeducedType>(body.get()).type;
    SetDeducedType(
        node, {CreateType<ast::TypeFun>(std::move(param_type), std::move(body_type))});
}

void TypeChecker::VisitParamDecl(const ast::NodeParamDecl& node) {
    SetDeducedType(node, {node.GetType()});
}

void TypeChecker::VisitExprAbstraction(const ast::NodeExprAbstraction& node) {
    // If the expected type is a non-function concrete type, override the mismatch error to
    // ERROR_UNEXPECTED_LAMBDA (this is a lambda where a non-function type is expected).
    // Must happen BEFORE SetProvisionalType which also checks against expected type.
    {
        const auto* exp = types_storage_.tryGet<ExpectedType>(&node);
        if (exp && !exp->GetType()->IsSentinel() &&
            !std::dynamic_pointer_cast<const ast::TypeFun>(exp->GetType()) &&
            !exp->HasExplicitMismatchError()) {
            // Directly overwrite the ExpectedType slot (not via ExpectType which would
            // immediately check and throw using the old error code).
            types_storage_.set<ExpectedType>(
                &node, ExpectedType::EqualsTo(exp->GetType(), ErrorCode::ERROR_UNEXPECTED_LAMBDA));
        }
    }

    SetProvisionalType(node, {ast::TypeFun::MakeSentinel()});

    std::shared_ptr<const ast::Type> expected_arg_type = nullptr;
    std::shared_ptr<const ast::Type> expected_body_type = nullptr;

    if (const auto expected_fun_type = TryGetExpectedType<ast::TypeFun>(node)) {
        expected_arg_type = expected_fun_type->GetArgType();
        expected_body_type = expected_fun_type->GetReturnType();
    }

    const auto& param = node.GetParam();
    Visit(*param->GetType());
    if (expected_arg_type) {
        ExpectType(*param, ExpectedType::EqualsTo(expected_arg_type,
                                                  ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_PARAMETER));
    }
    Visit(*param);

    auto name_guard = name_context_.Push(std::string{param->GetName()}, *param);

    const auto& body = node.GetBody();
    if (expected_body_type) {
        ExpectType(*body, ExpectedType::EqualsTo(expected_body_type));
    }
    Visit(*body);

    auto param_type = types_storage_.get<DeducedType>(param.get()).type;
    auto body_type = types_storage_.get<DeducedType>(body.get()).type;
    SetDeducedType(node,
                   {CreateType<ast::TypeFun>(std::move(param_type), std::move(body_type))});
}

void TypeChecker::VisitExprFix(const ast::NodeExprFix& node) {
    const auto& expr = node.GetExpr();
    const auto* exp = types_storage_.tryGet<ExpectedType>(&node);

    ExpectType(*expr, ExpectedType::EqualsTo(ast::TypeFun::MakeSentinel()));

    if (exp) {
        const auto expected_type_type = exp->GetType();
        const auto expected_fun = std::dynamic_pointer_cast<const ast::TypeFun>(expected_type_type);
        if (expected_fun && !expected_fun->IsSentinel()) {
            ExpectType(*expr, ExpectedType::EqualsTo(CreateType<ast::TypeFun>(
                                  expected_type_type, expected_type_type)));
        }
    }
    Visit(*expr);

    const auto fun_type = std::dynamic_pointer_cast<const ast::TypeFun>(
        types_storage_.get<DeducedType>(expr.get()).type);
    if (!fun_type) {
        OnInternalError("Unexpected fix expression deduction type");
    }

    auto err = CheckCompatible(fun_type->GetReturnType(), fun_type->GetArgType());

    if (err) {
        OnError(TypeCheckNodeError(
            *err, node, "Mismatched parameter type and return type in fix combinator argument"));
    }

    SetDeducedType(node, {fun_type->GetReturnType()});
}

void TypeChecker::VisitExprApplication(const ast::NodeExprApplication& node) {
    const auto& function = node.GetFunction();
    ExpectType(*function, ExpectedType::EqualsTo(ast::TypeFun::MakeSentinel()));
    Visit(*function);

    const auto deduced_fun = types_storage_.get<DeducedType>(function.get()).type;
    auto fun_type = std::dynamic_pointer_cast<const ast::TypeFun>(deduced_fun);

    if (!fun_type) {
        // In type-reconstruction mode the function expression may have type TypeAuto.
        // Introduce fresh type variables for arg and return, add a constraint, and proceed.
        if (HasExtension("#type-reconstruction") &&
            std::dynamic_pointer_cast<const ast::TypeAuto>(deduced_fun)) {
            auto arg_var = unifier_.FreshTypeVar();
            auto ret_var = unifier_.FreshTypeVar();
            fun_type = CreateType<ast::TypeFun>(arg_var, ret_var);
            unifier_.AddConstraint(deduced_fun, fun_type, &node);
            unifier_.SaveNewType(fun_type);
        } else {
            OnInternalError("Unexpected function deduction type");
        }
    }

    const auto& arg = node.GetArgument();
    const auto& param_type = fun_type->GetArgType();
    ExpectType(*arg, ExpectedType::EqualsTo(param_type));
    Visit(*arg);

    SetDeducedType(node, {fun_type->GetReturnType()});
}

} // namespace typecheck
} // namespace stella