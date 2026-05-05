#include "stella/fuzzing/proto_to_stella.h"

#include <cstdlib>
#include <functional>
#include <sstream>
#include <string>

namespace stella {

// ─── Identifier sanitisation ─────────────────────────────────────────────────
// Maps an arbitrary proto string to one of 10 short, collision-friendly names.
// This makes the fuzzer produce identifier collisions much more often, which
// exercises the type-checker's name-resolution and shadowing logic.
std::string SanitiseIdent(const std::string& raw) {
    if (raw.empty())
        return "v0";
    std::size_t h = std::hash<std::string>{}(raw) % 10;
    return "v" + std::to_string(h);
}

// ─── Extension name sanitisation ─────────────────────────────────────────────
// ExtensionName lexer rule: '#' ([\-_] | (DIGIT | LETTER))+
// Maps an arbitrary proto string to one of a small set of valid extension names.
static std::string SanitiseExtension(const std::string& raw) {
    // A fixed pool of valid Stella extension names.
    static const char* kExtensions[] = {
        "#structural-patterns",
        "#natural-literals",
        "#unit-type",
        "#pairs",
        "#tuples",
        "#records",
        "#sum-types",
        "#variants",
        "#lists",
        "#type-ascriptions",
    };
    static constexpr std::size_t kNumExtensions = sizeof(kExtensions) / sizeof(kExtensions[0]);
    if (raw.empty())
        return kExtensions[0];
    std::size_t h = std::hash<std::string>{}(raw) % kNumExtensions;
    return kExtensions[h];
}

// ─── Safe terminal tokens ─────────────────────────────────────────────────────
static const char* kSafeExpr = "0";
static const char* kSafeType = "Nat";
static const char* kSafePattern = "v0";

// ─── Join helper ──────────────────────────────────────────────────────────────
template <typename Container, typename Fn>
static std::string Join(const Container& items, const std::string& sep, Fn fn) {
    std::string result;
    bool first = true;
    for (const auto& item : items) {
        if (!first)
            result += sep;
        result += fn(item);
        first = false;
    }
    return result;
}

// ─── Program ──────────────────────────────────────────────────────────────────
std::string ToString(const Program& msg, int depth) {
    std::string out;
    out += "language core;\n";
    for (const auto& ext : msg.extensions()) {
        out += "extend with " + SanitiseExtension(ext) + ";\n";
    }
    for (const auto& decl : msg.decls()) {
        out += ToString(decl, depth + 1) + "\n";
    }
    return out;
}

// ─── Declarations ─────────────────────────────────────────────────────────────
std::string ToString(const Decl& msg, int depth) {
    switch (msg.decl_kind_case()) {
    case Decl::kFunDecl:
        return ToString(msg.fun_decl(), depth);
    // kFunGeneric, kTypeAlias, kExceptionType, kExceptionVariant are not yet
    // supported by the parser's AST builder (visitChildren fires FATAL).
    // Fall through to the safe fallback for all unimplemented declaration kinds.
    default:
        return "fn v0(n : Nat) -> Nat { return n }";
    }
}

std::string ToString(const DeclFun& msg, int depth) {
    std::string out;
    for (const auto& ann : msg.annotations()) {
        std::string a = ToString(ann, depth);
        if (!a.empty())
            out += a + " ";
    }
    out += "fn " + SanitiseIdent(msg.name()) + "(";
    if (msg.param_decls().empty()) {
        out += "n : Nat";
    } else {
        out += Join(msg.param_decls(), ", ",
                    [&](const ParamDecl& p) { return ToString(p, depth + 1); });
    }
    out += ")";
    if (msg.has_return_type()) {
        out += " -> " + ToString(msg.return_type(), depth + 1);
    }
    if (!msg.throw_types().empty()) {
        out += " throws ";
        out += Join(msg.throw_types(), ", ",
                    [&](const StellaType& t) { return ToString(t, depth + 1); });
    }
    out += " {\n";
    for (const auto& local : msg.local_decls()) {
        out += "  " + ToString(local, depth + 1) + "\n";
    }
    out += "  return " + ToString(msg.return_expr(), depth + 1) + "\n}";
    return out;
}

std::string ToString(const DeclFunGeneric& msg, int depth) {
    std::string out;
    for (const auto& ann : msg.annotations()) {
        std::string a = ToString(ann, depth);
        if (!a.empty())
            out += a + " ";
    }
    out += "generic fn " + SanitiseIdent(msg.name()) + "[";
    // Grammar requires at least one generic type parameter.
    if (msg.generics().empty()) {
        out += "T";
    } else {
        out += Join(msg.generics(), ", ", [&](const std::string& g) { return SanitiseIdent(g); });
    }
    out += "](";
    if (msg.param_decls().empty()) {
        out += "n : Nat";
    } else {
        out += Join(msg.param_decls(), ", ",
                    [&](const ParamDecl& p) { return ToString(p, depth + 1); });
    }
    out += ")";
    if (msg.has_return_type()) {
        out += " -> " + ToString(msg.return_type(), depth + 1);
    }
    if (!msg.throw_types().empty()) {
        out += " throws ";
        out += Join(msg.throw_types(), ", ",
                    [&](const StellaType& t) { return ToString(t, depth + 1); });
    }
    out += " {\n";
    for (const auto& local : msg.local_decls()) {
        out += "  " + ToString(local, depth + 1) + "\n";
    }
    out += "  return " + ToString(msg.return_expr(), depth + 1) + "\n}";
    return out;
}

std::string ToString(const DeclTypeAlias& msg, int depth) {
    return "type " + SanitiseIdent(msg.name()) + " = " + ToString(msg.atype(), depth + 1);
}

std::string ToString(const DeclExceptionType& msg, int depth) {
    return "exception type = " + ToString(msg.exception_type(), depth + 1);
}

std::string ToString(const DeclExceptionVariant& msg, int depth) {
    return "exception variant " + SanitiseIdent(msg.name()) + " : " +
           ToString(msg.variant_type(), depth + 1);
}

// ─── Annotation ───────────────────────────────────────────────────────────────
std::string ToString(const Annotation& msg, int /*depth*/) {
    return msg.inline_ann() ? "inline" : "";
}

// ─── ParamDecl ────────────────────────────────────────────────────────────────
std::string ToString(const ParamDecl& msg, int depth) {
    return SanitiseIdent(msg.name()) + " : " + ToString(msg.param_type(), depth + 1);
}

// ─── PatternBinding ───────────────────────────────────────────────────────────
std::string ToString(const PatternBinding& msg, int depth) {
    return ToString(msg.pat(), depth) + " = " + ToString(msg.rhs(), depth);
}

// ─── Binding ──────────────────────────────────────────────────────────────────
std::string ToString(const Binding& msg, int depth) {
    return SanitiseIdent(msg.name()) + " = " + ToString(msg.rhs(), depth);
}

// ─── MatchCase ────────────────────────────────────────────────────────────────
std::string ToString(const MatchCase& msg, int depth) {
    return ToString(msg.pattern(), depth) + " => " + ToString(msg.expr(), depth);
}

// ─── LabelledPattern ──────────────────────────────────────────────────────────
std::string ToString(const LabelledPattern& msg, int depth) {
    return SanitiseIdent(msg.label()) + " = " + ToString(msg.pattern(), depth);
}

// ─── RecordFieldType ──────────────────────────────────────────────────────────
std::string ToString(const RecordFieldType& msg, int depth) {
    return SanitiseIdent(msg.label()) + " : " + ToString(msg.type(), depth + 1);
}

// ─── VariantFieldType ─────────────────────────────────────────────────────────
std::string ToString(const VariantFieldType& msg, int depth) {
    std::string out = SanitiseIdent(msg.label());
    if (msg.has_type()) {
        out += " : " + ToString(msg.type(), depth + 1);
    }
    return out;
}

// ─── Expressions ──────────────────────────────────────────────────────────────
std::string ToString(const Expr& msg, int depth) {
    if (depth > kMaxDepth)
        return kSafeExpr;

    switch (msg.expr_kind_case()) {
    case Expr::kConstTrue:
        return "true";
    case Expr::kConstFalse:
        return "false";
    case Expr::kConstUnit:
        return "unit";
    case Expr::kConstInt:
        return ToString(msg.const_int(), depth);
    case Expr::kConstMemory:
        return ToString(msg.const_memory(), depth);
    case Expr::kVar:
        return ToString(msg.var(), depth);
    case Expr::kDotRecord:
        return ToString(msg.dot_record(), depth);
    case Expr::kDotTuple:
        return ToString(msg.dot_tuple(), depth);
    case Expr::kIfExpr:
        return ToString(msg.if_expr(), depth);
    case Expr::kSequence:
        return ToString(msg.sequence(), depth);
    case Expr::kAssign:
        return ToString(msg.assign(), depth);
    case Expr::kLetExpr:
        return ToString(msg.let_expr(), depth);
    case Expr::kLetrecExpr:
        return ToString(msg.letrec_expr(), depth);
    case Expr::kAbstraction:
        return ToString(msg.abstraction(), depth);
    case Expr::kApplication:
        return ToString(msg.application(), depth);
    case Expr::kTypeAbstraction:
        return ToString(msg.type_abstraction(), depth);
    case Expr::kTypeApplication:
        return ToString(msg.type_application(), depth);
    case Expr::kBinaryOp:
        return ToString(msg.binary_op(), depth);
    case Expr::kUnaryOp:
        return ToString(msg.unary_op(), depth);
    case Expr::kTypeAsc:
        return ToString(msg.type_asc(), depth);
    case Expr::kTypeCast:
        return ToString(msg.type_cast(), depth);
    case Expr::kInl:
        return ToString(msg.inl(), depth);
    case Expr::kInr:
        return ToString(msg.inr(), depth);
    case Expr::kConsList:
        return ToString(msg.cons_list(), depth);
    case Expr::kHead:
        return ToString(msg.head(), depth);
    case Expr::kIsEmpty:
        return ToString(msg.is_empty(), depth);
    case Expr::kTail:
        return ToString(msg.tail(), depth);
    case Expr::kListLiteral:
        return ToString(msg.list_literal(), depth);
    case Expr::kSucc:
        return ToString(msg.succ(), depth);
    case Expr::kPred:
        return ToString(msg.pred(), depth);
    case Expr::kIsZero:
        return ToString(msg.is_zero(), depth);
    case Expr::kNatRec:
        return ToString(msg.nat_rec(), depth);
    case Expr::kFold:
        return ToString(msg.fold(), depth);
    case Expr::kUnfold:
        return ToString(msg.unfold(), depth);
    case Expr::kRef:
        return ToString(msg.ref(), depth);
    case Expr::kDeref:
        return ToString(msg.deref(), depth);
    case Expr::kTuple:
        return ToString(msg.tuple(), depth);
    case Expr::kRecord:
        return ToString(msg.record(), depth);
    case Expr::kVariant:
        return ToString(msg.variant(), depth);
    case Expr::kMatch:
        return ToString(msg.match(), depth);
    case Expr::kPanic:
        return "panic!";
    case Expr::kThrowExpr:
        return ToString(msg.throw_expr(), depth);
    case Expr::kTryCatch:
        return ToString(msg.try_catch(), depth);
    case Expr::kTryCastAs:
        return ToString(msg.try_cast_as(), depth);
    case Expr::kTryWith:
        return ToString(msg.try_with(), depth);
    case Expr::kFix:
        return ToString(msg.fix(), depth);
    case Expr::kParenthesised:
        return ToString(msg.parenthesised(), depth);
    case Expr::kTerminatingSemicolon:
        return ToString(msg.terminating_semicolon(), depth);
    default:
        return kSafeExpr;
    }
}

// ── Atom expressions ──────────────────────────────────────────────────────────
std::string ToString(const ConstInt& msg, int /*depth*/) {
    // INTEGER token is DIGIT+ — only non-negative values are valid.
    return std::to_string(std::abs(msg.value()));
}

std::string ToString(const ConstMemory& msg, int /*depth*/) {
    // Grammar token: MemoryAddress = '<0x' (DIGIT | [A-F] | [a-f])+ '>'
    // Sanitise: keep only valid hex chars; fall back to "0" if empty.
    std::string hex;
    for (unsigned char c : msg.address()) {
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
            hex += static_cast<char>(c);
        }
    }
    if (hex.empty())
        hex = "0";
    return "<0x" + hex + ">";
}

