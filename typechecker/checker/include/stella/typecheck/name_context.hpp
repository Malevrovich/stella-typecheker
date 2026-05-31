#pragma once

#include <format>
#include <functional>
#include <stack>
#include <string>
#include <unordered_map>

#include <loguru.hpp>

#include "stella/ast/ast_fwd.hpp"
#include "stella/typecheck/error.hpp"

namespace stella {
namespace typecheck {

template <typename T>
class NameContext {
public:
    class NameContextGuard {
    public:
        NameContextGuard(NameContext& ctx_, std::string_view name)
            : ctx_(&ctx_),
              name_(name) {}

        NameContextGuard(const NameContextGuard&) = delete;
        NameContextGuard& operator=(const NameContextGuard&) = delete;

        NameContextGuard(NameContextGuard&& other) noexcept
            : ctx_(std::exchange(other.ctx_, nullptr)),
              name_(other.name_) {}

        NameContextGuard& operator=(NameContextGuard&& other) noexcept {
            if (this != &other) {
                release();
                ctx_ = std::exchange(other.ctx_, nullptr);
                name_ = other.name_;
            }
            return *this;
        }

        ~NameContextGuard() { release(); }

        void release() {
            if (ctx_) {
                ctx_->Pop(name_);
                ctx_ = nullptr;
            }
        }

        void detach() { ctx_ = nullptr; }

    private:
        NameContext* ctx_;
        std::string_view name_;
    };

    [[nodiscard]] NameContextGuard PushUnique(std::string name, T& value) {
        auto [it, emplaced] = context_.emplace(std::move(name), std::stack<T*>{{&value}});
        if (!emplaced) {
            throw TypeCheckNodeError(
                ErrorCode::ERROR_DUPLICATE_FUNCTION_DECLARATION, value,
                std::format("Previous declaration at {}", it->second.top()->ToString()));
        }
        return NameContextGuard{*this, it->first};
    }

    NameContextGuard Push(std::string name, T& value) {
        DLOG_S(INFO) << "Pushing name to context '" << name << "'";
        auto [it, _] = context_.emplace(std::move(name), std::stack<T*>{});
        it->second.push(&value);
        return NameContextGuard{*this, it->first};
    }

    void Pop(std::string_view name) {
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

    T& Get(std::string_view name, const ast::NodeBase& error_context) {
        auto it = context_.find(name);
        if (it == context_.end()) {
            throw TypeCheckNodeError(ErrorCode::ERROR_UNDEFINED_VARIABLE, error_context);
        }
        return *it->second.top();
    }

    T* TryGet(std::string_view name) {
        auto it = context_.find(name);
        if (it == context_.end() || it->second.empty()) {
            return nullptr;
        }
        return it->second.top();
    }

private:
    struct string_hash {
        using is_transparent = void;
        std::size_t operator()(std::string_view sv) const {
            return std::hash<std::string_view>{}(sv);
        }
        std::size_t operator()(const std::string& s) const { return std::hash<std::string>{}(s); }
    };

    std::unordered_map<std::string, std::stack<T*>, string_hash, std::equal_to<>> context_{};
};

} // namespace typecheck
} // namespace stella
