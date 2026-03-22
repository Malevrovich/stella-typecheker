#pragma once

#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <unordered_map>

namespace stella {
namespace ast {

namespace detail {

template <typename...>
struct are_unique : std::true_type {};

template <typename T, typename... Rest>
struct are_unique<T, Rest...>
    : std::bool_constant<(!std::is_same_v<T, Rest> && ...) && are_unique<Rest...>::value> {};

} // namespace detail

/**
 * @class AttributeStorage
 * @brief A type-safe, heterogeneous attribute storage container.
 *
 * This class provides a generic container for storing attributes of different types associated with
 * nodes (identified by void* pointers). It ensures all attribute types are unique at compile-time
 * and provides type-safe access through template methods.
 *
 * @tparam Attrs Template parameter pack of attribute types to store.
 *
 * @note All attribute types must be unique. A compile-time assertion ensures this invariant.
 */
template <typename... Attrs>
class AttributeStorage {
    static_assert(detail::are_unique<Attrs...>::value, "All attribute types must be unique");

public:
    /**
     * @brief Check if an attribute exists for the given node.
     * @tparam Attr The attribute type to check for.
     * @param node Pointer to the node to query.
     * @return true if the attribute exists for the node, false otherwise.
     */
    template <typename Attr>
    bool has(const void* node) const;

    /**
     * @brief Get a reference to an attribute for the given node.
     * @tparam Attr The attribute type to retrieve.
     * @param node Pointer to the node.
     * @return Reference to the attribute.
     * @throw std::runtime_error if the attribute does not exist for the node.
     */
    template <typename Attr>
    Attr& get(const void* node);

    /**
     * @brief Get a const reference to an attribute for the given node.
     * @tparam Attr The attribute type to retrieve.
     * @param node Pointer to the node.
     * @return Const reference to the attribute.
     * @throw std::runtime_error if the attribute does not exist for the node.
     */
    template <typename Attr>
    const Attr& get(const void* node) const;

    /**
     * @brief Attempt to get a pointer to an attribute for the given node.
     * @tparam Attr The attribute type to retrieve.
     * @param node Pointer to the node.
     * @return Pointer to the attribute if it exists, nullptr otherwise.
     */
    template <typename Attr>
    Attr* tryGet(const void* node);

    /**
     * @brief Attempt to get a const pointer to an attribute for the given node.
     * @tparam Attr The attribute type to retrieve.
     * @param node Pointer to the node.
     * @return Const pointer to the attribute if it exists, nullptr otherwise.
     */
    template <typename Attr>
    const Attr* tryGet(const void* node) const;

    /**
     * @brief Get or insert an attribute with a default value.
     * @tparam Attr The attribute type.
     * @param node Pointer to the node.
     * @param default_value The value to insert if the attribute does not exist. Passed by universal
     * reference.
     * @return Reference to the existing or newly inserted attribute.
     */
    template <typename Attr>
    Attr& get(const void* node, Attr&& default_value);

    /**
     * @brief Set or replace an attribute for the given node.
     * @tparam Attr The attribute type.
     * @param node Pointer to the node.
     * @param value The value to set. Passed by universal reference.
     */
    template <typename Attr>
    void set(const void* node, Attr&& value);

    /**
     * @brief Attempt to insert an attribute for the given node if it does not already exist.
     * @tparam Attr The attribute type.
     * @param node Pointer to the node.
     * @param value The value to insert if the attribute does not exist. Passed by universal
     * reference.
     * @return Pair containing a pointer to the attribute and a boolean. The boolean is true if the
     * attribute was inserted, false if it already existed.
     */
    template <typename Attr>
    std::pair<std::remove_cvref_t<Attr>*, bool> trySet(const void* node, Attr&& value);

    /**
     * @brief Remove an attribute from the given node.
     * @tparam Attr The attribute type to remove.
     * @param node Pointer to the node.
     */
    template <typename Attr>
    void remove(const void* node);

    /**
     * @brief Remove all attributes for the given node.
     * @param node Pointer to the node.
     */
    void removeAll(const void* node);