std::string ToString(const Var& msg, int /*depth*/) { return SanitiseIdent(msg.name()); }

// ── Field / index access ──────────────────────────────────────────────────────
std::string ToString(const DotRecord& msg, int depth) {
    return ToString(msg.expr(), depth + 1) + "." + SanitiseIdent(msg.label());
}

std::string ToString(const DotTuple& msg, int depth) {
    // Tuple index is an INTEGER token (DIGIT+) — must be non-negative.
    // Use abs() and clamp to at least 1 (tuple indices start at 1 in Stella).
    int idx = std::abs(msg.index());
    if (idx == 0)
        idx = 1;
    return ToString(msg.expr(), depth + 1) + "." + std::to_string(idx);
}

// ── Control flow ──────────────────────────────────────────────────────────────
std::string ToString(const If& msg, int depth) {
    return "if " + ToString(msg.condition(), depth + 1) + " then " +
           ToString(msg.then_expr(), depth + 1) + " else " + ToString(msg.else_expr(), depth + 1);
}

std::string ToString(const Sequence& msg, int depth) {
    return ToString(msg.expr1(), depth + 1) + "; " + ToString(msg.expr2(), depth + 1);
}

std::string ToString(const Assign& msg, int depth) {
    return ToString(msg.lhs(), depth + 1) + " := " + ToString(msg.rhs(), depth + 1);
}

