#include "stella/ast/generic.hpp"
#include "stella/ast/visitor.hpp"

#include <sstream>

namespace stella {
namespace ast {

// TypeVar implementation

TypeVar::TypeVar(std::string name,
                 const NodeBase* origin_node,
                 std::shared_ptr<const SourceInfo> source_info)
    : Type(origin_node, std::move(source_info)),
      name_(std::move(name)) {}

void TypeVar::OutputTo(std::ostream& out) const { out << name_; }

bool TypeVar::Contains(const std::function<bool(const Type&)>& pred) const { return pred(*this); }

std::optional<ErrorCode> TypeVar::CheckCompatibleImpl(const Type& other,
                                                      const Comparator& cmp) const {
    const auto* o = dynamic_cast<const TypeVar*>(&other);
    if (!o) {
        return FamilyMismatchError(other);
    }
    if (name_ != o->name_) {
        return ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION;
    }
    return std::nullopt;
}

// TypeForAll implementation

TypeForAll::TypeForAll(std::vector<std::string> type_params,
                      std::shared_ptr<const Type> body,
                      const NodeBase* origin_node,
                      std::shared_ptr<const SourceInfo> source_info)
    : Type(origin_node, std::move(source_info)),
      type_params_(std::move(type_params)),
      body_(std::move(body)) {}

std::shared_ptr<TypeForAll> TypeForAll::MakeSentinel() {
    return std::make_shared<TypeForAll>(SentinelTag{});
}

void TypeForAll::OutputTo(std::ostream& out) const {
    if (IsSentinel()) {
        out << "forall ...";
        return;
    }

    out << "forall ";
    for (size_t i = 0; i < type_params_.size(); ++i) {
        if (i > 0) {
            out << ", ";
        }
        out << type_params_[i];
    }
    out << ". ";
    body_->OutputTo(out);
}

bool TypeForAll::Contains(const std::function<bool(const Type&)>& pred) const {
    if (pred(*this)) {
        return true;
    }
    if (IsSentinel()) {
        return false;
    }
    return body_->Contains(pred);
}

std::optional<ErrorCode> TypeForAll::CheckCompatibleImpl(const Type& other,
                                                         const Comparator& cmp) const {
    const auto* o = dynamic_cast<const TypeForAll*>(&other);
    if (!o) {
        return FamilyMismatchError(other);
    }
    if (!body_ || !o->body_) {
        return std::nullopt; // sentinel matches any
    }
    // For now, we just check that the bodies are compatible
    // A full implementation would need to handle type parameter substitution
    return cmp(*body_, *o->body_);
}

// NodeDeclFunGeneric implementation

NodeDeclFunGeneric::NodeDeclFunGeneric(std::shared_ptr<SourceInfo> source_info, std::string name,
                                       std::vector<std::string> type_params,
                                       std::shared_ptr<const Type> return_type,
                                       std::shared_ptr<const NodeExprAbstraction> abstraction,
                                       std::vector<std::shared_ptr<const NodeDecl>> local_decls)
    : NodeDecl(std::move(source_info)),
      name_(std::move(name)),
      type_params_(std::move(type_params)),
      return_type_(std::move(return_type)),
      abstraction_(std::move(abstraction)),
      local_decls_(std::move(local_decls)) {}

// NodeExprTypeAbstraction implementation

NodeExprTypeAbstraction::NodeExprTypeAbstraction(std::shared_ptr<SourceInfo> source_info,
                                                 std::vector<std::string> type_params,
                                                 std::shared_ptr<const NodeExpr> body)
    : NodeExpr(std::move(source_info)),
      type_params_(std::move(type_params)),
      body_(std::move(body)) {}

// NodeExprTypeApplication implementation

NodeExprTypeApplication::NodeExprTypeApplication(std::shared_ptr<SourceInfo> source_info,
                                                 std::shared_ptr<const NodeExpr> function,
                                                 std::vector<std::shared_ptr<const Type>> type_args)
    : NodeExpr(std::move(source_info)),
      function_(std::move(function)),
      type_args_(std::move(type_args)) {}

} // namespace ast
} // namespace stella
