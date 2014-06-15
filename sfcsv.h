#ifndef SFCSV_H
#define SFCSV_H

#include <algorithm>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace sfcsv {

template <class StringT, class OutIter>
void parse_line(const StringT& s, OutIter out, const char sep = ',');

std::vector<std::string> parse_line(const std::string &s, const char sep = ',') {
    std::vector<std::string> out;

    parse_line(s, std::back_inserter(out), sep);

    return out;
}

/**
 * @brief Parse a CSV line from string
 * @pre Parameter s must have begin()/end() that satisfy InputIterator
 * @pre Parameter s must be default initializable
 * @pre Parameter s must have operator+=
 * @pre Parameter out must satisfy OutputIterator
 * @param out Output iterator
 * @param sep Field separator
 * @throws std::runtime_error If double quotes in non-quoted fields
 * @throws std::runtime_error If invalid separator after a field
 * @throws std::runtime_error If newline character in non-quoted field
 */
template <class StringT, class OutIter>
void parse_line(const StringT& s, OutIter out, const char sep = ',') {
    bool in_quotes = false;
    StringT field;

    for(auto it = s.begin(), end = s.end(); it != end; ++it) {
        const auto c = *it;
        if(c == '"') {
            if(!in_quotes && !field.empty()) {
                throw std::runtime_error("Double quotes not permitted in non-quoted fields");
            }

            // Find one past last quote
            const auto last_quote = std::find_if(it, end, [](const char c){
                return c != '"';
            });
            const auto num_quotes = std::distance(it, last_quote);

            if(num_quotes % 2 == 0) {
                // Even number of quotes, either an empty field or embedded quotes
                const auto quotes = field.empty() ? ((num_quotes - 2) / 2)
                                                  : (num_quotes / 2);
                field.append(quotes, '"');
            }
            else {
                // Odd number of quotes, a field either starts or ends
                in_quotes = !in_quotes;
                field.append(((num_quotes - 1) / 2), '"');
            }

            it = last_quote;
            if(!in_quotes && it != end && *(it) != sep) {
                // If next character after field ending quote is not a separator
                throw std::runtime_error("Invalid separator after a field: " + *(it));
            }
            --it;
        }
        else if(c == sep && !in_quotes) {
            // Separator ends field
            *out++ = field;
            field.erase();
        }
        else if(c == '\n' && !in_quotes) {
            throw std::runtime_error("Newline characters are not permitted in non-quoted fields");
        }
        else {
            field += c;
        }
    }

    // Push last field in result
    *out++ = field;
}

/**
 * @brief Encode a single string field
 *
 * This function duplicates all quote characters
 * and encloses the string with quote characters
 *
 * @pre Parameter s must have .reserve(size), operator+=,
 * and it must have .begin()/.end() that satisfy InputIterator
 *
 * @param s String to encode
 * @return Encoded string
 */
template <class StringT>
StringT encode_field(const StringT& s) {
    StringT out;
    // Reserve space for old string, surrounding quotes, duplicated quotes
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
 * @pre Start and end parameters must satisfy InputIterator
 * @pre Out parameter must satisfy OutputIterator.
 * @param start Iterator to the begin position
 * @param end Iterator to the end position
 * @param out Iterator to output
 * @param sep Field separator
 */
template <class InIter, class OutIter, class Char = char>
void encode_line(InIter start, InIter end, OutIter out, const Char* sep = ",") {
    while(start != end) {
        *out++ = encode_field(*start);
        if(++start != end) {
            *out++ = sep;
        }
    }
}

} // namespace sfcsv

#endif // SFCSV_H
