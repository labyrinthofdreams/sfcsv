###SFCSV - Simple & Fast CSV parser

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

####API usage:

```c++
template <class StringT, class OutIter, class CharT = class StringT::value_type>
void parse_line(const StringT& s, OutIter out, const CharT sep = ',');
```

**Examples**:

Parsing from a string:  
```c++
std::string csv("one,two,three");
std::vector<std::string> parsed;
parse_line(csv, std::back_inserter(parsed), ';');
```

Parsing from a file:  
```c++
std::ifstream infile("stats.csv");
std::string line;
while(std::getline(infile, line)) {
    std::vector<std::string> parsed;
    parse_line(line, std::back_inserter(parsed), ';');
    // ... do something with parsed row ...
}
```

```c++
template <class InIter, class OutIter, class Char = char>
void encode_line(InIter start, InIter end, OutIter out, const Char* sep = ",");
```

Note that the separator is a string literal

**Examples**:

Encoding into a string:  
```c++
std::vector<std::string> cols {"hello", "world", "and", "universe"};
std::ostringstream os;
encode_line(cols.cbegin(), cols.cend(), std::ostream_iterator<std::string>(os), ";");
std::string s = os.str();
```

Outputting to stdout:  
```c++
std::vector<std::string> cols {"hello", "world", "and", "universe"};
encode_line(cols.cbegin(), cols.cend(), std::ostream_iterator<std::string>(std::cout), ";");
```
