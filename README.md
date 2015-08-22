###SFCSV - Simple & Fast CSV parser

SFCSV is a single header CSV parser/encoder written in C++14
with no external dependencies. The tests require Googletest + Qt.

What works (see tests/main.cpp):

Fields without quotes:  
```one,two,spaces work too,three,four```

Fields with quotes:  
```"one","two","spaces work too"```

But you can't have spaces between the separator:  
```"one" , "two"```

Embedded quotes:  
```"hello ""world"" and ""universe""","more """"quotes"""""```

But you can't have embedded quotes in fields without quotes:  
```hello ""world"",foo,bar```

...Or an uneven number of quotes:  
```"hello "world"","hello """world"""```

Empty fields work like so:  
```
,,,  
"","","",""
```

Fields with only quotes work as well:  
`"""","""""",""""""""`

Newlines must be inside quoted fields, otherwise it fails:  
`"hello\nworld"`

And all the different field types can be mixed.

The separator can be changed too, e.g.:  
```
one;two;three  
one\ttwo\tthree
```

You can enable a "loose" mode which does the following:

1) Allow odd quotes inside quoted fields:  
`"abc"def" => abc"def`  
`"abc""def" => abc"def`  
`"abc"""def" => abc"""def` 

Note: Even number of consecutive quotes are still treated normally!

2) Allow quotes in non-quoted fields:  
`abc"def => abc"def`  
`abc""def => abc""def`  
`abc"""def => abc"""def`  

Note: Unlike in quoted fields, quotes always match source count 

3) Allow newlines in non-quoted fields:

`hello\nworld`

####Structures:

enum class Mode {
    Strict,
    Loose
};

####API usage - parse_line:

```c++
template <class StringPolicy = DefaultPolicy, class StringT, class OutIter, class CharT = class StringT::value_type>
void parse_line(const StringT& s, OutIter out, const CharT sep = ',', const Mode mode = Mode::Strict);
```

#####Examples:

Parsing from a string:  
```c++
std::string csv("one;two;three");
std::vector<std::string> parsed;
sfcsv::parse_line(csv, std::back_inserter(parsed), ';');
```

Parsing from a string in loose mode:  
```c++
std::string csv("one;two;three");
std::vector<std::string> parsed;
sfcsv::parse_line(csv, std::back_inserter(parsed), ';', sfcsv::Mode::Loose);
```

Parsing from a file:  
```c++
std::ifstream infile("stats.csv");
std::string line;
while(std::getline(infile, line)) {
    std::vector<std::string> parsed;
    sfcsv::parse_line(line, std::back_inserter(parsed), ';');
    // ... do something with parsed row ...
}
```

Parsing Qt QStrings:
```c++
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

QList<QString> parsed;
QString str = "hello,world";
sfcsv::parse_line<QtStringPolicy>(str, std::back_inserter(parsed), ',');
```

####API usage - encode_line:

```c++
template <class InIter, class OutIter, class CharT = char>
void encode_line(InIter start, InIter end, OutIter out, const CharT* sep = ",");
```

Note that the separator is a string literal

#####Examples:

Encoding into a string:  
```c++
std::vector<std::string> cols {"hello", "world", "and", "universe"};
std::ostringstream os;
sfcsv::encode_line(cols.cbegin(), cols.cend(), std::ostream_iterator<std::string>(os), ";");
std::string s = os.str();
```

Outputting to stdout:  
```c++
std::vector<std::string> cols {"hello", "world", "and", "universe"};
sfcsv::encode_line(cols.cbegin(), cols.cend(), std::ostream_iterator<std::string>(std::cout), ";");
```
