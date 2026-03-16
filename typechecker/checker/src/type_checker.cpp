#include "stella/typecheck/type_checker.hpp"

#include <memory>

#include "stella/ast/visitor.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/name_context.hpp"

namespace stella {
namespace typecheck {

namespace {
constexpr std::string_view kMain = "main";
}

void TypeChecker::VisitProgram(const ast::NodeProgram& node) {
    std::vector<NameContext::NameContextGuard> name_guards;
    name_guards.reserve(node.GetDeclarations().size());

    bool main_found = false;
    for (auto& decl : node.GetDeclarations()) {
        auto func_decl = std::dynamic_pointer_cast<ast::NodeDeclFun>(decl);

        if (func_decl->GetName() == kMain) {
            main_found = true;
        }

        name_guards.push_back(
            name_context_.PushUnique(std::string{func_decl->GetName()}, *func_decl));

        ExpectType(*func_decl, func_decl->GetReturnType(), [](const ast::Type&, const ast::Type&) {
            throw InternalTypeCheckError("Unexpected type conflict");
        });
    }

    if (!main_found) {
        throw TypeCheckError(ErrorCode::ERROR_MISSING_MAIN);
    }

    for (auto& decl : node.GetDeclarations()) {
        ast::BaseNodeVisitor::Visit(*decl);
    }
}

void TypeChecker::VisitDeclFun(const ast::NodeDeclFun& node) {}

} // namespace typecheck
} // namespace stella