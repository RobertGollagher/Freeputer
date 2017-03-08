<meta http-equiv="content-type" content="text/html;charset=utf-8">

# Freeputer&nbsp;2.0

Freeputer ( ) \[ \] { } smaller simpler better

## Values

1. VM **simple and easy to implement**.
2. VM exceedingly **portable**.
3. VM entirely **correct**.
4. VM largely **robust**.
5. VM **standard**.
6. VM **small**.

## Benefits

Freeputer&nbsp;2.0 will be ***much easier to implement***, making it even more portable. To demonstrate this, the prototype for Freeputer&nbsp;2.0 is now being developed in JavaScript and HTML 5, which should make it easy to run Freeputer in popular web browsers available ***on billions of consumer devices***.

Freeputer&nbsp;2.0 will continue to support targeting x86, gcc, Linux, and Java. There will also be new bare-metal support for targeting Arduino (ARM) and chipKIT (PIC32) boards via the Arduino IDE.

Freeputer&nbsp;2.0 ***adds excellent robustness***. Whereas Freeputer&nbsp;1.0 trapped (stopping the virtual machine) to preserve its excellent correctness, the design of Freeputer&nbsp;2.0 is more robust in that it keeps running while maintaining excellent correctness. It achieves this by *branching on failure* rather than *trapping on failure*.

Freeputer&nbsp;2.0 has a smaller standard address space than Freeputer&nbsp;1.0. Keeping it small achieves excellent correctness and excellent robustness *at the same time*, in a manner which is portable *and* simpler and easier to implement. This is why the motto of Freeputer&nbsp;2.0 is: ***smaller simpler better***.

## Migration

Freeputer&nbsp;1.0 and 2.0 are ***quite similar but not binary compatible***. The instruction set and its bytecode has been changed somewhat. However, existing Freelang&nbsp;1.0 programs could probably be modified and recompiled as Freelang 2.0 programs for Freeputer&nbsp;2.0 ***without great difficulty***.

## Design

1. The VM is **correct without trapping**.
1. Its termination results in **success or failure** (0 or 1).
1. All instructions and all stack operations **branch on failure**.
1. Each element on the return stack records **subroutine success or failure**.
1. Inability to call a subroutine is treated as **subroutine failure**.
1. An illegal instruction causes **subroutine failure**.
1. A naked illegal instruction causes **VM failure**.
1. Addressing is **absolute** and **word-indexed** (1 cell = 1 word).
1. Address space is **256 banks of 64 MiB** (16777216 cells) each.
1. Program space is **24 unsigned bits** (the entire 64 MiB of bank 0).
1. Words are **32 signed bits** (little endian, two's complement).
1. **Simple instructions** are 1 word: an unsigned 24-bit *failure address* above an unsigned 8-bit *opcode*.
1. **Complex instructions** also have a second word: a signed 32-bit *literal*.
1. The VM has **3 stacks**: a data stack (ds) of words, a software stack (ss) of words, and a return stack (rs).
1. Each rs element contains unsigned 8-bit *metadata* above an unsigned 24-bit *return address*.
1. All I/O is **memory mapped** and unsupported I/O triggers **branch on failure**.
1. A *read-only* cell is one which only supports the @ instruction.
1. A *write-only* cell is one which only supports the ! instruction.
1. A *read/write* cell is one which supports both the @ and ! instructions.
1. An *unplumbed* cell is one which supports neither the @ instruction nor the ! instruction.
1. A *volatile* cell is one whose value can change spontaneously at runtime.
1. A *faithful* cell is one whose value cannot change spontaneously at runtime.
1. A *volatile* bank is one containing at least one volatile cell.
1. A *faithful* bank (such as bank 0) is one containing no volatile cells.
1. A block device is represented by a *faithful* bank.
1. Streams reside in a *volatile* bank.
1. All cells of bank 0 are *faithful* and either *read-only* or *read/write*.
1. All or most cells of other banks may well be *unplumbed*.


---

Copyright © Robert Gollagher 2017  

This document was written by Robert Gollagher.  
This document was created on 3 March 2017.  
This document was last updated on 8 March 2017 at 13:11.  
This document is licensed under a [Creative Commons Attribution-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-sa/4.0/).

[![](doc/img/80x15.png)](http://creativecommons.org/licenses/by-sa/4.0/)


The official Freeputer website is [freeputer.net](http://www.freeputer.net).  
Robert Gollagher may be reached at

![](doc/img/abc.png)

---