// ── Let / letrec ─────────────────────────────────────────────────────────────
std::string ToString(const Let& msg, int depth) {
    // Grammar: 'let' patternBinding (',' patternBinding)* 'in' expr
    // Requires at least one binding; fall back to a safe binding when empty.
    std::string out = "let ";
    if (msg.pattern_bindings().empty()) {
        out += "v0 = 0";
    } else {
        out += Join(msg.pattern_bindings(), ", ",
                    [&](const PatternBinding& pb) { return ToString(pb, depth + 1); });
    }
    out += " in " + ToString(msg.body(), depth + 1);
    return out;
}

std::string ToString(const LetRec& msg, int depth) {
    // Grammar: 'letrec' patternBinding (',' patternBinding)* 'in' expr
    // Requires at least one binding; fall back to a safe binding when empty.
    std::string out = "letrec ";
    if (msg.pattern_bindings().empty()) {
        out += "v0 = 0";
    } else {
        out += Join(msg.pattern_bindings(), ", ",
                    [&](const PatternBinding& pb) { return ToString(pb, depth + 1); });
    }
    out += " in " + ToString(msg.body(), depth + 1);
    return out;
}

// ── Abstraction / application ─────────────────────────────────────────────────
std::string ToString(const Abstraction& msg, int depth) {
    std::string out = "fn (";
    out +=
        Join(msg.param_decls(), ", ", [&](const ParamDecl& p) { return ToString(p, depth + 1); });
    out += ") { return " + ToString(msg.return_expr(), depth + 1) + " }";
    return out;
}

