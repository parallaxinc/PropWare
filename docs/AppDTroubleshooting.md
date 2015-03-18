Appendix D: Troubleshooting FAQ
===============================

[<< Appendix C: Limitations](http://david.zemon.name/PropWare/md_docs_AppCLimitations.html)<br />

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

Looks like I cant clone or update the git repository for libpropeller. Sorry!
-----------------------------------------------------------------------------

`libpropeller` is a fantastic C++ library provided by forum user SRLM. His code resides in a separate github repository.
%PropWare's installation script attempts to use `git` to pull the latest changes for `libpropeller` before recompiling.
The above message is displayed any time that update fails.

[<< Appendix C: Limitations](http://david.zemon.name/PropWare/md_docs_AppCLimitations.html)<br />
