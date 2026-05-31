#include "stella/typecheck/type_checker.hpp"

#include <format>
#include <memory>
#include <unordered_map>
#include <vector>

#include "stella/ast/ast.hpp"
#include "stella/ast/base.hpp"
#include "stella/ast/generic.hpp"
#include "stella/ast/fun.hpp"
#include "stella/ast/list.hpp"
#include "stella/ast/tuple.hpp"
#include "stella/ast/record.hpp"
#include "stella/ast/sum.hpp"
#include "stella/ast/variant.hpp"
#include "stella/ast/reference.hpp"
#include "stella/ast/top_bottom.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"
#include "stella/typecheck/name_context.hpp"

namespace stella {
namespace typecheck {

namespace {

using Subst = std::unordered_map<std::string, std::shared_ptr<const ast::Type>>;

std::shared_ptr<const ast::Type> SubstituteType(std::shared_ptr<const ast::Type> type,
                                                const Subst& subst) {
    if (subst.empty()) return type;

    if (const auto* tv = dynamic_cast<const ast::TypeVar*>(type.get())) {
        auto it = subst.find(std::string{tv->GetName()});
        if (it != subst.end()) return it->second;
        return type;
    }

    if (const auto* tf = dynamic_cast<const ast::TypeFun*>(type.get())) {
        if (tf->IsSentinel()) return type;
        return std::make_shared<ast::TypeFun>(SubstituteType(tf->GetArgType(), subst),
                                              SubstituteType(tf->GetReturnType(), subst));
    }

    if (const auto* tfa = dynamic_cast<const ast::TypeForAll*>(type.get())) {
        if (tfa->IsSentinel()) return type;
        // Remove shadowed params from subst
        Subst inner_subst = subst;
        for (const auto& param : tfa->GetTypeParams()) {
            inner_subst.erase(param);
        }
        return std::make_shared<ast::TypeForAll>(tfa->GetTypeParams(),
                                                 SubstituteType(tfa->GetBody(), inner_subst));
    }

    if (const auto* tl = dynamic_cast<const ast::TypeList*>(type.get())) {
        if (tl->IsSentinel()) return type;
        return std::make_shared<ast::TypeList>(SubstituteType(tl->GetElementType(), subst));
    }

    if (const auto* tt = dynamic_cast<const ast::TypeTuple*>(type.get())) {
        if (tt->IsSentinel()) return type;
        const auto& elems = tt->GetElementTypes();
        if (!elems) return type;
        std::vector<std::shared_ptr<const ast::Type>> new_elems;
        new_elems.reserve(elems->size());
        for (const auto& e : *elems) {
            new_elems.push_back(SubstituteType(e, subst));
        }
        return std::make_shared<ast::TypeTuple>(std::move(new_elems));
    }

    if (const auto* tr = dynamic_cast<const ast::TypeRecord*>(type.get())) {
        const auto& fields = tr->GetFields();
        if (!fields) return type; // sentinel
        std::vector<ast::TypeRecord::Field> new_fields;
        new_fields.reserve(fields->size());
        for (const auto& f : *fields) {
            new_fields.push_back({f.label, SubstituteType(f.type, subst)});
        }
        return std::make_shared<ast::TypeRecord>(std::move(new_fields));
    }

    if (const auto* ts = dynamic_cast<const ast::TypeSum*>(type.get())) {
        if (ts->IsSentinel()) return type;
        return std::make_shared<ast::TypeSum>(SubstituteType(ts->GetLeft(), subst),
                                              SubstituteType(ts->GetRight(), subst));
    }

    if (const auto* tv = dynamic_cast<const ast::TypeVariant*>(type.get())) {
        const auto& fields = tv->GetFields();
        if (!fields) return type; // sentinel
        std::vector<ast::TypeVariant::Field> new_fields;
        new_fields.reserve(fields->size());
        for (const auto& f : *fields) {
            std::optional<std::shared_ptr<const ast::Type>> new_type;
            if (f.type) {
                new_type = SubstituteType(*f.type, subst);
            }
            new_fields.push_back({f.label, new_type});
        }
        return std::make_shared<ast::TypeVariant>(std::move(new_fields));
    }

    if (const auto* tref = dynamic_cast<const ast::TypeRef*>(type.get())) {
        if (tref->IsSentinel()) return type;
        return std::make_shared<ast::TypeRef>(SubstituteType(tref->GetInnerType(), subst));
    }

    // Leaves: TypeNat, TypeBool, TypeUnit, TypeTop, TypeBottom, TypeUnknown, TypeAuto — no subst
    return type;
}

} // namespace

void TypeChecker::VisitTypeVar(const ast::TypeVar& type) {
    if (!type_var_context_.TryGet(type.GetName())) {
        OnError(TypeCheckTypeError{
            ErrorCode::ERROR_UNDEFINED_TYPE_VARIABLE,
            type,
            std::format("Undefined type variable '{}'", type.GetName()),
        });
    }
    // Types have no DeducedType — just validate.
}

void TypeChecker::VisitTypeForAll(const ast::TypeForAll& type) {
    if (type.IsSentinel()) return;

    // Push type params into scope, keeping the TypeVar objects alive in a local vector.
    std::vector<std::shared_ptr<ast::TypeVar>> type_var_nodes;
    std::vector<NameContext<const ast::TypeVar>::NameContextGuard> tv_guards;
    type_var_nodes.reserve(type.GetTypeParams().size());
    tv_guards.reserve(type.GetTypeParams().size());
    for (const auto& param : type.GetTypeParams()) {
        type_var_nodes.push_back(std::make_shared<ast::TypeVar>(param));
        tv_guards.push_back(type_var_context_.Push(param, *type_var_nodes.back()));
    }

    Visit(*type.GetBody());
    // tv_guards destruct → Pop params
}

void TypeChecker::VisitDeclFunGeneric(const ast::NodeDeclFunGeneric& node) {
    SetProvisionalType(node, {ast::TypeForAll::MakeSentinel()});

    const auto& type_params = node.GetTypeParams();

    // Push type params into type_var_context_
    std::vector<std::shared_ptr<ast::TypeVar>> type_var_nodes;
    std::vector<NameContext<const ast::TypeVar>::NameContextGuard> tv_guards;
    type_var_nodes.reserve(type_params.size());
    tv_guards.reserve(type_params.size());
    for (const auto& param : type_params) {
        type_var_nodes.push_back(std::make_shared<ast::TypeVar>(param));
        tv_guards.push_back(type_var_context_.Push(param, *type_var_nodes.back()));
    }

    Visit(*node.GetReturnType());

    const auto& abstraction = *node.GetAbstraction();
    const auto& param = abstraction.GetParam();
    Visit(*param->GetType());
    Visit(*param);

    // Pre-set DeducedType for recursive references to this function.
    {
        auto param_type = types_storage_.get<DeducedType>(param.get()).type;
        auto return_type = node.GetReturnType();
        auto fn_type = std::make_shared<const ast::TypeFun>(param_type, return_type);
        types_storage_.set<DeducedType>(
            &node, {std::make_shared<const ast::TypeForAll>(type_params, fn_type)});
    }

    auto name_guard = name_context_.Push(std::string{param->GetName()}, *param);

    const bool was_in_function_body = in_function_body_;
    in_function_body_ = true;

    for (const auto& local_decl : node.GetLocalDecls()) {
        Visit(*local_decl);
    }

    const auto& body = abstraction.GetBody();
    ExpectType(*body, ExpectedType::EqualsTo(node.GetReturnType()));
    Visit(*body);

    in_function_body_ = was_in_function_body;

    auto param_type = types_storage_.get<DeducedType>(param.get()).type;
    auto body_type = types_storage_.get<DeducedType>(body.get()).type;
    auto fn_type = std::make_shared<const ast::TypeFun>(std::move(param_type), std::move(body_type));
    SetDeducedType(node, {std::make_shared<const ast::TypeForAll>(type_params, std::move(fn_type))});

    // tv_guards destruct → Pop type params
}

void TypeChecker::VisitExprTypeAbstraction(const ast::NodeExprTypeAbstraction& node) {
    SetProvisionalType(node, {ast::TypeForAll::MakeSentinel()});

    const auto& type_params = node.GetTypeParams();

    std::vector<std::shared_ptr<ast::TypeVar>> type_var_nodes;
    std::vector<NameContext<const ast::TypeVar>::NameContextGuard> tv_guards;
    type_var_nodes.reserve(type_params.size());
    tv_guards.reserve(type_params.size());
    for (const auto& param : type_params) {
        type_var_nodes.push_back(std::make_shared<ast::TypeVar>(param));
        tv_guards.push_back(type_var_context_.Push(param, *type_var_nodes.back()));
    }

    const auto& body = node.GetBody();
    Visit(*body);

    auto body_type = types_storage_.get<DeducedType>(body.get()).type;
    SetDeducedType(node,
                   {std::make_shared<const ast::TypeForAll>(type_params, std::move(body_type))});

    // tv_guards destruct → Pop type params
}

void TypeChecker::VisitExprTypeApplication(const ast::NodeExprTypeApplication& node) {
    const auto& function = node.GetFunction();
    ExpectType(*function, ExpectedType::EqualsTo(ast::TypeForAll::MakeSentinel(),
                                                 ErrorCode::ERROR_NOT_A_GENERIC_FUNCTION));
    Visit(*function);

    const auto deduced = types_storage_.get<DeducedType>(function.get()).type;
    const auto forall = std::dynamic_pointer_cast<const ast::TypeForAll>(deduced);
    if (!forall || forall->IsSentinel()) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_NOT_A_GENERIC_FUNCTION,
            node,
            std::format("Expected a generic function but got {}", deduced->ToString()),
        });
    }

    const auto& type_params = forall->GetTypeParams();
    const auto& type_args = node.GetTypeArgs();

    if (type_args.size() != type_params.size()) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_INCORRECT_NUMBER_OF_TYPE_ARGUMENTS,
            node,
            std::format("Expected {} type arguments for the expression\n  {} as {}\nbut got {} "
                        "type arguments\nin the type application",
                        type_params.size(), function->ToString(), forall->ToString(),
                        type_args.size()),
        });
    }

    // Validate TypeVar references inside the type arguments themselves.
    for (const auto& type_arg : type_args) {
        Visit(*type_arg);
    }

    // Build substitution map: T → type_args[i]
    Subst subst;
    for (std::size_t i = 0; i < type_params.size(); ++i) {
        subst[type_params[i]] = type_args[i];
    }

    auto instantiated = SubstituteType(forall->GetBody(), subst);
    SetDeducedType(node, {std::move(instantiated)});
}

} // namespace typecheck
} // namespace stella
