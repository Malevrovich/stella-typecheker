#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

namespace stella {
namespace ast {

class TypeAuto final : public Type {
public:
    TypeAuto(const NodeBase* origin_node = nullptr,
             std::shared_ptr<const SourceInfo> source_info = nullptr);
    ~TypeAuto();

    void OutputTo(std::ostream& out) const override { out << "auto"; }
    void Accept(TypeVisitor& visitor) const override;

    bool Contains(const std::function<bool(const Type&)>& pred) const override {
        return pred(*this);
    }

    class Registry {
    public:
        static Registry& GetInstance();
        std::vector<const TypeAuto*> GetAll() const;
        void Register(const TypeAuto* type);
        void Unregister(const TypeAuto* type);
        void Clear();
        size_t Size() const;

        Registry(const Registry&) = delete;
        Registry& operator=(const Registry&) = delete;
        Registry(Registry&&) = delete;
        Registry& operator=(Registry&&) = delete;

    private:
        Registry() = default;
        ~Registry() = default;

        mutable std::mutex mutex_;
        std::vector<const TypeAuto*> instances_;
    };

protected:
    bool MatchesAny() const override { return true; }

    std::optional<ErrorCode> CheckCompatibleImpl(const Type&,
                                                  const Type::Comparator&) const override {
        return std::nullopt;
    }
};

} // namespace ast
} // namespace stella
