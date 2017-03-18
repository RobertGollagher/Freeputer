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

Freeputer&nbsp;2.0 will be ***much easier to implement***, making it even more portable. To demonstrate this, the prototype for Freeputer&nbsp;2.0 is now being developed in JavaScript and HTML 5, which for the first time should make it easy to run Freeputer in popular web browsers available ***on billions of consumer devices***.

Freeputer&nbsp;2.0 will continue to support targeting x86, C, Linux, and Java. There will also be new bare-metal support for targeting Arduino (ARM) and chipKIT (PIC32) boards via the Arduino IDE. So the same small Freeputer program could run on a powerful server or in a web browser or on a microcontroller!

Freeputer&nbsp;2.0 ***adds excellent robustness***. Whereas Freeputer&nbsp;1.0 trapped (stopping the virtual machine) to preserve its excellent correctness, the design of Freeputer&nbsp;2.0 is more robust in that it keeps running while maintaining excellent correctness. It achieves this by *branching on failure* rather than *trapping on failure*.

Freeputer&nbsp;2.0 has a larger *address space* but a smaller *program space* than Freeputer&nbsp;1.0. Having a smaller *program space* allows the achievement of excellent correctness and excellent robustness *at the same time*, in a manner which is portable *and* simpler and easier to implement. This is why the motto of Freeputer&nbsp;2.0 is: ***smaller simpler better***.

## Migration

Freeputer&nbsp;1.0 and 2.0 are ***quite similar but not binary compatible***. The instruction set and its bytecode has been changed somewhat. However, existing Freelang&nbsp;1.0 programs could probably be modified and recompiled as Freelang 2.0 programs for Freeputer&nbsp;2.0 ***without great difficulty***.

## Proposed Design

