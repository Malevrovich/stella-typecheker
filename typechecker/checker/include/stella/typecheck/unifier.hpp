#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "stella/ast/auto.hpp"
#include "stella/ast/base.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/subtype.hpp"

namespace stella {
namespace typecheck {

class TypeChecker;

class Unifier final {
public:
    Unifier() = default;

    void AddConstraint(const ast::Type& lhs, const ast::Type& rhs) {
        AddConstraint(std::shared_ptr<const ast::Type>(std::shared_ptr<void>{}, &lhs),
                      std::shared_ptr<const ast::Type>(std::shared_ptr<void>{}, &rhs));
    }
    void AddConstraint(std::shared_ptr<const ast::Type> lhs, std::shared_ptr<const ast::Type> rhs);

    std::shared_ptr<const ast::TypeAuto> FreshTypeVar();
    void SaveNewType(std::shared_ptr<const ast::Type> type) { new_types_.push_back(type); }

    void UnifyAll(const SubtypeChecker* subtype_checker);

private:
    struct Constraint {
        std::shared_ptr<const ast::Type> lhs;
        std::shared_ptr<const ast::Type> rhs;
    };

    struct EqClass {
        std::unordered_set<const ast::TypeAuto*> members;
        std::shared_ptr<const ast::Type> bound_type;
    };

    EqClass& ClassOf(const ast::TypeAuto* var);

    void MergeClasses(const ast::TypeAuto* a, const ast::TypeAuto* b, const SubtypeChecker* sc);

    void BindClass(const ast::TypeAuto* var, std::shared_ptr<const ast::Type> concrete,
                   const SubtypeChecker* sc);

    std::size_t FindClass(const ast::TypeAuto* var);

    void UnifyOne(std::shared_ptr<const ast::Type> lhs, std::shared_ptr<const ast::Type> rhs,
                  const SubtypeChecker* sc);

    void ReportUnificationError(ErrorCode error_code, const ast::Type& lhs, const ast::Type& rhs);

    std::vector<Constraint> constraints_;
    std::vector<std::unique_ptr<EqClass>> classes_;
    std::unordered_map<const ast::TypeAuto*, std::size_t> class_of_;
    std::vector<std::shared_ptr<const ast::Type>> new_types_;
};

} // namespace typecheck
} // namespace stella