std::string ToString(const Application& msg, int depth) {
    std::string out = ToString(msg.fun(), depth + 1) + "(";
    out += Join(msg.args(), ", ", [&](const Expr& e) { return ToString(e, depth + 1); });
    out += ")";
    return out;
}

// ── Generic type abstraction / application ────────────────────────────────────
std::string ToString(const TypeAbstraction& msg, int depth) {
    std::string out = "generic [";
    // Grammar requires at least one generic type parameter.
    if (msg.generics().empty()) {
        out += "T";
    } else {
        out += Join(msg.generics(), ", ", [&](const std::string& g) { return SanitiseIdent(g); });
    }
    out += "] " + ToString(msg.expr(), depth + 1);
    return out;
}

std::string ToString(const TypeApplication& msg, int depth) {
    std::string out = ToString(msg.fun(), depth + 1) + "[";
    // Grammar requires at least one type argument.
    if (msg.types().empty()) {
        out += "Nat";
    } else {
        out += Join(msg.types(), ", ", [&](const StellaType& t) { return ToString(t, depth + 1); });
    }
    out += "]";
    return out;
}

// ── Binary operators ──────────────────────────────────────────────────────────
std::string ToString(const BinaryOp& msg, int depth) {
    std::string op;
    switch (msg.op()) {
    case BinaryOpKind::ADD:
        op = "+";
        break;
    case BinaryOpKind::SUBTRACT:
        op = "-";
        break;
    case BinaryOpKind::MULTIPLY:
        op = "*";
        break;
    case BinaryOpKind::DIVIDE:
        op = "/";
        break;
    case BinaryOpKind::LESS_THAN:
        op = "<";
        break;
    case BinaryOpKind::LESS_THAN_OR_EQUAL:
        op = "<=";
        break;
    case BinaryOpKind::GREATER_THAN:
        op = ">";
        break;
    case BinaryOpKind::GREATER_THAN_OR_EQUAL:
        op = ">=";
        break;
    case BinaryOpKind::EQUAL:
        op = "==";
        break;
    case BinaryOpKind::NOT_EQUAL:
        op = "!=";
        break;
    case BinaryOpKind::LOGIC_AND:
        op = "and";
        break;
    case BinaryOpKind::LOGIC_OR:
        op = "or";
        break;
    default:
        op = "+";
        break;
    }
    return ToString(msg.left(), depth + 1) + " " + op + " " + ToString(msg.right(), depth + 1);
}

