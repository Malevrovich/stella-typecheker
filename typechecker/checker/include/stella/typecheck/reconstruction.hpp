#pragma once

#include <optional>

#include "stella/ast/auto.hpp"
#include "stella/ast/base.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/subtype.hpp"
#include "stella/typecheck/unifier.hpp"

namespace stella {
namespace typecheck {

class ReconstructionComparator {
public:
    explicit ReconstructionComparator(Unifier& unifier,
                                      const SubtypeChecker* subtype_checker = nullptr)
        : unifier_(unifier),
          subtype_checker_(subtype_checker) {}

    std::optional<ErrorCode> operator()(const ast::Type& a, const ast::Type& b) const {
        const bool a_is_auto = dynamic_cast<const ast::TypeAuto*>(&a) != nullptr;
        const bool b_is_auto = dynamic_cast<const ast::TypeAuto*>(&b) != nullptr;

        if (a_is_auto || b_is_auto) {
            unifier_.AddConstraint(a, b);
            return std::nullopt;
        }

        if (subtype_checker_) {
            return subtype_checker_->IsSubtypeOrError(a, b);
        }
        return a.CheckCompatible(b, std::cref(*this));
    }

private:
    Unifier& unifier_;
    const SubtypeChecker* subtype_checker_; // non-owning; null if no subtyping
};

} // namespace typecheck
} // namespace stella
