#pragma once

#include <memory>
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
    void OutputTo(std::ostream& out) const override { out << "auto"; }
    void Accept(TypeVisitor& visitor) const override;

    bool Contains(const std::function<bool(const Type&)>& pred) const override {
        return pred(*this);
    }

protected:
    bool MatchesAny() const override { return true; }

    // Unreachable: MatchesAny() short-circuits CheckCompatible before reaching here.
    std::optional<ErrorCode> CheckCompatibleImpl(const Type&,
                                                 const Type::Comparator&) const override {
        return std::nullopt;
    }
};

} // namespace ast
} // namespace stella