// ── Unary operators ───────────────────────────────────────────────────────────
std::string ToString(const UnaryOp& msg, int depth) {
    switch (msg.op()) {
    case UnaryOpKind::LOGIC_NOT:
        return "not(" + ToString(msg.expr(), depth + 1) + ")";
    case UnaryOpKind::DEREF_OP:
        return "*" + ToString(msg.expr(), depth + 1);
    default:
        return ToString(msg.expr(), depth + 1);
    }
}

// ── Type annotations ──────────────────────────────────────────────────────────
std::string ToString(const TypeAsc& msg, int depth) {
    return ToString(msg.expr(), depth + 1) + " as " + ToString(msg.type(), depth + 1);
}

std::string ToString(const TypeCast& msg, int depth) {
    return ToString(msg.expr(), depth + 1) + " cast as " + ToString(msg.type(), depth + 1);
}

// ── Sum types ─────────────────────────────────────────────────────────────────
std::string ToString(const Inl& msg, int depth) {
    return "inl(" + ToString(msg.expr(), depth + 1) + ")";
}

std::string ToString(const Inr& msg, int depth) {
    return "inr(" + ToString(msg.expr(), depth + 1) + ")";
}

// ── Lists ─────────────────────────────────────────────────────────────────────
std::string ToString(const ConsList& msg, int depth) {
    return "cons(" + ToString(msg.head(), depth + 1) + ", " + ToString(msg.tail(), depth + 1) + ")";
}

std::string ToString(const Head& msg, int depth) {
    return "List::head(" + ToString(msg.list(), depth + 1) + ")";
}

std::string ToString(const IsEmpty& msg, int depth) {
    return "List::isempty(" + ToString(msg.list(), depth + 1) + ")";
}

std::string ToString(const Tail& msg, int depth) {
    return "List::tail(" + ToString(msg.list(), depth + 1) + ")";
}

std::string ToString(const ListLiteral& msg, int depth) {
    std::string out = "[";
    out += Join(msg.exprs(), ", ", [&](const Expr& e) { return ToString(e, depth + 1); });
    out += "]";
    return out;
}

