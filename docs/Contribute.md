Contribute to %PropWare {#Contribute}
=======================

Code contributions can be made by either [contacting David Zemon](mailto:david@zemon.name) or forking the repository 
and creating a pull request. If you'd like to help but don't know how, start by perusing the 
[issue tracker](https://github.com/DavidZemon/PropWare/issues/) and finding something that piques your interest.

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
