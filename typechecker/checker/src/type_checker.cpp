#include "stella/typecheck/type_checker.hpp"

#include <exception>
#include <format>
#include <loguru.hpp>
#include <memory>

#include "stella/ast/base.hpp"
#include "stella/ast/exception.hpp"
#include "stella/ast/fun.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"
#include "stella/typecheck/name_context.hpp"
#include "stella/typecheck/reconstruction.hpp"
#include "stella/typecheck/subtype.hpp"

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

std::optional<ErrorCode> TypeChecker::CheckCompatible(std::shared_ptr<const ast::Type> given,
                                                      std::shared_ptr<const ast::Type> expected) {
    if (!given || !expected) {
        return std::nullopt;
    }

    if (HasExtension("#type-reconstruction")) {
        const SubtypeChecker* sc =
            HasExtension("#structural-subtyping") ? &subtype_checker_ : nullptr;
        ReconstructionComparator cmp(unifier_, sc);
        return cmp(*given, *expected);
    }

    if (HasExtension("#structural-subtyping")) {
        SubtypeAwareComparator cmp(subtype_checker_);
        return cmp(*given, *expected);
    }

    return given->CheckCompatible(*expected);
}

// Returns true if the error code is a "structural" same-family mismatch that should
// be overridden by the ExpectedType's mismatch_error (default:
// ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION) when raised implicitly through SetDeducedType
// or SetProvisionalType on a non-constructor expression.
//
// The precise codes are still emitted explicitly by the visitor methods for constructors
// (e.g. VisitExprRecord emits MISSING/UNEXPECTED_RECORD_FIELDS before SetDeducedType).
//
// ERROR_UNEXPECTED_RECORD_FIELDS: raised by CheckCompatibleImpl when given has MORE
//   fields than expected.  On a function call result this means the callee returns a
//   wider record — not a record-constructor problem, so use the generic fallback.
// ERROR_UNEXPECTED_TYPE_FOR_PARAMETER: raised by TypeFun::CheckCompatibleImpl when arg
//   types differ — only meaningful for lambda parameter mismatches (raised explicitly in
//   VisitExprAbstraction), not for function-valued expressions at call sites.
static bool IsStructuralMismatch(ErrorCode code) {
    switch (code) {
    case ErrorCode::ERROR_UNEXPECTED_RECORD_FIELDS:
    case ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_PARAMETER:
        return true;
    default:
        return false;
    }
}

void TypeChecker::ExpectType(const ast::NodeBase& node, ExpectedType&& expected_type) {
    const auto* deduced_type = types_storage_.tryGet<DeducedType>(&node);

    if (deduced_type) {
        auto error = CheckCompatible(deduced_type->type, expected_type.GetType());

        if (error) {
            const ErrorCode final_error =
                (expected_type.HasExplicitMismatchError() || IsStructuralMismatch(*error))
                    ? expected_type.GetMismatchError()
                    : *error;
            OnError(TypeCheckNodeError{final_error, node,
                                       std::format("Expected type {}, but expr has type {}",
                                                   expected_type.ToString(),
                                                   deduced_type->type->ToString())});
        }
    }

    types_storage_.set<ExpectedType>(&node, std::move(expected_type));
}

void TypeChecker::SetProvisionalType(const ast::NodeBase& node, ProvisionalType&& provisional_type,
                                     bool skip_structural_mismatch) {
    const auto* exp = types_storage_.tryGet<ExpectedType>(&node);

    if (exp) {
        auto error = CheckCompatible(provisional_type.type, exp->GetType());

        if (error) {
            const ErrorCode final_error =
                (exp->HasExplicitMismatchError() ||
                 (!skip_structural_mismatch && IsStructuralMismatch(*error)))
                    ? exp->GetMismatchError()
                    : *error;
            OnError(TypeCheckNodeError{final_error, node,
                                       std::format("Expected type {}, but expr has type {}",
                                                   exp->ToString(),
                                                   provisional_type.type->ToString())});
        }
    }

    types_storage_.set<ProvisionalType>(&node, std::move(provisional_type));
}

void TypeChecker::SetDeducedType(const ast::NodeBase& node, DeducedType&& deduced_type) {
    const auto* exp = types_storage_.tryGet<ExpectedType>(&node);

    if (exp) {
        auto error = CheckCompatible(deduced_type.type, exp->GetType());

        if (error) {
            const ErrorCode final_error =
                (exp->HasExplicitMismatchError() || IsStructuralMismatch(*error))
                    ? exp->GetMismatchError()
                    : *error;
            OnError(
                TypeCheckNodeError{final_error, node,
                                   std::format("Expected type {}, but expr has type {}",
                                               exp->ToString(), deduced_type.type->ToString())});
        }
    }

    types_storage_.set<DeducedType>(&node, std::move(deduced_type));
}

void TypeChecker::PropagateExpectedType(const ast::NodeBase& src, const ast::NodeBase& dst) {
    const auto* exp = types_storage_.tryGet<ExpectedType>(&src);
    // Do not propagate sentinel (family-hint) expected types: they express a family
    // constraint on the parent node itself, not a concrete expectation for sub-expressions.
    if (exp && !exp->GetType()->IsSentinel()) {
        ExpectType(dst, ExpectedType{*exp});
    }
}

bool TypeChecker::HasExtension(std::string_view name) const {
    return extensions_.count(std::string{name}) > 0;
}

void TypeChecker::SetAmbiguousOrError(const ast::NodeBase& node,
                                      std::shared_ptr<const ast::Type> bot_type,
                                      ErrorCode ambiguous_error, std::string_view message) {
    if (HasExtension("#ambiguous-type-as-bottom")) {
        SetDeducedType(node, {std::move(bot_type)});
    } else {
        OnError(TypeCheckNodeError{ambiguous_error, node, message});
    }
}

void TypeChecker::VisitProgram(const ast::NodeProgram& node) {
    extensions_ = node.GetExtensions();

    std::vector<NameContext<const ast::NodeBase>::NameContextGuard> name_guards;
    auto declarations = node.GetDeclarations();
    name_guards.reserve(declarations.size());

    for (auto& decl : declarations) {
        if (std::dynamic_pointer_cast<const ast::NodeDeclExceptionType>(decl)) {
            continue;
        }
        if (std::dynamic_pointer_cast<const ast::NodeDeclExceptionVariant>(decl)) {
            continue;
        }

        if (auto func_decl = std::dynamic_pointer_cast<const ast::NodeDeclFun>(decl)) {
            name_guards.push_back(
                name_context_.PushUnique(std::string{func_decl->GetName()}, *func_decl));
        } else if (auto gen_decl = std::dynamic_pointer_cast<const ast::NodeDeclFunGeneric>(decl)) {
            name_guards.push_back(
                name_context_.PushUnique(std::string{gen_decl->GetName()}, *gen_decl));
        } else {
            OnError(NotSupportedError(*decl));
        }
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

    // After visiting all declarations, run unification to resolve type variables.
    if (HasExtension("#type-reconstruction")) {
        if (auto err = unifier_.UnifyAll()) {
            OnError(TypeCheckError{*err, "Type unification failed during type reconstruction"});
        }
    }

    SetDeducedType(node, {main_type});
}

} // namespace typecheck
} // namespace stella