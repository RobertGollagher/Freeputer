<meta http-equiv="content-type" content="text/html;charset=utf-8">

# Freeputer

Freeputer ( ) \[ \] { } forever free

## What is Freeputer?

Freeputer is a free computer. Free as in freedom.

Freeputer is a tiny virtual machine easily ported to most modern architectures, including bare metal, and requiring neither file system nor operating system. This offers extreme portability and the freedom to use software forever without designed obsolescence.

* Freeputer is a virtual machine: the Freeputer Virtual Machine (FVM).
* Freeputer is a self-hosted, self-contained software development platform.
* Freeputer is a platform for modular software that lasts forever.
* Freeputer is a powerful platform for bare metal computing.
* Freeputer is a user's computer not a vendor's computer.
* Freeputer supports the freedom of the user.
* Freeputer is a not an operating system.

## Where can I learn more?

For an introduction to Freeputer, including a Quick Start tutorial, see [README.md](archive/1.0/README.md) or [README.html](archive/1.0/README.html).

To learn Freeputer you should use the provided [Freeputer 1.0](archive/1.0) archive (equivalent to the git tag **fpv1.0.0.2**) since that is the version for which the Quick Start tutorial was written. Using Freeputer 1.0 will teach you the basic principles of Freeputer and Freelang while avoiding unnecessary complexity.

See also [Other Resources](#other-resources) below.

## What's the latest news?

Freeputer 1.1 is coming!

Expected as an alpha release by mid-2016, it should include:

* an FVM 1.1, with some additional instructions
* continued support for targeting x86 or gcc or Linux or Java
* slightly enhanced versions of the Freelang compilers flc and flx
* support for an extraordinarily portable textual user-interface (the 'tape')
* support for Arduino (AVR, ARM) and chipKIT (PIC32) boards via the Arduino IDE
* support for TI Launchpad (MSP430, ARM) boards via the Energia IDE
* design enhancements meant to facilitate future standalone use

Most of the above has already been successfully spiked.

Freeputer 1.1 is fundamentally about hardware freedom. For example, it very easily allows a Freeputer Virtual Machine to be deployed to an Arduino as a bare-metal, dedicated, interactive system with its own PS/2 keyboard and simple LCD display. Alternatively, it very easily allows that Arduino to be connected via serial connection to a remote 'tape terminal' with a more sophisticated display (driven by another Arduino or running on a Linux computer). Such hardware freedom is a prerequisite to true software freedom. That the same software module can easily be run on a tiny Arduino or on a powerful server demonstrates true freedom and protects your investment in writing that software.

## So where's the code?

The complete Freeputer 1.0 code is in the [archive/1.0](archive/1.0) folder (git tag **fpv1.0.0.2**). The Freeputer 1.1 code isn't online yet; you will gradually see some of it start to appear in the pre-alpha folder as work proceeds.

## Other Resources

* The official Freeputer website: [www.freeputer.net](http://www.freeputer.net)
* The Freeputer [GitHub](https://github.com/RobertGollagher/Freeputer) and [BitBucket](https://bitbucket.org/RobertGollagher/freeputer/src) git repositories
* [Freelang](http://www.concatenative.org/wiki/view/Freelang) at concatenative.org
* [Freelang syntax highlighting](https://bitbucket.org/sts-q/freeputer/src) for Emacs (thanks sts-q)

---

Copyright © Robert Gollagher 2016  

This document was written by Robert Gollagher.  
This document was first published on 11 March 2016.  
This document was last updated on 27 March 2016 at 1704.  
This document is licensed under a [Creative Commons Attribution-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-sa/4.0/).

[![](doc/img/80x15.png)](http://creativecommons.org/licenses/by-sa/4.0/)


The official Freeputer website is [freeputer.net](http://www.freeputer.net).  
Robert Gollagher may be reached at

![](doc/img/abc.png)

---

