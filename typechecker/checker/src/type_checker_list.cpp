#include "stella/typecheck/type_checker.hpp"

#include <memory>

#include "stella/ast/ast.hpp"
#include "stella/ast/list.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitExprList(const ast::NodeExprList& node) {
    SetDeducedTypeFamily<ast::TypeList>(node, ErrorCode::ERROR_UNEXPECTED_LIST);

    std::shared_ptr<const ast::Type> element_type = nullptr;
    if (const auto expected_list_type = TryGetExpectedType<ast::TypeList>(node)) {
        element_type = expected_list_type->GetElementType();
    }

    if (!element_type && node.GetElements().empty()) {
        OnError(TypeCheckNodeError{ErrorCode::ERROR_AMBIGUOUS_LIST, node,
                                   "Cannot determine element type of empty list literal"});
    }

    for (const auto& elem : node.GetElements()) {
        if (element_type) {
            ExpectType(*elem, ExpectedType::EqualsTo(element_type));
        }
        Visit(*elem);

        if (!element_type) {
            element_type = types_storage_.get<DeducedType>(elem.get()).type;
        }
    }

    SetDeducedType(node, {std::make_shared<ast::TypeList>(element_type)});
}

void TypeChecker::VisitExprConsList(const ast::NodeExprConsList& node) {
    SetDeducedTypeFamily<ast::TypeList>(node, ErrorCode::ERROR_UNEXPECTED_LIST);

    std::shared_ptr<const ast::Type> element_type = nullptr;
    if (const auto expected_list_type = TryGetExpectedType<ast::TypeList>(node)) {
        element_type = expected_list_type->GetElementType();
    }

    const auto& head = node.GetHead();
    const auto& tail = node.GetTail();

    ExpectType(*tail, ExpectedType::CompatibleWith<ast::TypeList>(ErrorCode::ERROR_NOT_A_LIST));

    if (element_type) {
        ExpectType(*head, ExpectedType::EqualsTo(element_type));
        ExpectType(*tail, ExpectedType::EqualsTo(std::make_shared<ast::TypeList>(element_type)));
    }

    Visit(*head);

    if (!element_type) {
        element_type = types_storage_.get<DeducedType>(head.get()).type;
        ExpectType(*tail, ExpectedType::EqualsTo(std::make_shared<ast::TypeList>(element_type)));
    }

    Visit(*tail);

    const auto tail_type = std::dynamic_pointer_cast<const ast::TypeList>(
        types_storage_.get<DeducedType>(tail.get()).type);
    if (!tail_type) {
        OnInternalError("Unexpected cons tail deduction type");
    }

    SetDeducedType(node, {std::make_shared<ast::TypeList>(element_type)});
}

void TypeChecker::VisitExprHead(const ast::NodeExprHead& node) {
    const auto& list = node.GetList();

    std::shared_ptr<const ast::Type> exact_element_type = nullptr;
    const auto expected_types = types_storage_.tryGet<ExpectedTypeList>(&node);
    if (expected_types) {
        for (const auto& exp : expected_types->All()) {
            const auto t = exp.TryGetType();
            if (t) {
                exact_element_type = t;
                break;
            }
        }
    }

    ExpectType(*list, ExpectedType::CompatibleWith<ast::TypeList>(ErrorCode::ERROR_NOT_A_LIST));
    if (exact_element_type) {
        ExpectType(*list,
                   ExpectedType::EqualsTo(std::make_shared<ast::TypeList>(exact_element_type)));
    }

    Visit(*list);

    const auto list_type = std::dynamic_pointer_cast<const ast::TypeList>(
        types_storage_.get<DeducedType>(list.get()).type);
    if (!list_type) {
        OnInternalError("Unexpected head list deduction type");
    }

    SetDeducedType(node, {list_type->GetElementType()});
}

void TypeChecker::VisitExprTail(const ast::NodeExprTail& node) {
    const auto& list = node.GetList();

    std::shared_ptr<const ast::Type> exact_list_type = nullptr;
    const auto expected_types = types_storage_.tryGet<ExpectedTypeList>(&node);
    if (expected_types) {
        for (const auto& exp : expected_types->All()) {
            const auto t = exp.TryGetType();
            if (t) {
                exact_list_type = t;
                break;
            }
        }
    }

    ExpectType(*list, ExpectedType::CompatibleWith<ast::TypeList>(ErrorCode::ERROR_NOT_A_LIST));
    if (exact_list_type) {
        ExpectType(*list, ExpectedType::EqualsTo(exact_list_type));
    }

    Visit(*list);

    const auto list_type = std::dynamic_pointer_cast<const ast::TypeList>(
        types_storage_.get<DeducedType>(list.get()).type);
    if (!list_type) {
        OnInternalError("Unexpected tail list deduction type");
    }

    SetDeducedType(node, {list_type});
}

void TypeChecker::VisitExprIsEmpty(const ast::NodeExprIsEmpty& node) {
    SetDeducedTypeFamily<ast::TypeBool>(node);

    const auto& list = node.GetList();
    ExpectType(*list, ExpectedType::CompatibleWith<ast::TypeList>(ErrorCode::ERROR_NOT_A_LIST));
    Visit(*list);

    SetDeducedType(node, {std::make_shared<ast::TypeBool>()});
}

} // namespace typecheck
} // namespace stella
