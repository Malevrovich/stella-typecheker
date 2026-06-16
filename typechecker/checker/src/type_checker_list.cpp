#include "stella/typecheck/type_checker.hpp"

#include <memory>

#include "stella/ast/ast.hpp"
#include "stella/ast/base.hpp"
#include "stella/ast/list.hpp"
#include "stella/ast/top_bottom.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitExprList(const ast::NodeExprList& node) {
    SetProvisionalType(node, {ast::TypeList::MakeSentinel()});

    std::shared_ptr<const ast::Type> element_type = nullptr;
    if (const auto expected_list_type = TryGetExpectedType<ast::TypeList>(node)) {
        element_type = expected_list_type->GetElementType();
    }

    if (!element_type && node.GetElements().empty()) {
        SetAmbiguousOrError(node, std::make_shared<ast::TypeList>(ast::TypeBottom::Get()),
                            ErrorCode::ERROR_AMBIGUOUS_LIST,
                            "Cannot determine element type of empty list literal");
        return;
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
    SetProvisionalType(node, {ast::TypeList::MakeSentinel()});

    std::shared_ptr<const ast::Type> element_type = nullptr;
    if (const auto expected_list_type = TryGetExpectedType<ast::TypeList>(node)) {
        if (!expected_list_type->IsSentinel()) {
            element_type = expected_list_type->GetElementType();
        }
    }

    const auto& head = node.GetHead();
    const auto& tail = node.GetTail();

    // Phase 1: just ensure tail is a list family.
    ExpectType(*tail, ExpectedType::EqualsTo(ast::TypeList::MakeSentinel()));

    if (element_type) {
        ExpectType(*head, ExpectedType::EqualsTo(element_type));
        // Phase 2: refine to concrete list type (overwrites the sentinel expectation).
        ExpectType(*tail, ExpectedType::EqualsTo(std::make_shared<ast::TypeList>(element_type)));
    }

    Visit(*head);

    if (!element_type) {
        element_type = types_storage_.get<DeducedType>(head.get()).type;
        // Phase 2: refine after deducing head type.
        ExpectType(*tail, ExpectedType::EqualsTo(std::make_shared<ast::TypeList>(element_type)));
    }

    Visit(*tail);

    const auto tail_deduced = types_storage_.get<DeducedType>(tail.get()).type;
    const auto tail_type = std::dynamic_pointer_cast<const ast::TypeList>(tail_deduced);
    if (!tail_type) {
        // In type-reconstruction mode the tail may have type TypeAuto.
        if (HasExtension("#type-reconstruction") &&
            std::dynamic_pointer_cast<const ast::TypeAuto>(tail_deduced)) {
            auto list_var = unifier_.FreshTypeVar();
            auto new_list_type = std::make_shared<ast::TypeList>(element_type);
            unifier_.AddConstraint(tail_deduced, new_list_type, &node);
            unifier_.SaveNewType(new_list_type);
            SetDeducedType(node, {new_list_type});
            return;
        }
        OnInternalError("Unexpected cons tail deduction type");
    }

    SetDeducedType(node, {std::make_shared<ast::TypeList>(element_type)});
}

void TypeChecker::VisitExprHead(const ast::NodeExprHead& node) {
    const auto& list = node.GetList();

    // If we know the expected element type, propagate as concrete list; otherwise sentinel.
    const auto* exp = types_storage_.tryGet<ExpectedType>(&node);
    const auto exact_element_type = exp ? exp->GetType() : nullptr;

    if (exact_element_type && !exact_element_type->IsSentinel()) {
        ExpectType(*list,
                   ExpectedType::EqualsTo(std::make_shared<ast::TypeList>(exact_element_type)));
    } else {
        ExpectType(*list, ExpectedType::EqualsTo(ast::TypeList::MakeSentinel()));
    }

    Visit(*list);

    const auto list_deduced = types_storage_.get<DeducedType>(list.get()).type;
    const auto list_type = std::dynamic_pointer_cast<const ast::TypeList>(list_deduced);
    if (!list_type) {
        // In type-reconstruction mode the list may have type TypeAuto.
        if (HasExtension("#type-reconstruction") &&
            std::dynamic_pointer_cast<const ast::TypeAuto>(list_deduced)) {
            auto elem_var = unifier_.FreshTypeVar();
            auto new_list_type = std::make_shared<ast::TypeList>(elem_var);
            unifier_.AddConstraint(list_deduced, new_list_type, &node);
            unifier_.SaveNewType(new_list_type);
            SetDeducedType(node, {elem_var});
            return;
        }
        OnInternalError("Unexpected head list deduction type");
    }

    SetDeducedType(node, {list_type->GetElementType()});
}

void TypeChecker::VisitExprTail(const ast::NodeExprTail& node) {
    const auto& list = node.GetList();

    // If we know the expected list type, propagate as concrete; otherwise sentinel.
    const auto* exp = types_storage_.tryGet<ExpectedType>(&node);
    const auto exact_list_type = exp ? exp->GetType() : nullptr;

    if (exact_list_type && !exact_list_type->IsSentinel()) {
        ExpectType(*list, ExpectedType::EqualsTo(exact_list_type));
    } else {
        ExpectType(*list, ExpectedType::EqualsTo(ast::TypeList::MakeSentinel()));
    }

    Visit(*list);

    const auto list_deduced = types_storage_.get<DeducedType>(list.get()).type;
    const auto list_type = std::dynamic_pointer_cast<const ast::TypeList>(list_deduced);
    if (!list_type) {
        // In type-reconstruction mode the list may have type TypeAuto.
        if (HasExtension("#type-reconstruction") &&
            std::dynamic_pointer_cast<const ast::TypeAuto>(list_deduced)) {
            auto elem_var = unifier_.FreshTypeVar();
            auto inferred_list = std::make_shared<ast::TypeList>(elem_var);
            unifier_.AddConstraint(list_deduced, inferred_list, &node);
            unifier_.SaveNewType(inferred_list);
            SetDeducedType(node, {inferred_list});
            return;
        }
        OnInternalError("Unexpected tail list deduction type");
    }

    SetDeducedType(node, {list_type});
}

void TypeChecker::VisitExprIsEmpty(const ast::NodeExprIsEmpty& node) {
    SetProvisionalType(node, {std::make_shared<ast::TypeBool>()});

    const auto& list = node.GetList();
    ExpectType(*list, ExpectedType::EqualsTo(ast::TypeList::MakeSentinel()));
    Visit(*list);

    SetDeducedType(node, {std::make_shared<ast::TypeBool>()});
}

} // namespace typecheck
} // namespace stella
