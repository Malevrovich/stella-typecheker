#pragma once

#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "stella/ast/auto.hpp"
#include "stella/ast/base.hpp"
#include "stella/typecheck/error.hpp"

namespace stella {
namespace typecheck {

class Unifier final {
public:
    Unifier() = default;

    void AddConstraint(std::shared_ptr<const ast::Type> lhs, std::shared_ptr<const ast::Type> rhs);

    void AddConstraint(const ast::Type& lhs, const ast::Type& rhs) {
        AddConstraint(std::shared_ptr<const ast::Type>(std::shared_ptr<void>{}, &lhs),
                      std::shared_ptr<const ast::Type>(std::shared_ptr<void>{}, &rhs));
    }

    std::shared_ptr<const ast::TypeAuto> FreshTypeVar();

    std::optional<ErrorCode> UnifyAll();

private:
    struct EqClass {
        std::unordered_set<const ast::TypeAuto*> members;
        std::shared_ptr<const ast::Type> bound_type;
    };

    EqClass& ClassOf(const ast::TypeAuto* var);

    std::optional<ErrorCode> MergeClasses(const ast::TypeAuto* a, const ast::TypeAuto* b);

    std::optional<ErrorCode> BindClass(const ast::TypeAuto* var,
                                       std::shared_ptr<const ast::Type> concrete);

    std::size_t FindClass(const ast::TypeAuto* var);

    std::optional<ErrorCode> UnifyOne(std::shared_ptr<const ast::Type> lhs,
                                      std::shared_ptr<const ast::Type> rhs);

    uint64_t next_type_var_id_ = 0;

    std::vector<std::pair<std::shared_ptr<const ast::Type>, std::shared_ptr<const ast::Type>>>
        constraints_;

    std::vector<std::unique_ptr<EqClass>> classes_;

    std::unordered_map<const ast::TypeAuto*, std::size_t> class_of_;
};

} // namespace typecheck
} // namespace stella
