#include "stella/typecheck/unifier.hpp"

#include <memory>
#include <vector>

#include "stella/ast/auto.hpp"

namespace stella {
namespace typecheck {

void Unifier::AddConstraint(std::shared_ptr<const ast::Type> lhs,
                            std::shared_ptr<const ast::Type> rhs) {
    constraints_.emplace_back(std::move(lhs), std::move(rhs));
}

std::shared_ptr<const ast::TypeAuto> Unifier::FreshTypeVar() {
    ++next_type_var_id_;
    return std::make_shared<const ast::TypeAuto>();
}

std::optional<ErrorCode> Unifier::UnifyAll() {
    std::vector<std::pair<std::shared_ptr<const ast::Type>, std::shared_ptr<const ast::Type>>>
        worklist = std::move(constraints_);
    constraints_.clear();

    while (!worklist.empty()) {
        auto [lhs, rhs] = worklist.back();
        worklist.pop_back();
        if (auto err = UnifyOne(std::move(lhs), std::move(rhs)))
            return err;
    }
    return std::nullopt;
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

std::optional<ErrorCode> Unifier::MergeClasses(const ast::TypeAuto* a, const ast::TypeAuto* b) {
    const std::size_t ia = FindClass(a);
    const std::size_t ib = FindClass(b);
    if (ia == ib)
        return std::nullopt;

    EqClass& ca = *classes_[ia];
    EqClass& cb = *classes_[ib];

    if (ca.bound_type && cb.bound_type) {
        for (const auto* m : cb.members) {
            if (ca.bound_type->Contains([m](const ast::Type& t) {
                    return dynamic_cast<const ast::TypeAuto*>(&t) == m;
                }))
                return ErrorCode::ERROR_OCCURS_CHECK_INFINITE_TYPE;
        }
        for (const auto* m : ca.members) {
            if (cb.bound_type->Contains([m](const ast::Type& t) {
                    return dynamic_cast<const ast::TypeAuto*>(&t) == m;
                }))
                return ErrorCode::ERROR_OCCURS_CHECK_INFINITE_TYPE;
        }

        if (auto error = ca.bound_type->CheckCompatible(*cb.bound_type)) {
            return *error;
        }

        std::optional<ErrorCode> ca_family = ca.bound_type->GetFamilyErrorCode();
        std::optional<ErrorCode> cb_family = cb.bound_type->GetFamilyErrorCode();

        if (ca_family && cb_family && *ca_family != *cb_family) {
            return *ca_family;
        }

        std::optional<ErrorCode> ca_unexpected = ca.bound_type->GetUnexpectedErrorCode();
        std::optional<ErrorCode> cb_unexpected = cb.bound_type->GetUnexpectedErrorCode();

        if (ca_unexpected && cb_unexpected && *ca_unexpected != *cb_unexpected) {
            return *ca_unexpected;
        }
    }

    for (const auto* m : cb.members) {
        ca.members.insert(m);
        class_of_[m] = ia;
    }
    if (!ca.bound_type && cb.bound_type)
        ca.bound_type = std::move(cb.bound_type);

    classes_[ib] = std::make_unique<EqClass>();

    return std::nullopt;
}

std::optional<ErrorCode> Unifier::BindClass(const ast::TypeAuto* var,
                                            std::shared_ptr<const ast::Type> concrete) {
    EqClass& cls = ClassOf(var);

    if (cls.bound_type) {
        return std::nullopt;
    }

    for (const auto* m : cls.members) {
        if (concrete->Contains(
                [m](const ast::Type& t) { return dynamic_cast<const ast::TypeAuto*>(&t) == m; }))
            return ErrorCode::ERROR_OCCURS_CHECK_INFINITE_TYPE;
    }

    cls.bound_type = std::move(concrete);
    return std::nullopt;
}

std::optional<ErrorCode> Unifier::UnifyOne(std::shared_ptr<const ast::Type> lhs,
                                           std::shared_ptr<const ast::Type> rhs) {
    const auto* lvar = dynamic_cast<const ast::TypeAuto*>(lhs.get());
    const auto* rvar = dynamic_cast<const ast::TypeAuto*>(rhs.get());

    if (lvar && rvar) {
        return MergeClasses(lvar, rvar);
    }

    if (lvar) {
        return BindClass(lvar, std::move(rhs));
    }

    if (rvar) {
        return BindClass(rvar, std::move(lhs));
    }

    return std::nullopt;
}

} // namespace typecheck
} // namespace stella
