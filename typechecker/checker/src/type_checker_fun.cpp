#include "stella/ast/fun.hpp"
#include "stella/typecheck/type_checker.hpp"

#include <memory>

#include <loguru.hpp>

#include "stella/ast/ast.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"
#include "stella/typecheck/name_context.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitDeclFun(const ast::NodeDeclFun& node) {
    SetDeducedTypeFamily<ast::TypeFun>(node);

    const auto& abstraction = *node.GetAbstraction();
    const auto& param = abstraction.GetParam();
    Visit(*param);
    auto name_guard = name_context_.Push(std::string{param->GetName()}, *param);

    const auto& body = abstraction.GetBody();
    ExpectType(*body, ExpectedType::EqualsTo(node.GetReturnType()));
    Visit(*body);

    auto param_type = types_storage_.get<DeducedType>(param.get()).type;
    auto body_type = types_storage_.get<DeducedType>(body.get()).type;
    SetDeducedType(
        node, {std::make_shared<const ast::TypeFun>(std::move(param_type), std::move(body_type))});
}

void TypeChecker::VisitParamDecl(const ast::NodeParamDecl& node) {
    SetDeducedType(node, {node.GetType()});
}

void TypeChecker::VisitExprAbstraction(const ast::NodeExprAbstraction& node) {
    SetDeducedTypeFamily<ast::TypeFun>(node, ErrorCode::ERROR_UNEXPECTED_LAMBDA);

    std::shared_ptr<const ast::Type> expected_arg_type = nullptr;
    std::shared_ptr<const ast::Type> expected_body_type = nullptr;

    const auto expected_type = types_storage_.tryGet<ExpectedType>(&node);
    if (expected_type) {
        const auto expected_type_type =
            std::dynamic_pointer_cast<const ast::TypeFun>(expected_type->TryGetType());
        if (expected_type_type) {
            expected_arg_type = expected_type_type->GetArgType();
            expected_body_type = expected_type_type->GetReturnType();
        }
    }

    const auto& param = node.GetParam();
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
                   {std::make_shared<ast::TypeFun>(std::move(param_type), std::move(body_type))});
}

void TypeChecker::VisitExprApplication(const ast::NodeExprApplication& node) {
    const auto& function = node.GetFunction();
    ExpectType(*function,
               ExpectedType::CompatibleWith<ast::TypeFun>(ErrorCode::ERROR_NOT_A_FUNCTION));
    Visit(*function);

    const auto fun_type = std::dynamic_pointer_cast<const ast::TypeFun>(
        types_storage_.get<DeducedType>(function.get()).type);
    if (!fun_type) {
        OnInternalError("Unexpected function deduction type");
    }

    const auto& arg = node.GetArgument();
    const auto& param_type = fun_type->GetArgType();
    ExpectType(*arg, ExpectedType::EqualsTo(param_type));
    Visit(*arg);

    SetDeducedType(node, {fun_type->GetReturnType()});
}

} // namespace typecheck
} // namespace stella