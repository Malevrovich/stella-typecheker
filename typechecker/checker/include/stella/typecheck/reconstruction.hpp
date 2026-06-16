#pragma once

#include <optional>

#include "stella/ast/base.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/subtype.hpp"
#include "stella/typecheck/unifier.hpp"

namespace stella {
namespace typecheck {

class ReconstructionComparator {
public:
    explicit ReconstructionComparator(Unifier& unifier, const ast::NodeBase* ctx = nullptr,
                                      const SubtypeChecker* subtype_checker = nullptr);

    std::optional<ErrorCode> operator()(const ast::Type& a, const ast::Type& b) const;

private:
    const ast::NodeBase* ctx_node_;
    Unifier& unifier_;
    const SubtypeChecker* subtype_checker_; // non-owning; null if no subtyping
};

} // namespace typecheck
} // namespace stella
