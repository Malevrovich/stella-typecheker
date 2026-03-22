#include "stella/typecheck/name_context.hpp"

#include <format>
#include <utility>

#include <loguru.hpp>

#include "stella/ast/ast_fwd.hpp"
#include "stella/typecheck/error.hpp"

namespace stella {
namespace typecheck {

NameContext::NameContextGuard::NameContextGuard(NameContext& ctx_, std::string_view name)
    : ctx_(&ctx_),
      name_(name) {}

NameContext::NameContextGuard::NameContextGuard(NameContextGuard&& other) noexcept
    : ctx_(std::exchange(other.ctx_, nullptr)),
      name_(other.name_) {}

NameContext::NameContextGuard&
NameContext::NameContextGuard::operator=(NameContextGuard&& other) noexcept {
    if (this != &other) {
        release();
        ctx_ = std::exchange(other.ctx_, nullptr);
        name_ = other.name_;
    }
    return *this;
}

NameContext::NameContextGuard::~NameContextGuard() { release(); }

void NameContext::NameContextGuard::release() {
    if (ctx_) {
        ctx_->Pop(name_);
    }
}

void NameContext::NameContextGuard::detach() { ctx_ = nullptr; }

NameContext::NameContextGuard NameContext::PushUnique(std::string name, const ast::NodeBase& node) {
    auto [it, emplaced] =
        context_.emplace(std::move(name), std::stack<const ast::NodeBase*>{{&node}});
    if (!emplaced) {
        throw TypeCheckNodeError(
            ErrorCode::ERROR_DUPLICATE_FUNCTION_DECLARATION, node,
            std::format("Previous declaration at {}", it->second.top()->ToString()));
    }
    return NameContextGuard{*this, it->first};
}

NameContext::NameContextGuard NameContext::Push(std::string name, const ast::NodeBase& node) {
    DLOG_S(INFO) << "Pushing name to context '" << name << "'";
    auto [it, _] = context_.emplace(std::move(name), std::stack<const ast::NodeBase*>{});
    it->second.push(&node);
    return NameContextGuard{*this, it->first};
}

void NameContext::Pop(std::string_view name) {
    DLOG_S(INFO) << "Poping name from context '" << name << "'";

    auto it = context_.find(name);
    if (it == context_.end()) {
        throw InternalTypeCheckError(std::format("Name not found for pop: {}", name));
    }
    if (it->second.size() == 0) {
        throw InternalTypeCheckError(std::format("Unexpected end of stack for name: {}", name));
    }
    it->second.pop();
    if (it->second.size() == 0) {
        context_.erase(it);
    }
}

const ast::NodeBase& NameContext::Get(std::string_view name, const ast::NodeBase& error_context) {
    auto it = context_.find(name);
    if (it == context_.end()) {
        throw TypeCheckNodeError(ErrorCode::ERROR_UNDEFINED_VARIABLE, error_context);
    }
    return *it->second.top();
}

} // namespace typecheck
} // namespace stella