// ── Nat builtins ──────────────────────────────────────────────────────────────
std::string ToString(const Succ& msg, int depth) {
    return "succ(" + ToString(msg.n(), depth + 1) + ")";
}

std::string ToString(const Pred& msg, int depth) {
    return "Nat::pred(" + ToString(msg.n(), depth + 1) + ")";
}

std::string ToString(const IsZero& msg, int depth) {
    return "Nat::iszero(" + ToString(msg.n(), depth + 1) + ")";
}

std::string ToString(const NatRec& msg, int depth) {
    return "Nat::rec(" + ToString(msg.n(), depth + 1) + ", " + ToString(msg.initial(), depth + 1) +
           ", " + ToString(msg.step(), depth + 1) + ")";
}

// ── Recursive types ───────────────────────────────────────────────────────────
std::string ToString(const Fold& msg, int depth) {
    return "fold[" + ToString(msg.type(), depth + 1) + "] " + ToString(msg.expr(), depth + 1);
}

std::string ToString(const Unfold& msg, int depth) {
    return "unfold[" + ToString(msg.type(), depth + 1) + "] " + ToString(msg.expr(), depth + 1);
}

// ── References ────────────────────────────────────────────────────────────────
std::string ToString(const Ref& msg, int depth) {
    return "new(" + ToString(msg.expr(), depth + 1) + ")";
}

std::string ToString(const Deref& msg, int depth) { return "*" + ToString(msg.expr(), depth + 1); }

// ── Tuples / records / variants ───────────────────────────────────────────────
std::string ToString(const Tuple& msg, int depth) {
    std::string out = "{";
    out += Join(msg.exprs(), ", ", [&](const Expr& e) { return ToString(e, depth + 1); });
    out += "}";
    return out;
}

std::string ToString(const Record& msg, int depth) {
    // Grammar: '{' binding (',' binding)* '}' — requires at least one binding.
    std::string out = "{";
    if (msg.bindings().empty()) {
        out += "v0 = 0";
    } else {
        out += Join(msg.bindings(), ", ", [&](const Binding& b) { return ToString(b, depth + 1); });
    }
    out += "}";
    return out;
}

std::string ToString(const Variant& msg, int depth) {
    std::string out = "<| " + SanitiseIdent(msg.label());
    if (msg.has_rhs()) {
        out += " = " + ToString(msg.rhs(), depth + 1);
    }
    out += " |>";
    return out;
}

// ── Pattern matching ──────────────────────────────────────────────────────────
std::string ToString(const Match& msg, int depth) {
    std::string out = "match " + ToString(msg.expr(), depth + 1) + " {";
    bool first = true;
    for (const auto& c : msg.cases()) {
        if (!first)
            out += " | ";
        out += " " + ToString(c, depth + 1);
        first = false;
    }
    out += " }";
    return out;
}

// ── Exceptions ────────────────────────────────────────────────────────────────
std::string ToString(const Throw& msg, int depth) {
    return "throw(" + ToString(msg.expr(), depth + 1) + ")";
}

std::string ToString(const TryCatch& msg, int depth) {
    return "try { " + ToString(msg.try_expr(), depth + 1) + " } catch { " +
           ToString(msg.pat(), depth + 1) + " => " + ToString(msg.fallback_expr(), depth + 1) +
           " }";
}

std::string ToString(const TryCastAs& msg, int depth) {
    return "try { " + ToString(msg.try_expr(), depth + 1) + " } cast as " +
           ToString(msg.type(), depth + 1) + " { " + ToString(msg.pattern(), depth + 1) + " => " +
           ToString(msg.expr(), depth + 1) + " } with { " +
           ToString(msg.fallback_expr(), depth + 1) + " }";
}

std::string ToString(const TryWith& msg, int depth) {
    return "try { " + ToString(msg.try_expr(), depth + 1) + " } with { " +
           ToString(msg.fallback_expr(), depth + 1) + " }";
}

// ── Fix ───────────────────────────────────────────────────────────────────────
std::string ToString(const Fix& msg, int depth) {
    return "fix(" + ToString(msg.expr(), depth + 1) + ")";
}

