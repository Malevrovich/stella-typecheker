#include "stella/typecheck/type_checker.hpp"

#include <memory>

#include "stella/ast/panic.hpp"
#include "stella/ast/top_bottom.hpp"
#include "stella/typecheck/error.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitExprPanic(const ast::NodeExprPanic& node) {
    const auto expected_type = TryGetExpectedType<ast::Type>(node);
    if (!expected_type) {
        SetAmbiguousOrError(node, ast::TypeBottom::Get(), ErrorCode::ERROR_AMBIGUOUS_PANIC_TYPE,
                            "Cannot determine type of panic!: no expected type provided");
        return;
    }

    SetDeducedType(node, {expected_type});
}

} // namespace typecheck
} // namespace stella
