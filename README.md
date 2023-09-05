# SuffixTree

C++ implementation of suffix trees, using Ukkonen's algorithm.

A suffix tree is a tree that stores all suffixes of a given input text in a compressed form.
Once the tree is created, it allows the efficient execution of many useful string operations.
For example, all occurrences of a pattern string in the text can be found by just traversing the pattern
string along the tree instead of searching the whole text.

The construction of a suffix tree takes time linear in the length of the text.
Suffix trees are most useful when the text is fixed or does not change often.


## Requirements

C++11

This library uses the `#pragma once` directive, which is non-standard but widely supported.
You either need a compiler that supports it or replace the directive with include guards.

Note: Suffix trees internally have to use an "end of text" marker that must not occur in the text otherwise.
By default, this library uses [the "end of text" ASCII character](https://en.wikipedia.org/wiki/End-of-text_character) (`0x03`).
If your text contains this character, please supply a different unique character to the constructor of the `SuffixTree` class.

## Usage

Simply include the library as a subdirectory in your CMake project and it will compile along your project.

For a standalone compilation use the usual CMake procedure:

```
cmake -B build
cmake --build build
```

or, for older versions of CMake

```
mkdir build
cd build
cmake ..
make
```


## Examples

Here is an example on how to use a suffix tree for string matching.

```
#include <suffixtree/SuffixTree.h>

// [...]

const std::string text = "bismarck biss mark bis mark bismarck biss";
const std::string pattern_1 = "ss m";
const std::string pattorn_2 = "ark biss";
const std::string pattern_3 = "bis";

SuffixTree tree(text);
const bool contains_1 = tree.contains(pattern_1);                  // true
const bool contains_2 = tree.contains(pattern_2);                  // false
const std::vector<size_t> occurrences_3 = tree.find(pattern_3);    // {19, 37, 9, 28, 0};

```

