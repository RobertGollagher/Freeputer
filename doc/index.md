<head>
<title>Freeputer</title>
<meta http-equiv="content-type" content="text/html;charset=utf-8">
<meta name="keywords" content="Freeputer, Freelang, FreeLine, Robert Gollagher">
<meta name="description" content="The official Freeputer website"
</head>

# Freeputer

Freeputer ( ) \[ \] { } forever free

## What is Freeputer?

Freeputer is a free computer. [Free](http://www.fsf.org/) as in freedom.

Freeputer is a tiny virtual machine easily ported to most modern architectures, including bare metal, and requiring neither file system nor operating system. This offers extreme portability and the freedom to use software forever without designed obsolescence.

* Freeputer is a virtual machine: the Freeputer Virtual Machine (FVM).
* Freeputer is a self-hosted, self-contained software development platform.
* Freeputer is a platform for modular software that lasts forever.
* Freeputer is a powerful platform for bare metal computing.
* Freeputer is a user's computer not a vendor's computer.
* Freeputer supports the freedom of the user.
* Freeputer is a not an operating system.

## History

Freeputer is brand new. It was released on 8 August 2015.

Freeputer and Freelang are the product of several years of independent research by Robert Gollagher. Although the current implementation is brand new, the general design concept has been well proven by earlier prototypes. The Freelang language is stable and productive.

<font color="#000099">**For the latest news see [Milestones](#milestones) below.**</font>

## Freeputer says 你好 ...

Here is the output from a simple Freeputer program:

![Hello from Freeputer](img/Hello-World.png "Hello from Freeputer")

As you can see, the program greets the user in Chinese and English.

The compiled program is 276 *bytes* in size. It requires neither an operating system nor a file system. It was written in Freelang, a stack-oriented, concatenative, compiled language for Freeputer. The Freelang compiler (known as flc) is self-hosted; that is, it is written in Freelang, runs on the FVM and can compile itself.

## Freeputer edits text ...

You can create, edit, compile and run programs on Freeputer instances. For example, the FreeLine text editor is written in Freelang and runs on Freeputer. It is accessed from a terminal emulator.

Here is FreeLine being used to edit the source code for itself:

![The Freelang source code for FreeLine, viewed in FreeLine](img/FreeLine-Source-Code.png "The Freelang source code for FreeLine, viewed in FreeLine")

For comparison, here's the same code in [gedit](https://en.wikipedia.org/wiki/Gedit):

![The Freelang source code for FreeLine, viewed in gedit](img/gedit-FreeLine.png "The Freelang source code for FreeLine, viewed in gedit")

## The future

Freeputer and Freelang are free and extensible.

What will you make with Freeputer?

## For further details

Please visit the Freeputer project on [GitHub](https://github.com/RobertGollagher/Freeputer) or [Bitbucket](https://bitbucket.org/RobertGollagher/freeputer/src).

---

## Milestones

- **New platform added: Java**
<br/>FVM 1.0 implementation for Java 6, Java 7, Java 8
<br/>0.1.0.0 version of FVM.java <font color="red">**`alpha`**</font>
<br/>See: `dev/fvm/java/src/com/freeputer/FVM.java`
<br/>Known to run on Java 8 for x86 Linux (desktop), ARM Linux (Raspberry Pi), Windows 8 (tablet)
<br/>Known to run on Java 6 for x86 Linux (desktop)
<br/>Should also run on <font color="#000099">**numerous devices that support Java 6 or higher**</font>
<br/>Should also run on OS X and other popular operating systems
<br/>Should also run on embedded devices that support Java
<br/>*15 September 2015*

- **New platform added: C**
<br/>FVM 1.0 implementation for gcc
<br/>Portable 0.1.0.0 version of fvm.c <font color="red">**`alpha`**</font>
<br/>See: `dev/fvm/c/src/fvm.c` and `Makefile`
<br/>Known to run on <font color="#000099">**x86 Linux**</font> (desktop), <font color="#000099">**ARM Linux**</font> (Raspberry Pi) and <font color="#000099">**Windows&nbsp;8**</font> (tablet)
<br/>Should also run on <font color="#000099">**numerous devices that gcc can target**</font>
<br/>Should also run on OS X and other popular operating systems
<br/>Should also run on microcontrollers
<br/>Should also run on bare metal
<br/>*13 September 2015*

- **First public release of Freeputer 1.0.0.0**
<br/>FVM 1.0 implementation in <font color="#000099">**x86 assembly language for 32-bit Linux**</font>
<br/>1.0.0.0 version of fvm.s (the reference implementation of the FVM 1.0) <font color="green">**`beta`**</font>
<br/>See: `dev/fvm/x86/src/fvm.s` and subdirectories of `dev/freelang` and `dev/xcompiler`
<br/>1.0.0.0 versions of flc, fld, fvmtest and FreeLine (in Freelang) <font color="green">**`beta`**</font>
<br/>1.0.0.0 versions of flx (in Ruby) <font color="green">**`beta`**</font>
<br/>*8 August 2015*

---

Copyright © Robert Gollagher 2015  
This document was written by Robert Gollagher.  
This document was first published on 7 August 2015.  
This document was last updated on 15 September 2015 at 15:12.  
This document is licensed under a [Creative Commons Attribution-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-sa/4.0/).

[![](img/80x15.png)](http://creativecommons.org/licenses/by-sa/4.0/)


The official Freeputer website is [freeputer.net](http://www.freeputer.net).  
Robert Gollagher may be reached at

![](img/abc.png)

---
