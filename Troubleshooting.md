Troubleshooting FAQ
===================

C and C++ are not the simplest languages on the planet, and using them on an embedded system doesn't help. Hopefully
you'll be able to find answers to some of your problems on this page.

Can't find what you're looking for? Ask on the 
[Parallax forums](http://forums.parallax.com/forumdisplay.php/65-Propeller-1-Multicore-Microcontroller) or 
[contact the author](http://david.zemon.name/professional/contact.shtml).

error: no matching function for call to 'PropWare::Printer::print(const PropWare::Printer&, PropWare::Printer::Format&) const'
------------------------------------------------------------------------------------------------------------------------------

`PropWare::Printer::printf` only supports a select few parameters. The first argument must be a string (character 
array), and all other arguments (if they exist) must be one of the following types:
* Any integer type (`uint8_t`, `int`, `unsigned int`, `long`, `int32_t`, `size_t`, etc)
* `float` or `double`
* Any pointer type (`int *`, `MyClass *`, etc). Note that `char *` will be treated specially - as a string, not an 
  `uint32_t`
