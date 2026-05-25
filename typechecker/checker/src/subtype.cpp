#include "stella/typecheck/subtype.hpp"

#include <algorithm>
#include <optional>

#include "stella/ast/base.hpp"
#include "stella/ast/fun.hpp"
#include "stella/ast/list.hpp"
#include "stella/ast/record.hpp"
#include "stella/ast/reference.hpp"
#include "stella/ast/sum.hpp"
#include "stella/ast/top_bottom.hpp"
#include "stella/ast/tuple.hpp"
#include "stella/ast/variant.hpp"
#include "stella/typecheck/error.hpp"

namespace stella {
namespace typecheck {

// Sentinel value meaning "this rule does not apply, try the next one".
static constexpr auto kNoMatch = ErrorCode::ERROR_UNEXPECTED_SUBTYPE;

std::optional<ErrorCode> SubtypeChecker::IsSubtypeOrError(const ast::Type& sub,
                                                          const ast::Type& super) const {
    if (auto err = RuleUnknown(sub, super); !err)
        return std::nullopt;
    if (auto err = RuleBot(sub, super); !err)
        return std::nullopt;
    if (auto err = RuleTop(sub, super); !err)
        return std::nullopt;
    if (auto err = RuleReflexivity(sub, super); !err)
        return std::nullopt;
    if (auto err = RuleFun(sub, super); err != kNoMatch)
        return err;
    if (auto err = RuleList(sub, super); err != kNoMatch)
        return err;
    if (auto err = RuleTuple(sub, super); err != kNoMatch)
        return err;
    if (auto err = RuleRecord(sub, super); err != kNoMatch)
        return err;
    if (auto err = RuleVariant(sub, super); err != kNoMatch)
        return err;
    if (auto err = RuleSum(sub, super); err != kNoMatch)
        return err;
    if (auto err = RuleRef(sub, super); err != kNoMatch)
        return err;
    return ErrorCode::ERROR_UNEXPECTED_SUBTYPE;
}

bool SubtypeChecker::IsSubtype(const ast::Type& sub, const ast::Type& super) const {
    return !IsSubtypeOrError(sub, super).has_value();
}

// Returns nullopt (success) when either type is Unknown — we treat Unknown as
// compatible with everything.
std::optional<ErrorCode> SubtypeChecker::RuleUnknown(const ast::Type& sub,
                                                     const ast::Type& super) const {
    if (dynamic_cast<const ast::TypeUnknown*>(&sub) ||
        dynamic_cast<const ast::TypeUnknown*>(&super))
        return std::nullopt;
    return kNoMatch;
}

// Bottom is a subtype of everything.
std::optional<ErrorCode> SubtypeChecker::RuleBot(const ast::Type& sub,
                                                 const ast::Type& /*super*/) const {
    if (dynamic_cast<const ast::TypeBottom*>(&sub))
        return std::nullopt;
    return kNoMatch;
}

// Everything is a subtype of Top.
std::optional<ErrorCode> SubtypeChecker::RuleTop(const ast::Type& /*sub*/,
                                                 const ast::Type& super) const {
    if (dynamic_cast<const ast::TypeTop*>(&super))
        return std::nullopt;
    return kNoMatch;
}

// Structural equality: sub and super are the same concrete type.
std::optional<ErrorCode> SubtypeChecker::RuleReflexivity(const ast::Type& sub,
                                                         const ast::Type& super) const {
    if (!sub.CheckCompatible(super).has_value())
        return std::nullopt;
    return kNoMatch;
}

// (T1 -> T2) <: (S1 -> S2)  iff  S1 <: T1  and  T2 <: S2
std::optional<ErrorCode> SubtypeChecker::RuleFun(const ast::Type& sub,
                                                 const ast::Type& super) const {
    const auto* sub_fun = dynamic_cast<const ast::TypeFun*>(&sub);
    const auto* super_fun = dynamic_cast<const ast::TypeFun*>(&super);
    if (!sub_fun || !super_fun)
        return kNoMatch;
    if (super_fun->IsSentinel())
        return std::nullopt; // any function <: (? -> ?)
    if (sub_fun->IsSentinel())
        return ErrorCode::ERROR_UNEXPECTED_SUBTYPE;
    // contravariant argument, covariant return
    if (auto err = IsSubtypeOrError(*super_fun->GetArgType(), *sub_fun->GetArgType()))
        return err;
    return IsSubtypeOrError(*sub_fun->GetReturnType(), *super_fun->GetReturnType());
}

// [T] <: [S]  iff  T <: S
std::optional<ErrorCode> SubtypeChecker::RuleList(const ast::Type& sub,
                                                  const ast::Type& super) const {
    const auto* sub_list = dynamic_cast<const ast::TypeList*>(&sub);
    const auto* super_list = dynamic_cast<const ast::TypeList*>(&super);
    if (!sub_list || !super_list)
        return kNoMatch;
    if (super_list->IsSentinel())
        return std::nullopt; // any list <: [_]
    if (sub_list->IsSentinel())
        return ErrorCode::ERROR_UNEXPECTED_SUBTYPE;
    return IsSubtypeOrError(*sub_list->GetElementType(), *super_list->GetElementType());
}

// {T1, ..., Tn, ...} <: {S1, ..., Sm}  iff  n >= m  and  Ti <: Si for i <= m
std::optional<ErrorCode> SubtypeChecker::RuleTuple(const ast::Type& sub,
                                                   const ast::Type& super) const {
    const auto* sub_tuple = dynamic_cast<const ast::TypeTuple*>(&sub);
    const auto* super_tuple = dynamic_cast<const ast::TypeTuple*>(&super);
    if (!sub_tuple || !super_tuple)
        return kNoMatch;
    if (super_tuple->IsSentinel())
        return std::nullopt; // any tuple <: {_}
    if (sub_tuple->IsSentinel())
        return ErrorCode::ERROR_UNEXPECTED_SUBTYPE;
    const auto& sub_elems = *sub_tuple->GetElementTypes();
    const auto& super_elems = *super_tuple->GetElementTypes();
    if (sub_elems.size() != super_elems.size())
        return ErrorCode::ERROR_UNEXPECTED_TUPLE_LENGTH;
    for (std::size_t i = 0; i < super_elems.size(); ++i) {
        if (auto err = IsSubtypeOrError(*sub_elems[i], *super_elems[i]))
            return err;
    }
    return std::nullopt;
}

// {l1: T1, ..., ln: Tn, ...} <: {l1: S1, ..., lm: Sm}
// (sub must have all fields of super with compatible types)
std::optional<ErrorCode> SubtypeChecker::RuleRecord(const ast::Type& sub,
                                                    const ast::Type& super) const {
    const auto* sub_rec = dynamic_cast<const ast::TypeRecord*>(&sub);
    const auto* super_rec = dynamic_cast<const ast::TypeRecord*>(&super);
    if (!sub_rec || !super_rec)
        return kNoMatch;
    if (super_rec->IsSentinel())
        return std::nullopt; // any record <: {_ : _}
    if (sub_rec->IsSentinel())
        return ErrorCode::ERROR_UNEXPECTED_SUBTYPE;
    const auto& sub_fields = *sub_rec->GetFields();
    const auto& super_fields = *super_rec->GetFields();
    for (const auto& super_field : super_fields) {
        auto it = std::find_if(sub_fields.begin(), sub_fields.end(),
                               [&](const auto& f) { return f.label == super_field.label; });
        if (it == sub_fields.end())
            return ErrorCode::ERROR_MISSING_RECORD_FIELDS;
        if (auto err = IsSubtypeOrError(*it->type, *super_field.type))
            return err;
    }
    return std::nullopt;
}

// <|l1: T1, ..., ln: Tn|> <: <|l1: S1, ..., lm: Sm, ...|>
// (all labels of sub must be present in super with compatible types)
std::optional<ErrorCode> SubtypeChecker::RuleVariant(const ast::Type& sub,
                                                     const ast::Type& super) const {
    const auto* sub_var = dynamic_cast<const ast::TypeVariant*>(&sub);
    const auto* super_var = dynamic_cast<const ast::TypeVariant*>(&super);
    if (!sub_var || !super_var)
        return kNoMatch;
    if (super_var->IsSentinel())
        return std::nullopt; // any variant <: <|_ : _|>
    if (sub_var->IsSentinel())
        return ErrorCode::ERROR_UNEXPECTED_SUBTYPE;
    const auto& sub_fields = *sub_var->GetFields();
    const auto& super_fields = *super_var->GetFields();
    for (const auto& sub_field : sub_fields) {
        auto it = std::find_if(super_fields.begin(), super_fields.end(),
                               [&](const auto& f) { return f.label == sub_field.label; });
        if (it == super_fields.end())
            return ErrorCode::ERROR_UNEXPECTED_VARIANT_LABEL;
        if (sub_field.type && it->type) {
            if (auto err = IsSubtypeOrError(**sub_field.type, **it->type))
                return err;
        }
    }
    return std::nullopt;
}

// (T1 + T2) <: (S1 + S2)  iff  T1 <: S1  and  T2 <: S2
std::optional<ErrorCode> SubtypeChecker::RuleSum(const ast::Type& sub,
                                                 const ast::Type& super) const {
    const auto* sub_sum = dynamic_cast<const ast::TypeSum*>(&sub);
    const auto* super_sum = dynamic_cast<const ast::TypeSum*>(&super);
    if (!sub_sum || !super_sum)
        return kNoMatch;
    if (super_sum->IsSentinel())
        return std::nullopt; // any sum <: (_ + _)
    if (sub_sum->IsSentinel())
        return ErrorCode::ERROR_UNEXPECTED_SUBTYPE;
    if (auto err = IsSubtypeOrError(*sub_sum->GetLeft(), *super_sum->GetLeft()))
        return err;
    return IsSubtypeOrError(*sub_sum->GetRight(), *super_sum->GetRight());
}

// &T <: &S  iff  T <: S  and  S <: T  (invariant)
std::optional<ErrorCode> SubtypeChecker::RuleRef(const ast::Type& sub,
                                                 const ast::Type& super) const {
    const auto* sub_ref = dynamic_cast<const ast::TypeRef*>(&sub);
    const auto* super_ref = dynamic_cast<const ast::TypeRef*>(&super);
    if (!sub_ref || !super_ref)
        return kNoMatch;
    if (super_ref->IsSentinel())
        return std::nullopt; // any ref <: &_
    if (sub_ref->IsSentinel())
        return ErrorCode::ERROR_UNEXPECTED_SUBTYPE;
    // references are invariant: inner types must be equal (or Unknown)
    if (auto err = IsSubtypeOrError(*sub_ref->GetInnerType(), *super_ref->GetInnerType()))
        return err;
    return IsSubtypeOrError(*super_ref->GetInnerType(), *sub_ref->GetInnerType());
}

} // namespace typecheck
} // namespace stella
