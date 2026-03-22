#include <gtest/gtest.h>

#include <memory>
#include <typeinfo>

#include "stella/ast/logic.hpp"
#include "stella/ast/nat.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"

namespace stella::typecheck::test {

class ExpectedTypeTest : public ::testing::Test {
protected:
    // Create a minimal SourceInfo for testing
    class MockSourceInfo : public ast::SourceInfo {
    public:
        std::string ToString() const override { return "MockSourceInfo"; }
        std::string GetLocation() const override { return "test:1:1"; }
    };

    std::shared_ptr<ast::SourceInfo> source_info_ = std::make_shared<MockSourceInfo>();
};

// Test EqualsTo factory method
TEST_F(ExpectedTypeTest, EqualsToCreatesExpectedTypeWithTypeName) {
    auto nat_type = std::make_shared<ast::TypeNat>();
    auto expected = ExpectedType::EqualsTo(nat_type);
    EXPECT_EQ(expected.ToString(), nat_type->ToString());
}

TEST_F(ExpectedTypeTest, EqualsToCheckAcceptsEqualType) {
    auto nat_type = std::make_shared<ast::TypeNat>();
    auto expected = ExpectedType::EqualsTo(nat_type);

    ast::TypeNat actual_type;
    auto result = expected.Check(actual_type);
    EXPECT_FALSE(result.has_value());
}

TEST_F(ExpectedTypeTest, EqualsToCheckRejectsIncompatibleType) {
    auto nat_type = std::make_shared<ast::TypeNat>();
    auto expected = ExpectedType::EqualsTo(nat_type);

    ast::TypeBool actual_type;
    auto result = expected.Check(actual_type);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION);
}

TEST_F(ExpectedTypeTest, EqualsToWithCustomErrorCode) {
    auto nat_type = std::make_shared<ast::TypeNat>();
    auto expected =
        ExpectedType::EqualsTo(nat_type, ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_PARAMETER);

    ast::TypeBool actual_type;
    auto result = expected.Check(actual_type);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_PARAMETER);
}

TEST_F(ExpectedTypeTest, EqualsToCheckParameterErrorCodeTakesPrecedence) {
    auto nat_type = std::make_shared<ast::TypeNat>();
    auto expected =
        ExpectedType::EqualsTo(nat_type, ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_PARAMETER);

    ast::TypeBool actual_type;
    auto result = expected.Check(actual_type, ErrorCode::ERROR_UNEXPECTED_LAMBDA);
    EXPECT_TRUE(result.has_value());
    // Check parameter error code takes precedence over constructor error code
    EXPECT_EQ(result.value(), ErrorCode::ERROR_UNEXPECTED_LAMBDA);
}

TEST_F(ExpectedTypeTest, CheckWithLocalErrorCodeOverridesDefault) {
    auto nat_type = std::make_shared<ast::TypeNat>();
    auto expected = ExpectedType::EqualsTo(nat_type);

    ast::TypeBool actual_type;
    auto result = expected.Check(actual_type, ErrorCode::ERROR_NOT_A_FUNCTION);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), ErrorCode::ERROR_NOT_A_FUNCTION);
}

// Test CompatibleWith factory method
TEST_F(ExpectedTypeTest, CompatibleWithCreatesExpectedType) {
    auto expected = ExpectedType::CompatibleWith<ast::TypeNat>();
    EXPECT_TRUE(!expected.ToString().empty());
}

TEST_F(ExpectedTypeTest, CompatibleWithCheckAcceptsCompatibleType) {
    auto expected = ExpectedType::CompatibleWith<ast::TypeNat>();
    ast::TypeNat actual_type;
    auto result = expected.Check(actual_type);
    EXPECT_FALSE(result.has_value());
}

TEST_F(ExpectedTypeTest, CompatibleWithCheckRejectsIncompatibleType) {
    auto expected = ExpectedType::CompatibleWith<ast::TypeNat>();
    ast::TypeBool actual_type;
    auto result = expected.Check(actual_type);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION);
}

TEST_F(ExpectedTypeTest, CompatibleWithCustomErrorCode) {
    auto expected =
        ExpectedType::CompatibleWith<ast::TypeNat>(ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_PARAMETER);

    ast::TypeBool actual_type;
    auto result = expected.Check(actual_type);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_PARAMETER);
}

// Test CheckCompatibleWith template method
TEST_F(ExpectedTypeTest, CheckCompatibleWithAcceptsMatchingType) {
    auto expected = ExpectedType::CompatibleWith<ast::TypeNat>();
    auto result = expected.CheckCompatibleWith<ast::TypeNat>();
    EXPECT_FALSE(result.has_value());
}

TEST_F(ExpectedTypeTest, CheckCompatibleWithRejectsIncompatibleType) {
    auto expected = ExpectedType::CompatibleWith<ast::TypeNat>();
    auto result = expected.CheckCompatibleWith<ast::TypeBool>();
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION);
}

TEST_F(ExpectedTypeTest, CheckCompatibleWithCustomErrorCode) {
    auto expected = ExpectedType::CompatibleWith<ast::TypeNat>();
    auto result = expected.CheckCompatibleWith<ast::TypeBool>(ErrorCode::ERROR_NOT_A_TUPLE);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), ErrorCode::ERROR_NOT_A_TUPLE);
}

TEST_F(ExpectedTypeTest, CheckCompatibleWithCustomErrorCodeFromConstructor) {
    auto expected =
        ExpectedType::CompatibleWith<ast::TypeNat>(ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_PARAMETER);

    auto result = expected.CheckCompatibleWith<ast::TypeBool>();
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_PARAMETER);
}

