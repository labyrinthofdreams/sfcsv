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

#ifndef SFCSV_H
#define SFCSV_H

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace sfcsv {

/**
 * @brief Exception class for csv errors
 */
struct csv_error : public std::runtime_error {
    csv_error(const char *msg) : std::runtime_error(msg) {}
};

/**
 * @brief Parser mode
 */
enum class mode {
    strict,
    loose
};

/**
 * @brief Default policy for strings
 */
struct default_policy {
    template <class StringT, class CharT = typename StringT::value_type>
    static void append(StringT &str, const CharT c) {
        str += c;
    }

    template <class StringT, class CharT = typename StringT::value_type>
    static void append(StringT &str, const unsigned count, const CharT c) {
        str.append(count, c);
    }

    template <class StringT>
    static bool empty(const StringT &str) {
        return str.empty();
    }
};

/**
 * @brief Parse a CSV line from string
 * @pre StringT must have cbegin()/cend() that satisfy InputIterator
 * @pre StringT must be default initializable
 * @pre StringT must have value_type
 * @pre OutIter must satisfy OutputIterator
 * @param s String to parse
 * @param out Output iterator
 * @param sep Field separator
 * @param pmode Parsing mode
 * @throws std::runtime_error If double quotes in non-quoted fields (strict mode)
 * @throws std::runtime_error If invalid separator after a field
 * @throws std::runtime_error If newline character in non-quoted field (strict mode)
 */
template <class StringPolicy = default_policy, class StringT, class OutIter,
          class CharT = class StringT::value_type>
void parse_line(const StringT& s, OutIter out, 
                const CharT sep = ',', const mode pmode = mode::strict) {
    bool in_quotes = false;
    StringT field;    
    for(auto it = s.cbegin(), end = s.cend(); it != end; ++it) {
        const auto c = *it;
        if(c == '"') {
            if(!in_quotes && !StringPolicy::empty(field)) {
                if(pmode == mode::loose) {
                    StringPolicy::append(field, '"');
                }
                else {
                    throw csv_error("Double quotes not permitted in non-quoted fields");
                }
            }
            else {
                // Find one past last quote
                const auto last_quote = std::find_if(it, end, [](const auto c){
                    return c != '"';
                });
                const auto num_quotes = std::distance(it, last_quote);

                // Enclosing quote starts or ends a field
                const bool enclosing = num_quotes % 2 != 0;

                if(in_quotes && enclosing && last_quote != end
                        && *(last_quote) != sep && pmode == mode::loose) {
                    StringPolicy::append(field, num_quotes, '"');
                    it = last_quote;
                }
                else {
                    // Ignore one quote for an enclosing group, two for an empty field
                    // or field with only quotes, and zero for embedded groups
                    const auto ignore_quotes = enclosing ? 1 : (StringPolicy::empty(field) ? 2 : 0);
                    StringPolicy::append(field, ((num_quotes - ignore_quotes) / 2), '"');

                    if(enclosing) {
                        in_quotes = !in_quotes;
                    }

                    it = last_quote;
                    if(!in_quotes && it != end && *(it) != sep && pmode == mode::strict) {
                        // If next character after field ending quote is not a separator
                        throw csv_error("Invalid separator after a field");
                    }
                }

                --it;
            }
        }
        else if(c == sep && !in_quotes) {
            // Separator ends field
            *out++ = std::move(field);
            field.clear();
        }
        else if(c == '\n' && !in_quotes && pmode == mode::strict) {
            throw csv_error("Newline characters are not permitted in non-quoted fields");
        }
        else {
            StringPolicy::append(field, c);
        }
    }

    // Push last field in result
    *out++ = std::move(field);
}

/**
 * @brief Encode a single string field
 *
 * This function duplicates all quote characters
 * and encloses the string with quote characters
 *
 * @pre StringT must have .reserve(size)
 * @pre StringT must have operator+=
 * @pre StringT must have .begin()/.end() that satisfy InputIterator
 * @pre StringT must be default initializable
 * @param s String to encode
 * @return Encoded string
 */
template <class StringT>
StringT encode_field(const StringT& s) {
    StringT out;
    // Reserve space for old string, enclosing quotes, and embedded quotes
    out.reserve(s.size() + 2 + std::count(std::begin(s), std::end(s), '"'));
    out += '"';
    for(const auto c : s) {
        out += c;
        if(c == '"') {
            out += c;
        }
    }

    out += '"';
    return out;
}

/**
 * @brief Encode strings from iterator range start to end
 * @pre InIter must satisfy InputIterator
 * @pre OutIter must satisfy OutputIterator
 * @param start Iterator to the begin position
 * @param end Iterator to the end position
 * @param out Iterator to output
 * @param sep Field separator
 */
template <class InIter, class OutIter, class CharT = char>
void encode_line(InIter start, InIter end, OutIter out, const CharT* sep = ",") {
    while(start != end) {
        *out++ = encode_field(*start);
        if(++start != end) {
            *out++ = sep;
        }
    }
}

} // namespace sfcsv

#endif // SFCSV_H
