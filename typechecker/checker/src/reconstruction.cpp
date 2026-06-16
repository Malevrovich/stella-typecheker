#include "stella/typecheck/reconstruction.hpp"

namespace stella::typecheck {

ReconstructionComparator::ReconstructionComparator(Unifier& unifier, const ast::NodeBase* ctx,
                                                   const SubtypeChecker* subtype_checker)
    : ctx_node_(ctx),
      unifier_(unifier),
      subtype_checker_(subtype_checker) {}

std::optional<ErrorCode> ReconstructionComparator::operator()(const ast::Type& a,
                                                              const ast::Type& b) const {
    const bool a_is_auto = dynamic_cast<const ast::TypeAuto*>(&a) != nullptr;
    const bool b_is_auto = dynamic_cast<const ast::TypeAuto*>(&b) != nullptr;

    if (a_is_auto || b_is_auto) {
        unifier_.AddConstraint(a, b, ctx_node_);
        return std::nullopt;
    }

    if (subtype_checker_) {
        return subtype_checker_->IsSubtypeOrError(a, b);
    }
    return a.CheckCompatible(b, std::cref(*this));
}

} // namespace stella::typecheck