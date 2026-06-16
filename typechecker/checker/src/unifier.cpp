#include "stella/typecheck/unifier.hpp"

#include <format>
#include <memory>
#include <vector>

#include "stella/ast/auto.hpp"
#include "stella/ast/base.hpp"
#include "stella/typecheck/reconstruction.hpp"
#include "stella/typecheck/subtype.hpp"

namespace stella {
namespace typecheck {

namespace {

std::optional<ErrorCode> TryCompareWith(const ast::Type::Comparator& comparator,
                                        const ast::Type& lhs, const ast::Type& rhs) {
    if (comparator) {
        return comparator(lhs, rhs);
    } else {
        return lhs.CheckCompatible(rhs);
    }
}

} // namespace

namespace {

std::string FormatTypeInfo(const ast::Type& type, std::string_view label) {
    std::string result = std::format("\n{}: {}", label, type.ToString());

    if (type.HasOriginNode()) {
        const auto* origin_node = type.GetOriginNode();
        result += std::format("\n  {} is from: {}", label, origin_node->ToString());
        if (origin_node->GetSourceInfo()) {
            result += std::format("\n  Location: {}", origin_node->GetSourceInfo()->GetLocation());
        }
    } else if (type.HasSourceInfo()) {
        result += std::format("\n  {} from source: {}", label, type.GetSourceInfo()->GetLocation());
    }

    return result;
}

} // namespace

void Unifier::ReportUnificationError(ErrorCode error_code, const ast::Type& lhs,
                                     const ast::Type& rhs) {
    // Build detailed error message with type origin information
    std::string message;
    if (error_code == ErrorCode::ERROR_OCCURS_CHECK_INFINITE_TYPE) {
        message = std::format("Occurs check failed: cannot unify {} with {}", lhs.ToString(),
                              rhs.ToString());
    } else {
        message = std::format("Cannot unify {} with {}", lhs.ToString(), rhs.ToString());
    }

    // Add type information using helper function
    message += FormatTypeInfo(lhs, "First type");
    message += FormatTypeInfo(rhs, "Second type");

    // Try to get origin node from types - prefer the type that has origin info
    const ast::NodeBase* node = nullptr;
    if (lhs.HasOriginNode()) {
        node = lhs.GetOriginNode();
    } else if (rhs.HasOriginNode()) {
        node = rhs.GetOriginNode();
    }

    if (node) {
        OnError(TypeCheckNodeError{error_code, *node, message});
    } else {
        // Fallback to TypeCheckError if no origin node is available
        OnError(TypeCheckError{error_code, message});
    }
}

void Unifier::AddConstraint(std::shared_ptr<const ast::Type> lhs,
                            std::shared_ptr<const ast::Type> rhs) {
    constraints_.push_back({std::move(lhs), std::move(rhs)});
}

std::shared_ptr<const ast::TypeAuto> Unifier::FreshTypeVar() {
    return std::make_shared<const ast::TypeAuto>(nullptr, nullptr);
}

void Unifier::UnifyAll(const SubtypeChecker* sc) {
    do {
        auto worklist = std::move(constraints_);
        constraints_.clear();

        while (!worklist.empty()) {
            auto constraint = worklist.back();
            worklist.pop_back();
            UnifyOne(std::move(constraint.lhs), std::move(constraint.rhs), sc);
        }
    } while (!constraints_.empty());

    // After unification, check that all TypeAuto instances have been bound to a concrete type
    auto all_auto = ast::TypeAuto::Registry::GetInstance().GetAll();
    for (const auto* type_auto : all_auto) {
        auto it = class_of_.find(type_auto);
        if (it != class_of_.end()) {
            const EqClass& cls = *classes_[it->second];
            if (!cls.bound_type) {
                // This TypeAuto has no bound type - report error
                std::string message = std::format("Ambiguous type: type variable was not inferred");
                message += FormatTypeInfo(*type_auto, "Auto:");
                OnError(TypeCheckError{ErrorCode::ERROR_AMBIGUOUS_TYPE, message});
            }
        } else {
            // If TypeAuto is not in class_of_, it was never used in constraints, so no bound type
            // exists This is also an ambiguous type error
            std::string message = std::format("Ambiguous type: type variable was not inferred");
            message += FormatTypeInfo(*type_auto, "Auto:");
            OnError(TypeCheckError{ErrorCode::ERROR_AMBIGUOUS_TYPE, message});
        }
    }
}

std::size_t Unifier::FindClass(const ast::TypeAuto* var) {
    auto it = class_of_.find(var);
    if (it != class_of_.end())
        return it->second;
    const std::size_t idx = classes_.size();
    classes_.push_back(std::make_unique<EqClass>());
    classes_.back()->members.insert(var);
    class_of_[var] = idx;
    return idx;
}

Unifier::EqClass& Unifier::ClassOf(const ast::TypeAuto* var) { return *classes_[FindClass(var)]; }

void Unifier::MergeClasses(const ast::TypeAuto* a, const ast::TypeAuto* b,
                           const SubtypeChecker* sc) {
    const std::size_t ia = FindClass(a);
    const std::size_t ib = FindClass(b);
    if (ia == ib)
        return;

    EqClass& ca = *classes_[ia];
    EqClass& cb = *classes_[ib];

    if (ca.bound_type && cb.bound_type) {
        for (const auto* m : cb.members) {
            if (ca.bound_type->Contains([m](const ast::Type& t) {
                    return dynamic_cast<const ast::TypeAuto*>(&t) == m;
                }))
                ReportUnificationError(ErrorCode::ERROR_OCCURS_CHECK_INFINITE_TYPE, *ca.bound_type,
                                       *cb.bound_type);
        }
        for (const auto* m : ca.members) {
            if (cb.bound_type->Contains([m](const ast::Type& t) {
                    return dynamic_cast<const ast::TypeAuto*>(&t) == m;
                }))
                ReportUnificationError(ErrorCode::ERROR_OCCURS_CHECK_INFINITE_TYPE, *ca.bound_type,
                                       *cb.bound_type);
        }

        ReconstructionComparator comparator{*this, sc};
        if (auto error = comparator(*ca.bound_type, *cb.bound_type)) {
            ReportUnificationError(ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION, *ca.bound_type,
                                   *cb.bound_type);
        }
    }

    for (const auto* m : cb.members) {
        ca.members.insert(m);
        class_of_[m] = ia;
    }
    if (!ca.bound_type && cb.bound_type)
        ca.bound_type = std::move(cb.bound_type);

    classes_[ib] = std::make_unique<EqClass>();
}

void Unifier::BindClass(const ast::TypeAuto* var, std::shared_ptr<const ast::Type> concrete,
                        const SubtypeChecker* sc) {
    EqClass& cls = ClassOf(var);

    if (cls.bound_type) {
        ReconstructionComparator comparator{*this, sc};
        if (auto error = comparator(*cls.bound_type, *concrete)) {
            ReportUnificationError(ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION, *cls.bound_type,
                                   *concrete);
        }
        return;
    }

    for (const auto* m : cls.members) {
        if (concrete->Contains(
                [m](const ast::Type& t) { return dynamic_cast<const ast::TypeAuto*>(&t) == m; }))
            ReportUnificationError(ErrorCode::ERROR_OCCURS_CHECK_INFINITE_TYPE, ast::TypeAuto{},
                                   *concrete);
    }

    cls.bound_type = std::move(concrete);
}

void Unifier::UnifyOne(std::shared_ptr<const ast::Type> lhs, std::shared_ptr<const ast::Type> rhs,
                       const SubtypeChecker* sc) {
    const auto* lvar = dynamic_cast<const ast::TypeAuto*>(lhs.get());
    const auto* rvar = dynamic_cast<const ast::TypeAuto*>(rhs.get());

    if (lvar && rvar) {
        MergeClasses(lvar, rvar, sc);
        return;
    } else if (lvar) {
        BindClass(lvar, std::move(rhs), sc);
    } else if (rvar) {
        BindClass(rvar, std::move(lhs), sc);
    }
}

} // namespace typecheck
} // namespace stella
