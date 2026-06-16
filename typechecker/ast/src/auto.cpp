#include "stella/ast/auto.hpp"

#include "stella/ast/visitor.hpp"

namespace stella {
namespace ast {

TypeAuto::TypeAuto(const NodeBase* origin_node,
                   std::shared_ptr<const SourceInfo> source_info)
    : Type(origin_node, std::move(source_info)) {
    // Register this instance in the singleton registry
    Registry::GetInstance().Register(this);
}

TypeAuto::~TypeAuto() {
    // Unregister this instance from the singleton registry
    Registry::GetInstance().Unregister(this);
}

// Registry implementation
TypeAuto::Registry& TypeAuto::Registry::GetInstance() {
    static Registry instance;
    return instance;
}

std::vector<const TypeAuto*> TypeAuto::Registry::GetAll() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return instances_;
}

void TypeAuto::Registry::Register(const TypeAuto* type) {
    std::lock_guard<std::mutex> lock(mutex_);
    instances_.push_back(type);
}

void TypeAuto::Registry::Unregister(const TypeAuto* type) {
    std::lock_guard<std::mutex> lock(mutex_);
    instances_.erase(
        std::remove(instances_.begin(), instances_.end(), type),
        instances_.end()
    );
}

void TypeAuto::Registry::Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    instances_.clear();
}

size_t TypeAuto::Registry::Size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return instances_.size();
}

} // namespace ast
} // namespace stella