// Test ToString method
TEST_F(ExpectedTypeTest, ToStringReturnsTypeName) {
    auto nat_type = std::make_shared<ast::TypeNat>();
    auto expected = ExpectedType::EqualsTo(nat_type);
    EXPECT_EQ(expected.ToString(), "Nat");
}

TEST_F(ExpectedTypeTest, ToStringForDifferentType) {
    auto bool_type = std::make_shared<ast::TypeBool>();
    auto expected = ExpectedType::EqualsTo(bool_type);
    EXPECT_EQ(expected.ToString(), "Bool");
}

TEST_F(ExpectedTypeTest, EqualsToMultipleTypes) {
    auto nat_type1 = std::make_shared<ast::TypeNat>();
    auto nat_type2 = std::make_shared<ast::TypeNat>();
    auto expected1 = ExpectedType::EqualsTo(nat_type1);
    auto expected2 = ExpectedType::EqualsTo(nat_type2);

    EXPECT_EQ(expected1.ToString(), expected2.ToString());
}

// Test DetermineErrorCode priority
TEST_F(ExpectedTypeTest, ErrorCodePriorityCheckParameterFirst) {
    auto nat_type = std::make_shared<ast::TypeNat>();
    auto expected =
        ExpectedType::EqualsTo(nat_type, ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_PARAMETER);

    ast::TypeBool actual_type;
    // When both constructor and Check have error codes, Check parameter wins
    auto result = expected.Check(actual_type, ErrorCode::ERROR_NOT_A_FUNCTION);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), ErrorCode::ERROR_NOT_A_FUNCTION);
}

TEST_F(ExpectedTypeTest, ErrorCodeFallbackToDefault) {
    auto nat_type = std::make_shared<ast::TypeNat>();
    auto expected = ExpectedType::EqualsTo(nat_type);

    ast::TypeBool actual_type;
    auto result = expected.Check(actual_type);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION);
}

// Test multiple type checks in sequence
TEST_F(ExpectedTypeTest, MultipleCheckSequence) {
    auto nat_type = std::make_shared<ast::TypeNat>();
    auto expected = ExpectedType::EqualsTo(nat_type);

    ast::TypeNat nat_actual;
    ast::TypeBool bool_actual;

    auto result1 = expected.Check(nat_actual);
    auto result2 = expected.Check(bool_actual);

    EXPECT_FALSE(result1.has_value());
    EXPECT_TRUE(result2.has_value());
}

// Test with different error code each time
TEST_F(ExpectedTypeTest, DifferentErrorCodesInMultipleChecks) {
    auto nat_type = std::make_shared<ast::TypeNat>();
    auto expected = ExpectedType::EqualsTo(nat_type);

    ast::TypeBool actual_type;

    auto result1 = expected.Check(actual_type, ErrorCode::ERROR_NOT_A_LIST);
    auto result2 = expected.Check(actual_type, ErrorCode::ERROR_NOT_A_RECORD);

    EXPECT_EQ(result1.value(), ErrorCode::ERROR_NOT_A_LIST);
    EXPECT_EQ(result2.value(), ErrorCode::ERROR_NOT_A_RECORD);
}

// Test EqualsTo with type-erased pointer (std::shared_ptr<ast::Type>)
TEST_F(ExpectedTypeTest, EqualsToWithTypeErasedPointer) {
    // Create a TypeNat but store it as ast::Type
    std::shared_ptr<ast::Type> type_erased = std::make_shared<ast::TypeNat>();
    auto expected = ExpectedType::EqualsTo(type_erased);

    // Check that it correctly identifies the same type
    ast::TypeNat nat_actual;
    auto result_match = expected.Check(nat_actual);
    EXPECT_FALSE(result_match.has_value());

    // Check that it rejects incompatible type
    ast::TypeBool bool_actual;
    auto result_mismatch = expected.Check(bool_actual);
    EXPECT_TRUE(result_mismatch.has_value());
    EXPECT_EQ(result_mismatch.value(), ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION);
}

TEST_F(ExpectedTypeTest, EqualsToWithTypeErasedPointerDifferentType) {
    // Create a TypeBool but store it as ast::Type
    std::shared_ptr<ast::Type> type_erased = std::make_shared<ast::TypeBool>();
    auto expected = ExpectedType::EqualsTo(type_erased);

    // Check with matching type
    ast::TypeBool bool_actual;
    auto result_match = expected.Check(bool_actual);
    EXPECT_FALSE(result_match.has_value());

    auto result_compatible_match = expected.CheckCompatibleWith<ast::TypeBool>();
    EXPECT_FALSE(result_compatible_match.has_value());

    // Check with different type
    ast::TypeNat nat_actual;
    auto result_mismatch = expected.Check(nat_actual);
    EXPECT_TRUE(result_mismatch.has_value());

    auto result_compatible_mismatch = expected.CheckCompatibleWith<ast::TypeNat>();
    EXPECT_TRUE(result_compatible_mismatch.has_value());
}

TEST_F(ExpectedTypeTest, EqualsToTypeErasedWithCustomErrorCode) {
    std::shared_ptr<ast::Type> type_erased = std::make_shared<ast::TypeNat>();
    auto expected = ExpectedType::EqualsTo(type_erased, ErrorCode::ERROR_NOT_A_RECORD);

    ast::TypeBool actual_type;
    auto result = expected.Check(actual_type);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), ErrorCode::ERROR_NOT_A_RECORD);
}

} // namespace stella::typecheck::test
