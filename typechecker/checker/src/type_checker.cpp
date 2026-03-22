#include "stella/typecheck/type_checker.hpp"

#include <format>
#include <memory>

#include <loguru.hpp>

#include "stella/ast/base.hpp"
#include "stella/ast/fun.hpp"
#include "stella/ast/visitor.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/name_context.hpp"

namespace stella {
namespace typecheck {

namespace {

constexpr std::string_view kMain = "main";

} // namespace

TypeChecker::~TypeChecker() = default;

void TypeChecker::Visit(const ast::NodeBase& node) {
    if (!types_storage_.has<DeducedType>(&node)) {
        node.Accept(*this);
        DCHECK_F(types_storage_.has<DeducedType>(&node));
    } // else nothing to do, goal of this visitor is to get DeducedType
}

void TypeChecker::CheckCompatibility(const ast::NodeBase& node, const ExpectedType& expected_type,
                                     const DeducedType& deduced_type) const {
    auto conflict_error_code = expected_type.Check(deduced_type.type);
    if (conflict_error_code) {
        OnError(TypeCheckNodeError{*conflict_error_code, node,
                                   std::format("Expected type {}, but expr has type of {}",
                                               expected_type.ToString(),
                                               deduced_type.type.ToString())});
    }
}

void TypeChecker::SetDeducedType(const ast::NodeBase& node, DeducedType&& deduced_type) {
    const auto expected_type = types_storage_.tryGet<ExpectedType>(&node);
    if (expected_type) {
        CheckCompatibility(node, *expected_type, deduced_type);
    }

    auto [result_type, was_set] =
        types_storage_.trySet<DeducedType>(&node, std::move(deduced_type));
    if (!was_set) {
        OnInternalError(std::format(
            "At: {}\n Unable to set deduced type to {}. Type is already set to {}", node.ToString(),
            deduced_type.type.ToString(), result_type->type.ToString()));
    }
}

void TypeChecker::ExpectType(const ast::NodeBase& node, ExpectedType&& expected_type) {
    const auto deduced_type = types_storage_.tryGet<DeducedType>(&node);
    if (deduced_type) {
        CheckCompatibility(node, expected_type, *deduced_type);
    }

    auto [prev_expected_type, was_set] =
        types_storage_.trySet<ExpectedType>(&node, std::move(expected_type));

    if (!was_set) {
        OnInternalError(std::format(
            "At node: {}.\nDouble expectation is not possible in the current version of "
            "the algorithm",
            node.ToString()));
    }
}

void TypeChecker::VisitProgram(const ast::NodeProgram& node) {
    std::vector<NameContext::NameContextGuard> name_guards;
    name_guards.reserve(node.GetDeclarations().size());

    for (auto& decl : node.GetDeclarations()) {
        auto func_decl = std::dynamic_pointer_cast<ast::NodeDeclFun>(decl);

        if (!func_decl) {
            OnError(NotSupportedError(*decl));
        }

        name_guards.push_back(
            name_context_.PushUnique(std::string{func_decl->GetName()}, *func_decl));

        ExpectType(*func_decl, ExpectedType::EqualsTo(func_decl->GetReturnType()));
    }

    const ast::Type* main_type = nullptr;

    for (auto& decl : node.GetDeclarations()) {
        ast::BaseNodeVisitor::Visit(*decl);

        auto decl_fun = std::dynamic_pointer_cast<ast::NodeDeclFun>(decl);
        if (decl_fun && decl_fun->GetName() == kMain) {
            main_type = &types_storage_.get<DeducedType>(decl_fun.get()).type;
        }
    }

    if (!main_type) {
        OnError(TypeCheckError{ErrorCode::ERROR_MISSING_MAIN});
    }

    SetDeducedType(node, {*main_type});
}

void TypeChecker::VisitDeclFun(const ast::NodeDeclFun& node) {}

} // namespace typecheck
} // namespace stella