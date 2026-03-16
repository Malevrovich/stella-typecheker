#include <gtest/gtest.h>

#include <stella/ast/attribute_storage.hpp>

namespace stella::ast::test {

// Test attribute types
struct TypeInfo {
    int value = 0;
};

struct DebugInfo {
    std::string name;
};

struct LineInfo {
    int line = 0;
    int column = 0;
};

using TestStorage = AttributeStorage<TypeInfo, DebugInfo, LineInfo>;

class AttributeStorageTest : public ::testing::Test {
protected:
    TestStorage storage;
    void* node1 = reinterpret_cast<void*>(0x1000);
    void* node2 = reinterpret_cast<void*>(0x2000);
};

// Test has()
TEST_F(AttributeStorageTest, HasReturnsFalseForNonexistentAttribute) {
    EXPECT_FALSE(storage.has<TypeInfo>(node1));
}

TEST_F(AttributeStorageTest, HasReturnsTrueAfterSet) {
    storage.set(node1, TypeInfo{42});
    EXPECT_TRUE(storage.has<TypeInfo>(node1));
}

// Test get() with exception
TEST_F(AttributeStorageTest, GetThrowsWhenAttributeNotFound) {
    EXPECT_THROW(storage.get<TypeInfo>(node1), std::runtime_error);
}

TEST_F(AttributeStorageTest, GetReturnsReferenceToStoredAttribute) {
    TypeInfo info{100};
    storage.set(node1, info);
    auto& retrieved = storage.get<TypeInfo>(node1);
    EXPECT_EQ(retrieved.value, 100);
}

TEST_F(AttributeStorageTest, GetMutableReferenceAllowsModification) {
    storage.set(node1, TypeInfo{50});
    auto& ref = storage.get<TypeInfo>(node1);
    ref.value = 75;
    EXPECT_EQ(storage.get<TypeInfo>(node1).value, 75);
}

// Test const get()
TEST_F(AttributeStorageTest, ConstGetThrowsWhenAttributeNotFound) {
    const TestStorage& const_storage = storage;
    EXPECT_THROW(const_storage.get<TypeInfo>(node1), std::runtime_error);
}

TEST_F(AttributeStorageTest, ConstGetReturnsConstReference) {
    storage.set(node1, TypeInfo{200});
    const TestStorage& const_storage = storage;
    const auto& ref = const_storage.get<TypeInfo>(node1);
    EXPECT_EQ(ref.value, 200);
}

// Test tryGet()
TEST_F(AttributeStorageTest, TryGetReturnsNullptrWhenNotFound) {
    EXPECT_EQ(storage.tryGet<TypeInfo>(node1), nullptr);
}

TEST_F(AttributeStorageTest, TryGetReturnsPointerWhenFound) {
    storage.set(node1, TypeInfo{300});
    auto* ptr = storage.tryGet<TypeInfo>(node1);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(ptr->value, 300);
}

// Test const tryGet()
TEST_F(AttributeStorageTest, ConstTryGetReturnsNullptrWhenNotFound) {
    const TestStorage& const_storage = storage;
    EXPECT_EQ(const_storage.tryGet<TypeInfo>(node1), nullptr);
}

TEST_F(AttributeStorageTest, ConstTryGetReturnsConstPointerWhenFound) {
    storage.set(node1, TypeInfo{400});
    const TestStorage& const_storage = storage;
    auto* ptr = const_storage.tryGet<TypeInfo>(node1);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(ptr->value, 400);
}

// Test get() with default value
TEST_F(AttributeStorageTest, GetWithDefaultReturnsReferenceToNewAttribute) {
    auto& ref = storage.get(node1, TypeInfo{500});
    EXPECT_EQ(ref.value, 500);
    EXPECT_TRUE(storage.has<TypeInfo>(node1));
}

TEST_F(AttributeStorageTest, GetWithDefaultReturnsExistingAttribute) {
    storage.set(node1, TypeInfo{600});
    auto& ref = storage.get(node1, TypeInfo{999});
    EXPECT_EQ(ref.value, 600);
}

TEST_F(AttributeStorageTest, GetWithDefaultAllowsModification) {
    auto& ref = storage.get(node1, TypeInfo{700});
    ref.value = 750;
    EXPECT_EQ(storage.get<TypeInfo>(node1).value, 750);
}

// Test set()
TEST_F(AttributeStorageTest, SetStoresAttribute) {
    TypeInfo info{800};
    storage.set(node1, info);
    EXPECT_EQ(storage.get<TypeInfo>(node1).value, 800);
}

TEST_F(AttributeStorageTest, SetReplacesExistingAttribute) {
    storage.set(node1, TypeInfo{100});
    storage.set(node1, TypeInfo{200});
    EXPECT_EQ(storage.get<TypeInfo>(node1).value, 200);
}

TEST_F(AttributeStorageTest, SetWithMoveSemantics) {
    DebugInfo debug{"original"};
    storage.set(node1, std::move(debug));
    EXPECT_EQ(storage.get<DebugInfo>(node1).name, "original");
}

// Test trySet()
TEST_F(AttributeStorageTest, TrySetInsertsNewAttribute) {
    auto [ptr, inserted] = storage.trySet(node1, TypeInfo{900});
    EXPECT_EQ(ptr->value, 900);
    EXPECT_TRUE(inserted);
    EXPECT_TRUE(storage.has<TypeInfo>(node1));
}

TEST_F(AttributeStorageTest, TrySetDoesNotReplaceExistingAttribute) {
    storage.set(node1, TypeInfo{111});
    auto [ptr, inserted] = storage.trySet(node1, TypeInfo{222});
    EXPECT_EQ(ptr->value, 111);
    EXPECT_FALSE(inserted);
}

TEST_F(AttributeStorageTest, TrySetReturnsReferenceToExisting) {
    storage.set(node1, TypeInfo{333});
    auto [ptr, inserted] = storage.trySet(node1, TypeInfo{444});
    EXPECT_EQ(storage.get<TypeInfo>(node1).value, 333);
    EXPECT_FALSE(inserted);
}

// Test remove()
TEST_F(AttributeStorageTest, RemoveDeletesAttribute) {
    storage.set(node1, TypeInfo{555});
    EXPECT_TRUE(storage.has<TypeInfo>(node1));
    storage.remove<TypeInfo>(node1);
    EXPECT_FALSE(storage.has<TypeInfo>(node1));
}

TEST_F(AttributeStorageTest, RemoveNonexistentAttributeDoesNotThrow) {
    EXPECT_NO_THROW(storage.remove<TypeInfo>(node1));
}

// Test removeAll()
TEST_F(AttributeStorageTest, RemoveAllDeletesAllAttributesForNode) {
    storage.set(node1, TypeInfo{1});
    storage.set(node1, DebugInfo{"debug"});
    storage.set(node1, LineInfo{10, 20});

    storage.removeAll(node1);

    EXPECT_FALSE(storage.has<TypeInfo>(node1));
    EXPECT_FALSE(storage.has<DebugInfo>(node1));
    EXPECT_FALSE(storage.has<LineInfo>(node1));
}

TEST_F(AttributeStorageTest, RemoveAllDoesNotAffectOtherNodes) {
    storage.set(node1, TypeInfo{100});
    storage.set(node2, TypeInfo{200});

    storage.removeAll(node1);

    EXPECT_FALSE(storage.has<TypeInfo>(node1));
    EXPECT_TRUE(storage.has<TypeInfo>(node2));
    EXPECT_EQ(storage.get<TypeInfo>(node2).value, 200);
}

// Test clear()
TEST_F(AttributeStorageTest, ClearRemovesAllAttributes) {
    storage.set(node1, TypeInfo{1});
    storage.set(node1, DebugInfo{"debug"});
    storage.set(node2, LineInfo{5, 10});

    storage.clear();

    EXPECT_FALSE(storage.has<TypeInfo>(node1));
    EXPECT_FALSE(storage.has<DebugInfo>(node1));
    EXPECT_FALSE(storage.has<LineInfo>(node2));
}

// Test multiple attributes on same node
TEST_F(AttributeStorageTest, MultipleAttributesOnSameNode) {
    storage.set(node1, TypeInfo{123});
    storage.set(node1, DebugInfo{"test"});
    storage.set(node1, LineInfo{42, 7});

    EXPECT_EQ(storage.get<TypeInfo>(node1).value, 123);
    EXPECT_EQ(storage.get<DebugInfo>(node1).name, "test");
    EXPECT_EQ(storage.get<LineInfo>(node1).line, 42);
    EXPECT_EQ(storage.get<LineInfo>(node1).column, 7);
}

// Test multiple nodes with same attribute type
TEST_F(AttributeStorageTest, MultiplNodesWithSameAttributeType) {
    storage.set(node1, TypeInfo{111});
    storage.set(node2, TypeInfo{222});

    EXPECT_EQ(storage.get<TypeInfo>(node1).value, 111);
    EXPECT_EQ(storage.get<TypeInfo>(node2).value, 222);
}

// Test universal reference forwarding
TEST_F(AttributeStorageTest, UniversalReferencePreservesTemporaries) {
    storage.set(node1, TypeInfo{std::move(TypeInfo{999}).value});
    EXPECT_EQ(storage.get<TypeInfo>(node1).value, 999);
}

// Test default value with move semantics
TEST_F(AttributeStorageTest, GetWithDefaultUsesMoveSemantics) {
    std::string name = "constructed";
    auto& ref = storage.get(node1, DebugInfo{std::move(name)});
    EXPECT_EQ(ref.name, "constructed");
}

// Test storage is shared between const and non-const access
TEST_F(AttributeStorageTest, ConstAccessSeesNonConstSet) {
    storage.set(node1, TypeInfo{555});
    const TestStorage& const_storage = storage;
    EXPECT_EQ(const_storage.get<TypeInfo>(node1).value, 555);
}

TEST_F(AttributeStorageTest, ConstTryGetSeesNonConstSet) {
    storage.set(node1, TypeInfo{666});
    const TestStorage& const_storage = storage;
    auto* ptr = const_storage.tryGet<TypeInfo>(node1);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(ptr->value, 666);
}

TEST_F(AttributeStorageTest, NonConstAccessSeesConstGet) {
    storage.set(node1, DebugInfo{"initial"});
    const TestStorage& const_storage = storage;
    const_storage.get<DebugInfo>(node1);

    // Non-const storage still sees the same value
    EXPECT_EQ(storage.tryGet<DebugInfo>(node1)->name, "initial");
}

TEST_F(AttributeStorageTest, SharedStorageMultipleAttributes) {
    // Add through non-const
    storage.set(node1, TypeInfo{100});
    storage.set(node1, DebugInfo{"test"});

    // Read through const
    const TestStorage& const_storage = storage;
    EXPECT_EQ(const_storage.get<TypeInfo>(node1).value, 100);
    EXPECT_EQ(const_storage.get<DebugInfo>(node1).name, "test");
}

TEST_F(AttributeStorageTest, NonConstGetDefaultAndConstAccess) {
    auto& ref = storage.get(node1, TypeInfo{777});
    EXPECT_EQ(ref.value, 777);

    // Access through const storage
    const TestStorage& const_storage = storage;
    EXPECT_EQ(const_storage.get<TypeInfo>(node1).value, 777);
}

TEST_F(AttributeStorageTest, TrySetThenConstAccess) {
    auto [ptr, inserted] = storage.trySet(node1, TypeInfo{888});
    EXPECT_EQ(ptr->value, 888);
    EXPECT_TRUE(inserted);

    const TestStorage& const_storage = storage;
    EXPECT_TRUE(const_storage.has<TypeInfo>(node1));
    EXPECT_EQ(const_storage.tryGet<TypeInfo>(node1)->value, 888);
}

// Test storing const qualified types uses same storage as non-const
TEST_F(AttributeStorageTest, ConstQualifiedTypeUsesSharedStorage) {
    TypeInfo info{200};
    const TypeInfo const_info = info;

    // Set with const value
    storage.set(node1, const_info);

    // Retrieve with non-const type
    EXPECT_TRUE(storage.has<TypeInfo>(node1));
    EXPECT_EQ(storage.get<TypeInfo>(node1).value, 200);
}

TEST_F(AttributeStorageTest, TrySetWithConstValue) {
    const TypeInfo const_val{400};
    auto [ptr, inserted] = storage.trySet(node1, const_val);
    EXPECT_EQ(ptr->value, 400);
    EXPECT_TRUE(inserted);
    EXPECT_EQ(storage.get<TypeInfo>(node1).value, 400);
}

TEST_F(AttributeStorageTest, GetWithConstDefaultValue) {
    const TypeInfo const_default{500};
    auto& ref = storage.get(node1, const_default);
    EXPECT_EQ(ref.value, 500);

    // Verify it's stored under non-const TypeInfo
    EXPECT_TRUE(storage.has<TypeInfo>(node1));
}

TEST_F(AttributeStorageTest, MultipleConstAssignmentsShareStorage) {
    const TypeInfo c1{111};
    const TypeInfo c2{222};

    storage.set(node1, c1);
    EXPECT_EQ(storage.get<TypeInfo>(node1).value, 111);

    storage.set(node1, c2);
    EXPECT_EQ(storage.get<TypeInfo>(node1).value, 222);
}

} // namespace stella::ast::test
