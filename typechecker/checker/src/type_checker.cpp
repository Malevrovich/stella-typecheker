#include "stella/typecheck/type_checker.hpp"

#include <exception>
#include <format>
#include <memory>

#include <loguru.hpp>

#include "stella/ast/base.hpp"
#include "stella/ast/fun.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"
#include "stella/typecheck/name_context.hpp"

namespace stella {
namespace typecheck {

namespace {

constexpr std::string_view kMain = "main";

} // namespace

TypeChecker::~TypeChecker() = default;

void TypeChecker::Visit(const ast::NodeBase& node) {
    DLOG_S(INFO) << std::format("Visiting node {} {}", static_cast<const void*>(&node),
                                node.ToString());
    if (!types_storage_.has<DeducedType>(&node)) {
        try {
            node.Accept(*this);
        } catch (const std::exception& exc) {
            DLOG_S(ERROR) << "Exception occured during processing of node\n" << node.ToString();
            DLOG_S(ERROR) << "Exception: " << exc.what();
            throw;
        }
        DCHECK_F(types_storage_.has<DeducedType>(&node));
        DLOG_S(INFO) << std::format("Node {} got type {}", static_cast<const void*>(&node),
                                    types_storage_.get<DeducedType>(&node).type->ToString());
    } // else nothing to do, goal of this visitor is to get DeducedType
    DLOG_S(INFO) << std::format("Exited node {} {}", static_cast<const void*>(&node),
                                node.ToString());
}

void TypeChecker::CheckCompatibility(const ast::NodeBase& node,
                                     const ExpectedTypeList& expected_types,
                                     const DeducedType& deduced_type) const {
    for (const auto& exp : expected_types.All()) {
        auto conflict_error_code = exp.Check(*deduced_type.type);
        if (conflict_error_code) {
            OnError(TypeCheckNodeError{*conflict_error_code, node,
                                       std::format("Expected type {}, but expr has type {}",
                                                   exp.ToString(), deduced_type.type->ToString())});
        }
    }
}

void TypeChecker::SetDeducedType(const ast::NodeBase& node, DeducedType&& deduced_type) {
    const auto expected_types = types_storage_.tryGet<ExpectedTypeList>(&node);
    if (expected_types) {
        CheckCompatibility(node, *expected_types, deduced_type);
    }

    auto [result_type, was_set] =
        types_storage_.trySet<DeducedType>(&node, std::move(deduced_type));
    if (!was_set) {
        OnInternalError(std::format(
            "At: {}\n Unable to set deduced type to {}. Type is already set to {}", node.ToString(),
            deduced_type.type->ToString(), result_type->type->ToString()));
    }
}

void TypeChecker::ExpectType(const ast::NodeBase& node, ExpectedType&& expected_type) {
    const auto deduced_type = types_storage_.tryGet<DeducedType>(&node);

    auto& expected_list = types_storage_.get<ExpectedTypeList>(&node, ExpectedTypeList{});

    if (deduced_type) {
        auto conflict_error_code = expected_type.Check(*deduced_type->type);
        if (conflict_error_code) {
            OnError(TypeCheckNodeError{*conflict_error_code, node,
                                       std::format("Expected type {}, but expr has type {}",
                                                   expected_type.ToString(),
                                                   deduced_type->type->ToString())});
        }
    }

    expected_list.Push(std::move(expected_type));
}

void TypeChecker::PropagateExpectedType(const ast::NodeBase& src, const ast::NodeBase& dst) {
    const auto expected_types = types_storage_.tryGet<ExpectedTypeList>(&src);
    if (expected_types) {
        for (const auto& exp : expected_types->All()) {
            ExpectType(dst, ExpectedType{exp});
        }
    }
}

void TypeChecker::VisitProgram(const ast::NodeProgram& node) {
    std::vector<NameContext::NameContextGuard> name_guards;
    auto declarations = node.GetDeclarations();
    name_guards.reserve(declarations.size());

    for (auto& decl : declarations) {
        auto func_decl = std::dynamic_pointer_cast<const ast::NodeDeclFun>(decl);

        if (!func_decl) {
            OnError(NotSupportedError(*decl));
        }

        name_guards.push_back(
            name_context_.PushUnique(std::string{func_decl->GetName()}, *func_decl));
    }

    std::shared_ptr<const ast::Type> main_type = nullptr;

    for (auto& decl : node.GetDeclarations()) {
        Visit(*decl);

        auto decl_fun = std::dynamic_pointer_cast<const ast::NodeDeclFun>(decl);
        if (decl_fun && decl_fun->GetName() == kMain) {
            main_type = types_storage_.get<DeducedType>(decl_fun.get()).type;
        }
    }

    if (!main_type) {
        OnError(TypeCheckError{ErrorCode::ERROR_MISSING_MAIN});
    }

    SetDeducedType(node, {main_type});
}

} // namespace typecheck
} // namespace stella