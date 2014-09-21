C++ Crash Course
================

For those coming from a background in Parallax's Spin language, your only concern will be syntax: the C-family of 
languages is wildly different. Don't worry - it's 
not hard.

For C developers, the biggest hurdle will be learning about the famed "object-oriented programming" (still not hard).

C-Family Syntax
---------------
* [Complete syntax reference](http://www.cprogramming.com/reference/)
* Capitalization matters
* Whitespace does not
* Every line of code must be terminated with `;`
* Assign variables with `=`
* Compare variables with `==`, `!=`, `>=`, `<=` and `!`
* Append boolean expressions with `&&` for AND and `||` for OR
* Use `&`, `|`, and `^` for the bitwise operations AND, OR and XOR respectively
* Single line comments are marked with `//`
  ~~~~~~~~~~~~~~~~~{.c}
  // This is a comment
  ~~~~~~~~~~~~~~~~~
* Multi-line comments are marked with `/*` and `*/`
  ~~~~~~~~~~~~~~~~~{.c}
  /* 
   A multi-line comment!
   */
  ~~~~~~~~~~~~~~~~~
* A variable must be declared before use, but could have an initializer tacked on
  ~~~~~~~~~~~~~~~~~{.c}
  int x;      // Remember, capitalization matters!
  int X = 5;  // These are two different variables!
  ~~~~~~~~~~~~~~~~~
* A `block` of code is marked with `{` and `}`
  ~~~~~~~~~~~~~~~~~{.c}
  if (x == y) {
      // A block of code only executed if `x` is equal to `y`
  }
  ~~~~~~~~~~~~~~~~~
* A function must declare what type of variable will be returned (use `void` if no return)
  ~~~~~~~~~~~~~~~~~{.c}
  void main () {
      // Some code here
  }
  
  int get_x () {
      // Some code to calculate an integer x
      return x;
  }
  ~~~~~~~~~~~~~~~~~
* For-loop: `for (<init>; <condition>; <end-of-iteration code>) {<Code executed each iteration>}`
  ~~~~~~~~~~~~~~~~~{.c}
  // C++ allows you to declare `i` within the for-loop
  for (int i = 0; i < 10; i++) {
      printf("The current value of `i`: %d", i);
  }
  
  // C must have `i` declared prior to the loop
  int i;
  for (i = 0; i < 10; i++) {
      printf("The current value of `i`: %d", i);
  }
  ~~~~~~~~~~~~~~~~~

Object-Oriented Programming
------------------
Classes. It's all about classes: organizing code into logical blocks. If you have 10 functions that all deal with a
serial terminal, you might want to categorize them as `Serial`. Congratulations! You just made a class called `Serial`. 

Namespaces
----------
Coming soon!

