/****************************************************************************

The MIT License (MIT)

Copyright (c) 2014 https://github.com/labyrinthofdreams

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*****************************************************************************/

#include <iostream>
#include <iterator>
#include <vector>
#include <string>
#include <QList>
#include <QString>
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

    void parse(const std::string& s, const char sep = ',', const sfcsv::mode mode = sfcsv::mode::strict) {
        decltype(result) tmp;
        sfcsv::parse_line(s, std::back_inserter(tmp), sep, mode);
        tmp.swap(result);
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

TEST_F(ParserTest, EmbeddedQuotes)
{

    parse(R"("hello ""world""")");
    EXPECT_TRUE(vec_eq(R"(hello "world")"));

    parse(R"("""hello"" world")");
    EXPECT_TRUE(vec_eq(R"("hello" world)"));

    parse(R"("""hello world""")");
    EXPECT_TRUE(vec_eq(R"("hello world")"));
}

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

TEST_F(ParserTest, EmbeddedDelimiters)
{
    parse(R"(",hello")");
    EXPECT_TRUE(vec_eq(",hello"));

    parse(R"("hello,")");
    EXPECT_TRUE(vec_eq("hello,"));

    parse(R"("hello, world")");
    EXPECT_TRUE(vec_eq("hello, world"));
}

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

TEST_F(ParserTest, OddQuotesInsideField)
{
    EXPECT_ANY_THROW(parse(R"(""Hello" odd quotes")"));

    EXPECT_ANY_THROW(parse(R"("Hello "odd" quotes")"));

    EXPECT_ANY_THROW(parse(R"("Hello odd "quotes"")"));

    EXPECT_ANY_THROW(parse(R"(""""Hello""" odd quotes")"));

    EXPECT_ANY_THROW(parse(R"("Hello """odd""" quotes")"));

    EXPECT_ANY_THROW(parse(R"("Hello odd """quotes"""")"));
}

TEST_F(ParserTest, Separator)
{
    EXPECT_ANY_THROW(parse(R"("hello","world")", ';'));

    parse("hello;world", ';');
    EXPECT_TRUE(vec_eq("hello", "world"));

    parse("hello\tworld", '\t');
    EXPECT_TRUE(vec_eq("hello", "world"));

    parse(R"("hello";"world")", ';');
    EXPECT_TRUE(vec_eq("hello", "world"));

    parse("\"hello\"\t\"world\"", '\t');
    EXPECT_TRUE(vec_eq("hello", "world"));
}

TEST_F(ParserTest, LooseDoubleQuotesInField)
{
    EXPECT_NO_THROW(parse(R"(hello,"this is not a "film",world)", ',', sfcsv::mode::loose));
    EXPECT_TRUE(vec_eq("hello", "this is not a \"film", "world"));

    EXPECT_NO_THROW(parse(R"(hello,"this is not a ""film",world)", ',', sfcsv::mode::loose));
    EXPECT_TRUE(vec_eq("hello", "this is not a \"film", "world"));

    EXPECT_NO_THROW(parse(R"(hello,"this is not a """film",world)", ',', sfcsv::mode::loose));
    EXPECT_TRUE(vec_eq("hello", "this is not a \"\"\"film", "world"));



    EXPECT_NO_THROW(parse(R"(hello,aa"bb,world)", ',', sfcsv::mode::loose));
    EXPECT_TRUE(vec_eq("hello", "aa\"bb", "world"));

    EXPECT_NO_THROW(parse(R"(hello,aa""bb,world)", ',', sfcsv::mode::loose));
    EXPECT_TRUE(vec_eq("hello", "aa\"\"bb", "world"));

    EXPECT_NO_THROW(parse(R"(hello,aa"""bb,world)", ',', sfcsv::mode::loose));
    EXPECT_TRUE(vec_eq("hello", "aa\"\"\"bb", "world"));
}

TEST_F(ParserTest, LooseNewlinesInNonQuotedFields)
{
    EXPECT_NO_THROW(parse("hello\nworld", ',', sfcsv::mode::loose));
    EXPECT_TRUE(vec_eq("hello\nworld"));
}

struct QtStringPolicy {
    template <class StringT, class CharT = typename StringT::value_type>
    static void append(StringT &str, const CharT c) {
        str.append(c);
    }

    template <class StringT, class CharT = typename StringT::value_type>
    static void append(StringT &str, const unsigned count, const CharT c) {
        for(unsigned i = 0; i < count; ++i) {
            str.append(c);
        }
    }

    template <class StringT>
    static bool empty(const StringT &str) {
        return str.isEmpty();
    }
};

TEST_F(ParserTest, QStringTest)
{
    QList<QString> parsed;
    QString str = "hello,world";
    sfcsv::parse_line<QtStringPolicy>(str, std::back_inserter(parsed), ',');
    EXPECT_TRUE(parsed.size() == 2);
    EXPECT_TRUE(parsed.at(0) == "hello");
    EXPECT_TRUE(parsed.at(1) == "world");
}

int main(int argc, char **argv)
{    
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

