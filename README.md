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

To learn Freeputer you should use the provided [Freeputer&nbsp;1.0](archive/1.0) archive (equivalent to the git tag **fpv1.0.0.2**) since that is the version for which the Quick Start tutorial was written. Using Freeputer 1.0 will teach you the basic principles of Freeputer and Freelang while avoiding unnecessary complexity.

See also [Other Resources](#other-resources) below.

## What's the latest news?

Freeputer&nbsp;1.1 is coming!

Expected as an alpha release by early 2017, it should include:

* an FVM 1.1, with **additional instructions**
* continued support for targeting **x86** or **gcc** or **Linux** or **Java**
* new optional support for **Arduino** (ARM) and **chipKIT** (PIC32) boards via the Arduino IDE
* new optional support for an extraordinarily portable textual user-interface (**the [grid](pre-alpha/pre-alpha1.1/grid/)**)
* new optional standard FVM sizes: **Freeputer Lite** and **Freeputer Heavy**  
both with **greater stack depth: 256 elements** instead of 32 elements
* stand-alone Freeputer Lite with PS/2 keyboard and LCD display
* updated Freelang compilers (flc and flx) and decompiler (fld)
* appropriately updated fvmtest test suite

Most of the above has already been successfully spiked. Please note that (due to other professional commitments) the expected release date for the alpha release of Freeputer&nbsp;1.1 has been pushed back further from mid-2016 to early 2017. In the meantime you can use [Freeputer&nbsp;1.0](archive/1.0); it is stable and fully functional.

**Freeputer Lite** (aka **FVM Lite**) will standardize the size of small Freeputer&nbsp;1.1 instances, facilitating the virtualization and reuse of small software modules of standard size. FVM Lite will have 16&nbsp;KiB of FVM RAM, 16&nbsp;KiB of FVM ROM and stack depths of 256 elements each for data stack, software stack and return stack. This is in accordance with the Freeputer design philosophy: *modular not monolithic*. Comfortable hardware minimums are 32&nbsp;KiB RAM and 128&nbsp;KiB flash; at least 64&nbsp;KiB RAM is preferred for FVM virtualization entirely in RAM. Arduino Due and chipKIT Max32 are therefore good choices. Large applications will in future be *composed* of FVM Lite instances.

**Freeputer Heavy** (aka **FVM Heavy**) will standardize the size of large Freeputer&nbsp;1.1 instances. FVM Heavy will have 16&nbsp;MiB of FVM RAM, 16&nbsp;MiB of FVM ROM and stack depths of 256 elements each for data stack, software stack and return stack. However, it is recommended that FVM Heavy should only be used when neither a single FVM Lite instance nor a cluster of FVM Lite instances could reasonably be used.

*You do not have to use the new standard FVM sizes* (FVM Lite, FVM Heavy). Freeputer&nbsp;1.1 will still support using arbitrary FVM sizes, as tiny or as huge as you want, just as Freeputer&nbsp;1.0 does. However, the adoption of these standard FVM sizes is recommended so as to maximize hardware portability, software modularity and software reuse. Remember that Freeputer is designed to be easy to run on bare metal (without any operating system) and to be future-proof such that it will be easy to port Freeputer to a wide variety of future hardware long after current hardware no longer exists. Adopting modest standard FVM sizes greatly facilitates the software-reuse goal: *Write once, run forever.*

Freeputer&nbsp;1.1 is fundamentally about **greater hardware freedom.** That the same small software module can easily be run on a microcontroller or on a powerful server protects your investment in writing that software.

## So where's the code?

The complete Freeputer&nbsp;1.0 code is in the [archive/1.0](archive/1.0) folder (git tag **fpv1.0.0.2**).

The Freeputer&nbsp;1.1 code isn't fully online yet. You can follow its progress in the [pre-alpha1.1](pre-alpha/pre-alpha1.1) folder.

## Other Resources

* The official Freeputer website: [www.freeputer.net](http://www.freeputer.net)
* The Freeputer [GitHub](https://github.com/RobertGollagher/Freeputer) and [BitBucket](https://bitbucket.org/RobertGollagher/freeputer/src) git repositories
* [**Freelang**](http://www.concatenative.org/wiki/view/Freelang), the original Freeputer language, at concatenative.org
* [**Freelang** syntax highlighting](https://bitbucket.org/sts-q/freeputer/src) for Emacs, see `freelang-mode.el` (thanks sts-q)
* [**Rabbit-vm**](https://bitbucket.org/sts-q/freeputer/src), an implementation of the [**Joy**](https://en.wikipedia.org/wiki/Joy_%28programming_language%29) language that runs on Freeputer (thanks sts-q)
* [**Toffee**](https://bitbucket.org/sts-q/toffee/src), a small programming language that runs on Freeputer (thanks sts-q)

---

Copyright © Robert Gollagher 2016  

This document was written by Robert Gollagher.  
This document was first published on 11 March 2016.  
This document was last updated on 28 December 2016 at 00:21.  
This document is licensed under a [Creative Commons Attribution-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-sa/4.0/).

[![](doc/img/80x15.png)](http://creativecommons.org/licenses/by-sa/4.0/)


The official Freeputer website is [freeputer.net](http://www.freeputer.net).  
Robert Gollagher may be reached at

![](doc/img/abc.png)

---

