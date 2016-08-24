Contribute to %PropWare {#Contribute}
=======================

Code contributions can be made by either [contacting David Zemon](mailto:david@zemon.name) or forking the repository 
and creating a pull request. If you'd like to help but don't know how, start by perusing the 
[issue tracker](https://github.com/parallaxinc/PropWare/issues/) and finding something that piques your interest.

Before submitting your pull request, please be sure to read the following guidelines for %PropWare's codebase. 

Coding Style
------------

### Whitespace & Brackets

* Indentation is 4 spaces
* No indentation within `namespace`
* One indentation for visibility keywords in a class
* One indentation relative to visibility keyword for members and methods of a class
* Members and methods of basic `struct`s with no visibility keywords are indented once relative to class declaration
* Open bracket is placed on the same line
* Closing bracket is placed on the next line
* Single-line statements should not use brackets

### Coding Conventions

* Use of methods and member variables should be prefixed with `this->`
* Method names use underscore, not camelCase
* Member variables are prefixed with `m_` and use camelCase (such as `m_myVariable`)
* Names are never abbreviated unless the name is excessively long or the acronym is only used colloquially, such as 
  "VGA" or "SPI"
* Class names use CamelCase, with the first letter capitalized
* Constant variables should be in all caps, separated by underscores

Coding Best Practices
---------------------

### enum class

`enum class MyEnum {...};` should be preferred over `typedef enum {...} MyEnum;`. Exceptions to this rule are,
however, frequent. `enum class` will be relegated to abstract concepts which are never or infrequently
converted to their ordinal values. "color"s and "mode"s of operation are often good examples of a sufficiently
abstract concept to use `enum class`. "Mode" would be a bad example, though, if the ordinal values represent
specific bits in a register, and those bits need to be accessed frequently. The `enum class` is a method to _prevent
automatic casting *down* to an `int`_. Remember that standard `enum`s already avoid automatic casting up to the enum. 