1. The VM is **correct without trapping**.
1. Its termination results in **success or failure** (0 or 1).
1. All instructions and all stack operations **branch on failure**.
1. Each element on the return stack records **subroutine success or failure**.
1. Inability to call a subroutine is treated as **subroutine failure**.
1. An illegal instruction causes **subroutine failure**.
1. A naked illegal instruction causes **VM failure**.
1. Addressing is **absolute** and **word-indexed** (1 cell = 1 word).
1. *Address space* is **32 signed bits** (the 16 GiB from word -2147483648 to word 2147483647).
1. *Address space* consists of 5 *zones* (regions of contiguous cells): VOL, SYS, PRG, MEM and BLK. 
1. *Program space* (the PRG *zone*) is **24 unsigned bits** (the 64 MiB from word 0 to word 16777215).
1. The VM can be implemented on **powerful servers** using physical memory.
1. The VM can be implemented on **small microcontrollers** using mainly logical memory.
1. Words are **32 signed bits** (little endian, two's complement).
1. **Simple instructions** are 1 word: an unsigned 24-bit *failure address* above an unsigned 8-bit *opcode*.
1. **Complex instructions** also have a second word: a signed 32-bit *literal*.
1. The VM has **3 stacks**: a data stack (ds) of words, a software stack (ss) of words, and a return stack (rs).
1. Each rs element contains unsigned 8-bit *metadata* above an unsigned 24-bit *return address*.
1. Each stack has a maximum depth of **256 elements**.
1. All I/O is **non-blocking** and unavailable I/O triggers **branch on failure**.
1. All I/O is **memory mapped** and unsupported I/O triggers **branch on failure**.
1. A *read-only cell* is one which only supports the @ instruction.
1. A *write-only cell* is one which only supports the ! instruction.
1. A *read/write cell* is one which supports both the @ and ! instructions.
1. A *plumbed cell* is one which supports at least one of the @ or ! instructions.
1. A *fully plumbed zone* is one containing only *plumbed cells*. PRG is always a *fully plumbed zone*.
1. An *unplumbed cell* is one which supports neither the @ instruction nor the ! instruction.
1. A *partially plumbed zone* is one containing *plumbed cells* and *unplumbed cells*.
1. An *unplumbed zone* is one in which all cells are *unplumbed*.
1. A *volatile cell* is either a *write-only cell* or a cell whose value can change spontaneously at runtime.
1. A *faithful cell* faithfully holds stored data; its value cannot change spontaneously at runtime.
1. A *faithful zone* is one containing no *volatile cells*.
1. A *faithful zone* neither causes nor is affected by side-effects at runtime.
1. A *volatile zone* is any zone which contains at least one *volatile cell*.
1. Any use of the @ or ! instructions in a *volatile zone* may cause side-effects:
    - directly in the *volatile zone* itself; and
    - in any external systems with which it communicates; and
    - indirectly in any *volatile zone* via external systems.
1. The nature of any such side-effects is determined by:
    - the nature of the *volatile zone* itself; and
    - the nature of any relevant external systems; and
    - the nature of any *volatile zones* with which relevant external systems communicate.
1. **All zones which begin at or above cell 0 are faithful zones:**
    - **PRG** is the 64 MiB from cell 0 to cell 16777215 (`0xffffff`).
        - PRG is for *program space*.
        - PRG is always a *fully plumbed zone*.
        - PRG must entirely consist of one of the following:
            - *read-only cells* (ROM); or
            - *read/write cells* (RAM); or
            - *read-only cells* followed by *read/write cells* (ROM then RAM).
        - For a large application, PRG may require megabytes of physical memory.
        - For a small application, PRG may require less than a kilobyte of physical memory.
    - **MEM** is the 960 MiB from cell 16777216 (`0x1000000`) to cell 1073741823 (`0x3fffffff`).
        - MEM is for RAM.
        - MEM must entirely consist of one of the following:
            - *unplumbed cells*; or
            - *read/write cells* (RAM); or
            - *read/write cells* (RAM) followed by *unplumbed cells*.
        - If MEM is an *unplumbed zone* then it requires no physical RAM at all.
        - Note: a VM may provide access to additional RAM via VOL.
    - **BLK** is the 1024 MiB from cell 1073741824 (`0x40000000`) to cell 2147483647 (`0x7fffffff`).
        - BLK is for persistent storage (retained between runtimes).
        - BLK must entirely consist of one of the following:
            - *unplumbed cells*; or
            - *read/write cells* (persistent storage); or
            - *read/write cells* (persistent storage) followed by *unplumbed cells*.
        - If BLK is an *unplumbed zone* then it requires no physical persistent storage at all.
        - Note: a VM may provide access to additional persistent storage via VOL.
1. **All zones which end below cell 0 are volatile zones:**
    - **SYS** is the 64 MiB from cell -1 (`0xffffffff`) to cell -16777216 (`0xff000000`).
        - SYS is reserved for system use. It provides standard system services to the VM:
            - the optional trace stream **`stdtrc`** as the *volatile write-only cell* -2 (`0xfffffffe`).
            - the optional data streams **`stdin`** and **`stdout`** as the *volatile read/write cell* -4 (`0xfffffffc`);
            - the optional standard UI streams **`grdin`** and **`grdout`** as the *volatile read/write cell* -6 (`0xfffffffa`).
            - the optional non-standard UI streams **`usrin`** and **`usrout`** as the *volatile read/write cell* -8 (`0xfffffff8`).
            - the ability to query the 4 values which define *faithful VM size*:
                1. the **address of the first cell of RAM** (called `RAMa`):
                    - this will be between 0 and 16777215 if PRG contains RAM; otherwise
                    - this will be 16777216 if MEM contains RAM; otherwise
                    - this will be -1.
                2. the **address of the last cell of RAM** (called `RAMz`):
                    - this will be between 16777216 and 1073741823 if MEM contains RAM; otherwise
                    - this will be between 0 and 16777215 if PRG contains RAM; otherwise
                    - this will be -1.
                3. the **address of the last *plumbed cell* of BLK** (called `BLKz`):
                    - this will be -1 if BLK is an *unplumbed zone*; otherwise
                    - this will be between 1073741824 and 2147483647.
                4. the **address of the last *plumbed cell* of VOL** (called `VOLz`):
                    - this will be -1 if VOL is an *unplumbed zone*; otherwise
                    - this will be between -16777217 and -2147483648.
        - SYS is largely *unplumbed* and for itself requires very little physical memory.
        - SYS contains the cell -1 (`0xffffffff`) which is always *unplumbed*.
    - **VOL** is the 1984 MiB from cell -16777217 (`0xfeffffff`) to cell -2147483648 (`0x80000000`).
        - The default implementation of VOL is as an *unplumbed zone* which does nothing and requires no resources.
        - VM implementors may add custom functionality within VOL in a reasonable and modular manner.
        - VOL is a *volatile zone* in which memory-mapped devices could be added to:
            - extend the connectivity of the VM; and/or
            - extend the I/O capabilities of the VM; and/or
            - extend the storage capabilities of the VM; and/or
            - extend the functionality of the VM; and all
            - to a virtually limitless extent.
1. **Data streams:**
    - The **`stdin`** and **`stdout`** streams, if available, are intended for use as data streams not user-interface streams.
    - Using **`stdin`** and **`stdout`** as data streams allows VM instances to be chained together as a processing pipeline.
    - The nature, behaviour and effect of the **`stdin`** and **`stdout`** streams may vary from environment to environment.
    - The trace stream **`stdtrc`**, if available, provides accurate trace information when tracing is enabled.
    - The information provided by **`stdtrc`** may reasonably differ between VM implementations.
    - Note: a VM may provide access to additional data streams via VOL.
1. **User interfaces:**
    - The **`usrin`** and **`usrout`** streams, if available, may represent any user interface whatsoever.
    - The nature, behaviour and effect of the **`usrin`** and **`usrout`** streams may vary from environment to environment.
    - The **`grdin`** and **`grdout`** streams, if available, represent the standard textual user interface known as the grid.
    - The logical behaviour of the **`grdin`** and **`grdout`** streams is identical in all environments.
    - Note: a VM may provide access to additional user interfaces via VOL.
1. **Examples of VM sizings:**
    - For a small microcontroller (to run a small program not using BLK):
        - 1 KiB of RAM in MEM; no BLK; PRG entirely ROM (using very little physical ROM):
            - `RAMa` 16777216, `RAMz` 16778239, `BLKz` -1, `VOLz` -1
    - For a powerful server (to run that same program or a much larger program):
        - 1 GiB of RAM (the whole of PRG and MEM); 1 GiB of BLK:
            - `RAMa` 0, `RAMz` 1073741823, `BLKz` 2147483647, `VOLz` -1


---

Copyright © Robert Gollagher 2017  

This document was written by Robert Gollagher.  
This document was created on 3 March 2017.  
This document was last updated on 18 March 2017 at 20:21  
This document is licensed under a [Creative Commons Attribution-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-sa/4.0/).

[![](doc/img/80x15.png)](http://creativecommons.org/licenses/by-sa/4.0/)


The official Freeputer website is [freeputer.net](http://www.freeputer.net).  
Robert Gollagher may be reached at

![](doc/img/abc.png)

---

