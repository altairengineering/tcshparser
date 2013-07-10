tcsh-alias-expander
===================

This program simulates the way in which aliases are expanded by
tcsh. It is modelled on a Java program which has been a part of
[Ellexus Breeze](http://www.ellexus.com/breeze.php).

Breeze includes a script which traces the execution of an arbitrary
program. For example, to see what happens when you use Firefox to
visit a web site, you might type something like:

    trace-program.sh -f output firefox www.ellexus.com

But if you're using tcsh and try to trace something which is an alias,
for example:

    alias ff firefox
    trace-program.sh -f output ff www.ellexus.com

then the "ff" alias is not expanded, because tcsh only looks for
aliases as the first word of each simple command. (By prefixing the
command with "trace-program.sh -f output", "ff" has become the fourth
word, and tcsh leaves it unchanged.)

The trace-program.sh script in Ellexus Breeze has an optional command
"-at <alias file>", which passes the command that the user has typed
through the tcshParser program. This expands any aliases it finds and
makes examples such as that above just work.

This version of tcshParser is written in C, and needs only the usual
glibc library. To build it:

    cd src
    make

To run the program first copy tcshParser to somewhere on your
path. Save the definitions of all your aliases in a file by typing
something like:

    alias > alias.txt

Now run tcshParser, giving it first the "alias.txt" file and then your command:

    tcshParser alias.txt ff www.ellexus.com

and the program will print...

    firefox www.ellexus.com

Tcsh aliases can use "history" substitutions, and tcshParser handles
these as well.  The "test" directory contains a script which runs a
number of test cases through the program and checks that the output is
what was expected. But do let us know if it gets something wrong!

    cd test
    ./test.sh

This code is licensed under the GPL, but if this is a problem for you
then please contact us.

http://www.ellexus.com/contact.php
