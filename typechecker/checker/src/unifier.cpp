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

void Unifier::ReportUnificationError(ErrorCode error_code, const ast::Type& lhs,
                                     const ast::Type& rhs, const ast::NodeBase* node) {
    std::string message;
    if (error_code == ErrorCode::ERROR_OCCURS_CHECK_INFINITE_TYPE) {
        message = std::format("Occurs check failed: cannot unify {} with {}", lhs.ToString(),
                              rhs.ToString());
    } else {
        message = std::format("Cannot unify {} with {}", lhs.ToString(), rhs.ToString());
    }

    if (node) {
        OnError(TypeCheckNodeError{error_code, *node, message});
    } else {
        OnError(TypeCheckError{error_code, message});
    }
}

void Unifier::AddConstraint(std::shared_ptr<const ast::Type> lhs,
                            std::shared_ptr<const ast::Type> rhs, const ast::NodeBase* node) {
    constraints_.push_back({std::move(lhs), std::move(rhs), node});
}

std::shared_ptr<const ast::TypeAuto> Unifier::FreshTypeVar() {
    return std::make_shared<const ast::TypeAuto>();
}

void Unifier::UnifyAll(const SubtypeChecker* sc) {
    do {
        auto worklist = std::move(constraints_);
        constraints_.clear();

        while (!worklist.empty()) {
            auto constraint = worklist.back();
            worklist.pop_back();
            UnifyOne(std::move(constraint.lhs), std::move(constraint.rhs), constraint.node, sc);
        }
    } while (!constraints_.empty());
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
                           const ast::NodeBase* node, const SubtypeChecker* sc) {
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
                                       *cb.bound_type, node);
        }
        for (const auto* m : ca.members) {
            if (cb.bound_type->Contains([m](const ast::Type& t) {
                    return dynamic_cast<const ast::TypeAuto*>(&t) == m;
                }))
                ReportUnificationError(ErrorCode::ERROR_OCCURS_CHECK_INFINITE_TYPE, *ca.bound_type,
                                       *cb.bound_type, node);
        }

        ReconstructionComparator comparator{*this, node, sc};
        if (auto error = comparator(*ca.bound_type, *cb.bound_type)) {
            ReportUnificationError(*error, *ca.bound_type, *cb.bound_type, node);
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
                        const ast::NodeBase* node, const SubtypeChecker* sc) {
    EqClass& cls = ClassOf(var);

    if (cls.bound_type) {
        ReconstructionComparator comparator{*this, node, sc};
        if (auto error = comparator(*cls.bound_type, *concrete)) {
            ReportUnificationError(*error, *cls.bound_type, *concrete, node);
        }
        return;
    }

    for (const auto* m : cls.members) {
        if (concrete->Contains(
                [m](const ast::Type& t) { return dynamic_cast<const ast::TypeAuto*>(&t) == m; }))
            ReportUnificationError(ErrorCode::ERROR_OCCURS_CHECK_INFINITE_TYPE, ast::TypeAuto{},
                                   *concrete, node);
    }

    cls.bound_type = std::move(concrete);
}

void Unifier::UnifyOne(std::shared_ptr<const ast::Type> lhs, std::shared_ptr<const ast::Type> rhs,
                       const ast::NodeBase* node, const SubtypeChecker* sc) {
    const auto* lvar = dynamic_cast<const ast::TypeAuto*>(lhs.get());
    const auto* rvar = dynamic_cast<const ast::TypeAuto*>(rhs.get());

    if (lvar && rvar) {
        MergeClasses(lvar, rvar, node, sc);
        return;
    } else if (lvar) {
        BindClass(lvar, std::move(rhs), node, sc);
    } else if (rvar) {
        BindClass(rvar, std::move(lhs), node, sc);
    }
}

} // namespace typecheck
} // namespace stella