    /**
     * @brief Clear all stored attributes.
     */
    void clear();

private:
    std::tuple<std::unordered_map<const void*, Attrs>...> maps_;

    template <typename Attr>
    auto& getMap();

    template <typename Attr>
    const auto& getMap() const;
};

namespace detail {

template <typename T, typename... Types>
struct Index;

template <typename T, typename... Rest>
struct Index<T, T, Rest...> : std::integral_constant<std::size_t, 0> {};

template <typename T, typename U, typename... Rest>
struct Index<T, U, Rest...> : std::integral_constant<std::size_t, 1 + Index<T, Rest...>::value> {};

} // namespace detail

template <typename... Attrs>
template <typename Attr>
auto& AttributeStorage<Attrs...>::getMap() {
    constexpr std::size_t idx = detail::Index<Attr, Attrs...>::value;
    return std::get<idx>(maps_);
}

template <typename... Attrs>
template <typename Attr>
const auto& AttributeStorage<Attrs...>::getMap() const {
    constexpr std::size_t idx = detail::Index<Attr, Attrs...>::value;
    return std::get<idx>(maps_);
}

template <typename... Attrs>
template <typename Attr>
bool AttributeStorage<Attrs...>::has(const void* node) const {
    const auto& map = getMap<Attr>();
    return map.find(node) != map.end();
}

template <typename... Attrs>
template <typename Attr>
Attr& AttributeStorage<Attrs...>::get(const void* node) {
    auto& map = getMap<Attr>();
    auto it = map.find(node);
    if (it == map.end()) {
        throw std::runtime_error("Attribute not found for node");
    }
    return it->second;
}

template <typename... Attrs>
template <typename Attr>
const Attr& AttributeStorage<Attrs...>::get(const void* node) const {
    const auto& map = getMap<Attr>();
    auto it = map.find(node);
    if (it == map.end()) {
        throw std::runtime_error("Attribute not found for node");
    }
    return it->second;
}

template <typename... Attrs>
template <typename Attr>
Attr* AttributeStorage<Attrs...>::tryGet(const void* node) {
    auto& map = getMap<Attr>();
    auto it = map.find(node);
    return (it != map.end()) ? &it->second : nullptr;
}

template <typename... Attrs>
template <typename Attr>
const Attr* AttributeStorage<Attrs...>::tryGet(const void* node) const {
    const auto& map = getMap<Attr>();
    auto it = map.find(node);
    return (it != map.end()) ? &it->second : nullptr;
}

template <typename... Attrs>
template <typename Attr>
Attr& AttributeStorage<Attrs...>::get(const void* node, Attr&& default_value) {
    auto& map = getMap<std::remove_cvref_t<Attr>>();
    auto result = map.emplace(node, std::forward<Attr>(default_value));
    return result.first->second;
}

template <typename... Attrs>
template <typename Attr>
void AttributeStorage<Attrs...>::set(const void* node, Attr&& value) {
    auto& map = getMap<std::remove_cvref_t<Attr>>();
    map[node] = std::forward<Attr>(value);
}

template <typename... Attrs>
template <typename Attr>
std::pair<std::remove_cvref_t<Attr>*, bool> AttributeStorage<Attrs...>::trySet(const void* node,
                                                                               Attr&& value) {
    auto& map = getMap<std::remove_cvref_t<Attr>>();
    auto result = map.try_emplace(node, std::forward<Attr>(value));
    return {&result.first->second, result.second};
}

template <typename... Attrs>
template <typename Attr>
void AttributeStorage<Attrs...>::remove(const void* node) {
    auto& map = getMap<Attr>();
    map.erase(node);
}

template <typename... Attrs>
void AttributeStorage<Attrs...>::removeAll(const void* node) {
    (remove<Attrs>(node), ...);
}

template <typename... Attrs>
void AttributeStorage<Attrs...>::clear() {
    (getMap<Attrs>().clear(), ...);
}

} // namespace ast
} // namespace stella