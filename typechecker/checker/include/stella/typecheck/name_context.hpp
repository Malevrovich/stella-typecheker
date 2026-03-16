#pragma once

#include <functional>
#include <stack>
#include <string>
#include <unordered_map>

#include "stella/ast/ast_fwd.hpp"

namespace stella {
namespace typecheck {

class NameContext {
public:
    class NameContextGuard {
    public:
        NameContextGuard(NameContext& ctx_, std::string_view name);

        NameContextGuard(const NameContextGuard&) = delete;
        NameContextGuard& operator=(const NameContextGuard&) = delete;

        NameContextGuard(NameContextGuard&& other) noexcept;
        NameContextGuard& operator=(NameContextGuard&& other) noexcept;

        ~NameContextGuard();

        void release();

        void detach();

    private:
        NameContext* ctx_;
        std::string_view name_;
    };

    [[nodiscard]] NameContextGuard PushUnique(std::string name, const ast::NodeBase& node);

    NameContextGuard Push(std::string name, const ast::NodeBase& node);
    void Pop(std::string_view name);

    const ast::NodeBase& Get(std::string_view name, const ast::NodeBase& error_context);

private:
    struct string_hash {
        using is_transparent = void;
        std::size_t operator()(std::string_view sv) const {
            return std::hash<std::string_view>{}(sv);
        }
        std::size_t operator()(const std::string& s) const { return std::hash<std::string>{}(s); }
    };

    std::unordered_map<std::string, std::stack<const ast::NodeBase*>, string_hash, std::equal_to<>>
        context_{};
};

} // namespace typecheck
} // namespace stella