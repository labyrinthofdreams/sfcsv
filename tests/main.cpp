#include <iostream>
#include <iterator>
#include <vector>
#include <string>
#include "sfcsv.h"
#include "gtest/gtest.h"

class ParserTest : public ::testing::Test {
protected:
    std::vector<std::string> all;
    std::vector<std::string> result;

    template <class ...Args>
    bool vec_eq(Args ...args) {
        return result == std::vector<std::string>({args...});
    }

    void parse(const std::string& s) {
        result = sfcsv::parse_line(s);
        for(const auto& r : result) {
            all.push_back(r);
        }
    }

    void print() {
        std::cout << "Elements: " << std::endl;
        for(const auto& r : result) {
            std::cout << r << std::endl;
        }
        std::cout << std::endl;
    }

    void print_all() {
        std::cout << "All elements: " << std::endl;
        for(const auto& r : all) {
            std::cout << r << std::endl;
        }
        std::cout << std::endl;
    }
};

TEST_F(ParserTest, NonQuotedFields)
{	
    parse("hello");
    EXPECT_TRUE(vec_eq("hello"));

    parse("hello,world");
    EXPECT_TRUE(vec_eq("hello", "world"));

    parse("hello,world,100.0");
    EXPECT_TRUE(vec_eq("hello", "world", "100.0"));
}

TEST_F(ParserTest, NonQuotedFieldsWithMixedQuotes)
{
    ASSERT_ANY_THROW(parse(R"(thisisa"long"word)"));

    ASSERT_ANY_THROW(parse(R"(thisisa""long""word)"));
}

TEST_F(ParserTest, QuotedFields)
{
    parse(R"("hello")");
    EXPECT_TRUE(vec_eq("hello"));

    parse(R"("hello","world")");
    EXPECT_TRUE(vec_eq("hello", "world"));

    parse(R"("hello","world","foobar")");
    EXPECT_TRUE(vec_eq("hello", "world", "foobar"));
}

/**
 * By default, spaces are not allowed in non-quoted fields
 * nor in quoted fields between the separator
 */
TEST_F(ParserTest, NonQuotedFieldsWithSpaces)
{
    parse("hello world");
    EXPECT_TRUE(vec_eq("hello world"));

    parse("hello      world");
    EXPECT_TRUE(vec_eq("hello      world"));

    parse("hello world,bye world");
    EXPECT_TRUE(vec_eq("hello world", "bye world"));

    parse("hello, world");
    EXPECT_TRUE(vec_eq("hello", " world"));

    parse("hello ,world");
    EXPECT_TRUE(vec_eq("hello ", "world"));

    parse("hello , world");
    EXPECT_TRUE(vec_eq("hello ", " world"));

    parse(" hello,world ");
    EXPECT_TRUE(vec_eq(" hello", "world "));
}

TEST_F(ParserTest, QuotedFieldsWithSpaces)
{
    ASSERT_ANY_THROW(parse(R"("hello", "world")"));

    ASSERT_ANY_THROW(parse(R"("hello" ,"world")"));

    ASSERT_ANY_THROW(parse(R"("hello" , "world")"));
}

TEST_F(ParserTest, MixedFields)
{
    parse(R"(hello,"world",foo)");
    EXPECT_TRUE(vec_eq("hello", "world", "foo"));

    parse(R"("hello",world,"foo")");
    EXPECT_TRUE(vec_eq("hello", "world", "foo"));
}

// Test fields with embedded quotes
TEST_F(ParserTest, EmbeddedQuotes)
{

    parse(R"("hello ""world""")");
    EXPECT_TRUE(vec_eq(R"(hello "world")"));

    parse(R"("""hello"" world")");
    EXPECT_TRUE(vec_eq(R"("hello" world)"));

    parse(R"("""hello world""")");
    EXPECT_TRUE(vec_eq(R"("hello world")"));
}

/**
 * Embedded quotes outside double-quoted fields are not permitted
 */
TEST_F(ParserTest, EmbeddedQuotesInvalid)
{
    EXPECT_ANY_THROW(parse(R"(hello "world")"));

    EXPECT_ANY_THROW(parse(R"("hello" world)"));

    EXPECT_ANY_THROW(parse(R"(hello ""world"")"));

    EXPECT_ANY_THROW(parse(R"(""hello"" world)"));

    EXPECT_ANY_THROW(parse(R"(""hello world"")"));

    EXPECT_ANY_THROW(parse(R"(hello """world""")"));

    EXPECT_ANY_THROW(parse(R"("""hello""" world)"));
}

// Test fields with embedded delimiters
TEST_F(ParserTest, EmbeddedDelimiters)
{
    parse(R"(",hello")");
    EXPECT_TRUE(vec_eq(",hello"));

    parse(R"("hello,")");
    EXPECT_TRUE(vec_eq("hello,"));

    parse(R"("hello, world")");
    EXPECT_TRUE(vec_eq("hello, world"));
}

// Test empty fields
TEST_F(ParserTest, EmptyFields)
{
    parse(R"("")");
    EXPECT_TRUE(vec_eq(""));

    parse(R"("","")");
    EXPECT_TRUE(vec_eq("", ""));

    parse(R"("","","")");
    EXPECT_TRUE(vec_eq("", "", ""));

	// Permit empty fields without quotes
    parse("");
    EXPECT_TRUE(vec_eq(""));

    parse(",");
    EXPECT_TRUE(vec_eq("", ""));

    parse(",,");
    EXPECT_TRUE(vec_eq("", "", ""));

    parse(",hello");
    EXPECT_TRUE(vec_eq("", "hello"));

    parse("hello,");
    EXPECT_TRUE(vec_eq("hello", ""));
}

// Test fields that have only double quotes
TEST_F(ParserTest, FieldsWithOnlyDoubleQuotes)
{
	// ", ", ", "
    parse(R"("""")");
    EXPECT_TRUE(vec_eq(R"(")"));

    parse(R"("""","""")");
    EXPECT_TRUE(vec_eq(R"(")", R"(")"));

    parse(R"("""","""","""")");
    EXPECT_TRUE(vec_eq(R"(")", R"(")", R"(")"));

    parse(R"("""""")");
    EXPECT_TRUE(vec_eq(R"("")"));

    parse(R"("""""","""""")");
    EXPECT_TRUE(vec_eq(R"("")", R"("")"));

    parse(R"("""""","""""","""""")");
    EXPECT_TRUE(vec_eq(R"("")", R"("")", R"("")"));
}

// Test fields with multiple lines with \n characters
TEST_F(ParserTest, MultilineFields)
{	
    parse("\"hello\nworld\"");
    EXPECT_TRUE(vec_eq("hello\nworld"));

	// Newlines outside double-quoted fields are not permitted
    // because it's interpreted as a line change and the sfcsv::parse
    // function only parses single lines
    EXPECT_ANY_THROW(parse("hello\nworld"));

    EXPECT_ANY_THROW(parse("hello world,hello\nworld"));
}

// Test odd number of quotes inside quoted fields
TEST_F(ParserTest, OddQuotesInsideField)
{
    EXPECT_ANY_THROW(parse(R"(""Hello" odd quotes")"));

    EXPECT_ANY_THROW(parse(R"("Hello "odd" quotes")"));

    EXPECT_ANY_THROW(parse(R"("Hello odd "quotes"")"));

    EXPECT_ANY_THROW(parse(R"(""""Hello""" odd quotes")"));

    EXPECT_ANY_THROW(parse(R"("Hello """odd""" quotes")"));

    EXPECT_ANY_THROW(parse(R"("Hello odd """quotes"""")"));
}

int main(int argc, char **argv)
{    
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