// ── Transparent wrappers ──────────────────────────────────────────────────────
std::string ToString(const ParenthesisedExpr& msg, int depth) {
    return "(" + ToString(msg.expr(), depth + 1) + ")";
}

std::string ToString(const TerminatingSemicolon& msg, int depth) {
    return ToString(msg.expr(), depth + 1) + ";";
}

// ─── Patterns ────────────────────────────────────────────────────────────────
std::string ToString(const Pattern& msg, int depth) {
    if (depth > kMaxDepth)
        return kSafePattern;

    switch (msg.pattern_kind_case()) {
    case Pattern::kVariant:
        return ToString(msg.variant(), depth);
    case Pattern::kInl:
        return ToString(msg.inl(), depth);
    case Pattern::kInr:
        return ToString(msg.inr(), depth);
    case Pattern::kTuple:
        return ToString(msg.tuple(), depth);
    case Pattern::kRecord:
        return ToString(msg.record(), depth);
    case Pattern::kList:
        return ToString(msg.list(), depth);
    case Pattern::kCons:
        return ToString(msg.cons(), depth);
    case Pattern::kConsPair:
        return ToString(msg.cons_pair(), depth);
    case Pattern::kPatFalse:
        return "false";
    case Pattern::kPatTrue:
        return "true";
    case Pattern::kPatUnit:
        return "unit";
    case Pattern::kPatInt:
        return ToString(msg.pat_int(), depth);
    case Pattern::kSucc:
        return ToString(msg.succ(), depth);
    case Pattern::kVar:
        return ToString(msg.var(), depth);
    case Pattern::kAsc:
        return ToString(msg.asc(), depth);
    case Pattern::kCastAs:
        return ToString(msg.cast_as(), depth);
    case Pattern::kParenthesised:
        return ToString(msg.parenthesised(), depth);
    default:
        return kSafePattern;
    }
}

std::string ToString(const PatternVariant& msg, int depth) {
    std::string out = "<| " + SanitiseIdent(msg.label());
    if (msg.has_pattern()) {
        out += " = " + ToString(msg.pattern(), depth + 1);
    }
    out += " |>";
    return out;
}

std::string ToString(const PatternInl& msg, int depth) {
    return "inl(" + ToString(msg.pattern(), depth + 1) + ")";
}

std::string ToString(const PatternInr& msg, int depth) {
    return "inr(" + ToString(msg.pattern(), depth + 1) + ")";
}

std::string ToString(const PatternTuple& msg, int depth) {
    std::string out = "{";
    out += Join(msg.patterns(), ", ", [&](const Pattern& p) { return ToString(p, depth + 1); });
    out += "}";
    return out;
}

std::string ToString(const PatternRecord& msg, int depth) {
    std::string out = "{";
    out += Join(msg.patterns(), ", ",
                [&](const LabelledPattern& lp) { return ToString(lp, depth + 1); });
    out += "}";
    return out;
}

std::string ToString(const PatternList& msg, int depth) {
    std::string out = "[";
    out += Join(msg.patterns(), ", ", [&](const Pattern& p) { return ToString(p, depth + 1); });
    out += "]";
    return out;
}

std::string ToString(const PatternCons& msg, int depth) {
    return "cons(" + ToString(msg.head(), depth + 1) + ", " + ToString(msg.tail(), depth + 1) + ")";
}

std::string ToString(const PatternConsPair& msg, int depth) {
    return "(" + ToString(msg.p1(), depth + 1) + ", " + ToString(msg.p2(), depth + 1) + ")";
}

std::string ToString(const PatternInt& msg, int /*depth*/) {
    // INTEGER token is DIGIT+ — only non-negative values are valid.
    return std::to_string(std::abs(msg.value()));
}

std::string ToString(const PatternSucc& msg, int depth) {
    return "succ(" + ToString(msg.pattern(), depth + 1) + ")";
}

std::string ToString(const PatternVar& msg, int /*depth*/) { return SanitiseIdent(msg.name()); }

