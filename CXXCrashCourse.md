C++ Crash Course
================

For those coming from a background in Parallax's Spin language, your only concern will be syntax: the C-family of 
languages is wildly different. Don't worry - it's 
not hard.

For C developers with no background in Spin, the biggest hurdle will be learning about the famed "object-oriented 
programming" (still not hard).

There are lots of code examples throughout PropWare and libpropeller's source code, so please feel free to peruse them.
You should also take advantage of the example projects provided in the `Examples` directory.

C-Family Syntax
---------------
* [Complete syntax reference](http://www.cprogramming.com/reference/)
* Capitalization matters
* Whitespace is not necessary except as a delimiter between words (you may format your code however you please)
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
  // A function that takes no parameters and does not return a value
  void main () {
      // Some code here
  }
  
  // C will prefer the above function slightly different - they both mean the same thing
  void main (void) {
      // Some code here
  }
  
  // This function returns an integer of basic type `int`
  int get_x () {
      int x = 5;
      return x;
  }
  
  // A function that takes two parameters, `x` and `y` and returns the sum
  int add (int x, int y) {
      return x + y;
  }
  ~~~~~~~~~~~~~~~~~
* Loops by example
  ~~~~~~~~~~~~~~~~~{.c}
  // Basic while-loop
  while (x == y) {
      // Do stuff
  }
  
  // C++ allows you to declare your index (`i`) within the for-loop
  for (int i = 0; i < 10; i++) {
      printf("The current value of `i`: %d", i);
  }
  
  // C must have the index declared prior to the loop
  int i;
  for (i = 0; i < 10; i++) {
      printf("The current value of `i`: %d", i);
  }
  
  // A simple do-while
  do {
      // Some stuff
  } while (x == y);
  ~~~~~~~~~~~~~~~~~

Object-Oriented Programming
------------------
Classes. It's all about classes: organizing code into logical blocks. If you have 3 functions that all deal with a
serial terminal, you might want to categorize them as `Serial`. That's all a `class` is - a group of functions that
belong together. This code might look something like

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
class Serial {
    public:
        void start (int baudrate) {
            // Initialize the serial terminal
        }
        
        void send_character (char c) {
            // Some code to send a character over the serial bus
        }
        
        char receive_character () {
           char c;
           // Some code to receive a character from the serial bus and store it in `c`
           return c;
        }
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Notice that, just like Parallax's Spin, we have visibility keywords: `public` in this case. This keyword marks 
everything beneath it as accessible to the outside world, just like Spin's `PUB` keyword before a function.
 
Where Spin might have some code such as
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.spin}
Obj
  pst : "Parallax Serial Terminal"
  
PUB Main | receivedChar
  pst.start(9600)
  pst.Char('A')
  receivedChar := pst.CharIn
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

C++ would write this as
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
void main () {
    Serial pst;                                // Instantiation of the `Serial` object
    char receivedChar;                         // Instantiation of the receivedChar variable
    
    pst.start(9600);                           // Starting our Serial terminal at 9,600 baud
    pst.send_character('A');                   // Send the single character, 'A', out the serial bus
    receivedChar = pst.receive_character();    // Store a character from the serial bus into `receivedChar`
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Namespaces
----------
The term "Serial" can be pretty generic. Maybe David's Serial object has a max baud of 115,000 but Bob's Serial object 
has configurable pins (so you don't have to use the default RX and TX hardware pins). You want to use BOTH Serial 
objects. In order to differentiate between David's `Serial` class and Bob's `Serial` class, we prefix the class names 
with a `namespace`. This looks like
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
namespace PropWare {
    class Serial {
        // David's code here
    }
}

namespace BobWare {
    class Serial {
        // Bob's code here
    }
}

void main () {
    PropWare::Serial davidSerial;
    BobWare::Serial bobSerial;
    
    davidSerial.call_function();
    bobSerial.call_function()
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now that both classes are surrounded by a namespace, our code can use both classes without conflict! 

Including Separate Files
------------------------
Coming soon!
