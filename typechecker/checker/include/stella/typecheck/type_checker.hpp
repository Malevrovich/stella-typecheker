#pragma once

#include <concepts>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>

#include "stella/ast/ast.hpp"
#include "stella/ast/attribute_storage.hpp"
#include "stella/ast/base.hpp"
#include "stella/ast/fun.hpp"
#include "stella/ast/visitor.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"
#include "stella/typecheck/name_context.hpp"
#include "stella/typecheck/subtype.hpp"

namespace stella {
namespace typecheck {

class TypeChecker : public ast::BaseNodeVisitor, public ast::BaseTypeVisitor {
public:
    virtual ~TypeChecker();

    void Visit(const ast::NodeBase& node) override;
    void Visit(const ast::Type& type) override;

    void VisitProgram(const ast::NodeProgram& node) override;

    void VisitDeclFun(const ast::NodeDeclFun& node) override;
    void VisitParamDecl(const ast::NodeParamDecl& node) override;
    void VisitExprAbstraction(const ast::NodeExprAbstraction& node) override;
    void VisitExprApplication(const ast::NodeExprApplication& node) override;
    void VisitExprFix(const ast::NodeExprFix& node) override;

    void VisitExprConstInt(const ast::NodeExprConstInt& node) override;
    void VisitExprIsZero(const ast::NodeExprIsZero& node) override;
    void VisitExprSucc(const ast::NodeExprSucc& node) override;
    void VisitExprPred(const ast::NodeExprPred& node) override;
    void VisitExprNatRec(const ast::NodeExprNatRec& node) override;

    void VisitExprConstFalse(const ast::NodeExprConstFalse& node) override;
    void VisitExprConstTrue(const ast::NodeExprConstTrue& node) override;
    void VisitExprIf(const ast::NodeExprIf& node) override;

    void VisitExprVar(const ast::NodeExprVar& node) override;

    void VisitExprConstUnit(const ast::NodeExprConstUnit& node) override;

    void VisitExprTypeAsc(const ast::NodeExprTypeAsc& node) override;

    void VisitPatternVar(const ast::NodePatternVar& node) override;
    void VisitPatternCastAs(const ast::NodePatternCastAs& node) override;
    void VisitExprLet(const ast::NodeExprLet& node) override;

    void VisitExprList(const ast::NodeExprList& node) override;
    void VisitExprConsList(const ast::NodeExprConsList& node) override;
    void VisitExprHead(const ast::NodeExprHead& node) override;
    void VisitExprTail(const ast::NodeExprTail& node) override;
    void VisitExprIsEmpty(const ast::NodeExprIsEmpty& node) override;

    void VisitExprTuple(const ast::NodeExprTuple& node) override;
    void VisitExprDotTuple(const ast::NodeExprDotTuple& node) override;

    void VisitExprRecord(const ast::NodeExprRecord& node) override;
    void VisitExprDotRecord(const ast::NodeExprDotRecord& node) override;

    void VisitTypeRecord(const ast::TypeRecord& type) override;

    void VisitExprInl(const ast::NodeExprInl& node) override;
    void VisitExprInr(const ast::NodeExprInr& node) override;
    void VisitExprMatch(const ast::NodeExprMatch& node) override;

    void VisitTypeSum(const ast::TypeSum& type) override;

    void VisitExprVariant(const ast::NodeExprVariant& node) override;
    void VisitTypeVariant(const ast::TypeVariant& type) override;

    void VisitExprSequence(const ast::NodeExprSequence& node) override;

    void VisitTypeTop(const ast::TypeTop& type) override;
    void VisitTypeBottom(const ast::TypeBottom& type) override;

    void VisitExprPanic(const ast::NodeExprPanic& node) override;

    void VisitDeclExceptionType(const ast::NodeDeclExceptionType& node) override;
    void VisitDeclExceptionVariant(const ast::NodeDeclExceptionVariant& node) override;
    void VisitExprThrow(const ast::NodeExprThrow& node) override;
    void VisitExprTryWith(const ast::NodeExprTryWith& node) override;
    void VisitExprTryCatch(const ast::NodeExprTryCatch& node) override;

    void VisitExprTypeCast(const ast::NodeExprTypeCast& node) override;
    void VisitExprTryCastAs(const ast::NodeExprTryCastAs& node) override;

    void VisitTypeRef(const ast::TypeRef& type) override;
    void VisitExprRef(const ast::NodeExprRef& node) override;
    void VisitExprDeref(const ast::NodeExprDeref& node) override;
    void VisitExprAssign(const ast::NodeExprAssign& node) override;
    void VisitExprConstMemory(const ast::NodeExprConstMemory& node) override;

    void VisitDefaultNode(const ast::NodeBase& node) override { throw NotSupportedError(node); }

private:
    struct DeducedType {
        std::shared_ptr<const ast::Type> type;
    };

    struct ProvisionalType {
        std::shared_ptr<const ast::Type> type;
    };

    bool HasExtension(std::string_view name) const;

    void SetAmbiguousOrError(const ast::NodeBase& node, std::shared_ptr<const ast::Type> bot_type,
                             ErrorCode ambiguous_error, std::string_view message);

    std::optional<ErrorCode> CheckCompatible(const ast::Type& given,
                                             const ast::Type& expected) const;

    void ExpectType(const ast::NodeBase& node, ExpectedType&& expected_type);
    void PropagateExpectedType(const ast::NodeBase& src, const ast::NodeBase& dst);
    void SetDeducedType(const ast::NodeBase& node, DeducedType&& deduced_type);

    void SetProvisionalType(const ast::NodeBase& node, ProvisionalType&& provisional_type,
                            bool skip_structural_mismatch = false);

    template <typename T>
        requires std::derived_from<std::remove_cvref_t<T>, ast::Type>
    std::shared_ptr<const T> TryGetExpectedType(const ast::NodeBase& node) const;

    void VisitMatchArm(const ast::NodePatternVar* pattern_var,
                       std::shared_ptr<const ast::Type> bound_type,
                       const ast::NodeExprMatch& match_node, const ast::NodeBase& case_expr,
                       std::shared_ptr<const ast::Type>& result_type);

    void VisitMatchSum(const ast::NodeExprMatch& node, const ast::TypeSum& scrutinee_type);
    void VisitMatchVariant(const ast::NodeExprMatch& node, const ast::TypeVariant& scrutinee_type);

    NameContext name_context_;
    ast::AttributeStorage<ExpectedType, DeducedType, ProvisionalType> types_storage_;

    std::shared_ptr<const ast::Type> exception_type_{nullptr};
    // true when currently type-checking inside a function body (not at top level)
    bool in_function_body_{false};
    // true if exception_type_ was set via "exception type = ..." (as opposed to variants)
    bool exception_type_is_monotype_{false};
    // labels accumulated from "exception variant" declarations
    std::unordered_set<std::string> exception_variant_labels_;
    std::unordered_set<std::string> extensions_;

    SubtypeChecker subtype_checker_;
};

template <typename T>
    requires std::derived_from<std::remove_cvref_t<T>, ast::Type>
std::shared_ptr<const T> TypeChecker::TryGetExpectedType(const ast::NodeBase& node) const {
    const auto* exp = types_storage_.tryGet<ExpectedType>(&node);
    if (!exp)
        return nullptr;
    return std::dynamic_pointer_cast<const T>(exp->GetType());
}

} // namespace typecheck
} // namespace stella