std::string ToString(const PatternAsc& msg, int depth) {
    return ToString(msg.pattern(), depth + 1) + " as " + ToString(msg.type(), depth + 1);
}

std::string ToString(const PatternCastAs& msg, int depth) {
    return ToString(msg.pattern(), depth + 1) + " cast as " + ToString(msg.type(), depth + 1);
}

std::string ToString(const ParenthesisedPattern& msg, int depth) {
    return "(" + ToString(msg.pattern(), depth + 1) + ")";
}

// ─── Types ───────────────────────────────────────────────────────────────────
std::string ToString(const StellaType& msg, int depth) {
    if (depth > kMaxDepth)
        return kSafeType;

    switch (msg.type_kind_case()) {
    case StellaType::kBoolType:
        return "Bool";
    case StellaType::kNatType:
        return "Nat";
    case StellaType::kUnitType:
        return "Unit";
    case StellaType::kTopType:
        return "Top";
    case StellaType::kBottomType:
        return "Bot";
    case StellaType::kAutoType:
        return "auto";
    case StellaType::kTypeVar:
        return SanitiseIdent(msg.type_var().name());
    case StellaType::kRefType:
        return ToString(msg.ref_type(), depth);
    case StellaType::kSumType:
        return ToString(msg.sum_type(), depth);
    case StellaType::kFunType:
        return ToString(msg.fun_type(), depth);
    case StellaType::kForallType:
        return ToString(msg.forall_type(), depth);
    case StellaType::kRecType:
        return ToString(msg.rec_type(), depth);
    case StellaType::kTupleType:
        return ToString(msg.tuple_type(), depth);
    case StellaType::kRecordType:
        return ToString(msg.record_type(), depth);
    case StellaType::kVariantType:
        return ToString(msg.variant_type(), depth);
    case StellaType::kListType:
        return ToString(msg.list_type(), depth);
    case StellaType::kParensType:
        return ToString(msg.parens_type(), depth);
    default:
        return kSafeType;
    }
}

std::string ToString(const TypeRef& msg, int depth) {
    return "&" + ToString(msg.type(), depth + 1);
}

std::string ToString(const TypeSum& msg, int depth) {
    return ToString(msg.left(), depth + 1) + " + " + ToString(msg.right(), depth + 1);
}

std::string ToString(const TypeFun& msg, int depth) {
    std::string out = "fn(";
    out +=
        Join(msg.param_types(), ", ", [&](const StellaType& t) { return ToString(t, depth + 1); });
    out += ") -> " + ToString(msg.return_type(), depth + 1);
    return out;
}

std::string ToString(const TypeForAll& msg, int depth) {
    std::string out = "forall";
    for (const auto& tv : msg.types()) {
        out += " " + SanitiseIdent(tv);
    }
    out += ". " + ToString(msg.type(), depth + 1);
    return out;
}

std::string ToString(const TypeRec& msg, int depth) {
    return "µ" + SanitiseIdent(msg.var()) + ". " + ToString(msg.type(), depth + 1);
}

std::string ToString(const TypeTuple& msg, int depth) {
    std::string out = "{";
    out += Join(msg.types(), ", ", [&](const StellaType& t) { return ToString(t, depth + 1); });
    out += "}";
    return out;
}

std::string ToString(const TypeRecord& msg, int depth) {
    std::string out = "{";
    out += Join(msg.field_types(), ", ",
                [&](const RecordFieldType& f) { return ToString(f, depth + 1); });
    out += "}";
    return out;
}

std::string ToString(const TypeVariant& msg, int depth) {
    std::string out = "<|";
    if (!msg.field_types().empty()) {
        out += " ";
        out += Join(msg.field_types(), ", ",
                    [&](const VariantFieldType& f) { return ToString(f, depth + 1); });
        out += " ";
    }
    out += "|>";
    return out;
}

std::string ToString(const TypeList& msg, int depth) {
    return "[" + ToString(msg.type(), depth + 1) + "]";
}

std::string ToString(const TypeParens& msg, int depth) {
    return "(" + ToString(msg.type(), depth + 1) + ")";
}

} // namespace stella