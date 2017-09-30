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

Freeputer&nbsp;2.0 is intended to be ***easier to implement*** and thus ***even more portable***. To demonstrate this, the prototype for Freeputer&nbsp;2.0 is now being developed in JavaScript and HTML 5, which for the first time should make it easy to run Freeputer in popular web browsers available ***on billions of consumer devices***. Freeputer&nbsp;2.0 will continue to support targeting x86, C, Linux, and Java. There will also be new bare-metal support for targeting Arduino (ARM) and chipKIT (PIC32) boards via the Arduino IDE. So the same small Freeputer program could run on a ***powerful server*** or in a ***web browser*** or on a ***microcontroller***.

Freeputer&nbsp;2.0 ***adds robustness***. Whereas Freeputer&nbsp;1.0 trapped (stopping the virtual machine) to preserve *correctness*, the design of Freeputer&nbsp;2.0 is likely to be more robust by *branching on failure*.

Freeputer&nbsp;2.0 may also have a different architecture and I/O strategy.

The motto of Freeputer&nbsp;2.0 is: ***smaller simpler better***.

## Migration

Freeputer&nbsp;2.0 will not run unmodified Freeputer&nbsp;1.0 programs.

However, Freeputer&nbsp;1.0 is an open-source platform, and free as in freedom, so you are welcome to keep using it forever (according to the provisions of the GPL-3.0+) if you prefer it to Freeputer&nbsp;2.0. Furthermore, it may well be possible to create an FVM 1.0 implementation which runs on FVM 2.0 (as a virtual machine within a virtual machine) which would allow Freeputer&nbsp;1.0 programs to run there.

## Shortlist

A final decision has been made (on 1 October 2017) that Freeputer&nbsp;2.0 will be a stack machine not a register machine. It will probably either look like Plan A (see below) or a simplified stack machine. It is noteworthy that extensive thought and experimentation over many weeks has led to the conclusion that: Register machines are for compilers. Stack machines are for humans. Therefore freedom most easily lies in the direction of stack machines not register machines. That is, Freeputer&nbsp;1.0 was on the right track.

### Proposed Design: Plan A: Improved Stack Machine

Plan A is a portable stack machine which is correct and robust by virtue of branching on failure.

This plan is moderately difficult to implement but makes factoring easy.

Performance is at least moderate. An excellent stack machine.

Implementation is on hold while Plan G is explored.

The detailed design is:

1. The VM is **correct without trapping**.
1. Program execution begins at cell 1 (not cell 0).
1. Its termination results in **success or failure** (0 or 1).
1. The only cause of VM success is:
    1. The halt opcode (0xff).
1. The only causes of VM failure are:
    1. The fail opcode (0x00);
    1. Platform failure.
1. All opcodes **branch on failure** except: halt (0xff), fail (0x00) and branches.
1. The following opcodes cause immediate VM termination regardless of any *metadata*:
    1. The halt opcode (0xff);
    1. The fail opcode (0x00).
1. A program can be an infinite loop which never terminates.
1. Failure is typically rare and means the run **cannot be trusted**.
1. For most opcodes, **branch on failure is equivalent to a noop branching on failure**.
1. All illegal opcodes are always treated as **a noop branching on failure**.
1. Addressing is **absolute** and **word-indexed** (1 cell = 1 word).
1. Common sense applies but generally speaking the default value of cells is **zero**.
1. *Address space* is **32 signed bits** (the 16 GiB from word -2147483648 to word 2147483647).
1. *Address space* consists of 5 *zones* (regions of contiguous cells): VOL, SYS, PRG, MEM and BLK. 
1. *Program space* (the PRG *zone*) is **24 unsigned bits** (the 64 MiB from word 0 to word 16777215).
1. Program **execution loops** back to the start of PRG when it reaches the end of PRG.
1. The VM can be implemented on **powerful servers** using physical memory.
1. The VM can be implemented on **small microcontrollers** using mainly logical memory.
1. Words are **32 signed bits** (little endian, two's complement).
1. All instructions are **1 cell wide** (that is, 32 bits wide) but a few use data from the following cell.
1. Thus instructions are **simple**, **symmetrical**, **compact** and **small-device friendly**.
1. The least-significant 8 bits of an instruction comprise its *opcode*.
1. The most-significant 24 bits of an instruction comprise its *metadata*.
1. For instructions which cannot fail the *metadata* is an *address* (in the case of *branch instructions*) or is ignored.
1. Most instructions can fail. For instructions which can fail, the *metadata* is a *failure address*.
1. *Literal instructions* and *call instructions* can fail. They are unusual in that they use data from the following cell:
    - the following cell is skipped upon success and contains the address or literal used by the instruction.
1. The VM has **3 stacks** of words: a data stack (ds), a software stack (ss) and a return stack (rs).
1. Each stack has a maximum depth of **256 elements**.
1. Inability to call a subroutine (call failure due to rs full) triggers **branch on failure**.
1. Inability to return from a subroutine (return failure due to rs empty) triggers **branch on failure**.
1. The fret opcode retrospectively triggers **branch on failure** of the currently executing subroutine call instruction.
1. The fret opcode itself undergoes **branch on failure** if that is not possible (fret failure due to rs empty or no call instruction).
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
    - **MEM** is the 4032 MiB from cell 16777216 (`0x1000000`) to cell 1073741823 (`0x3fffffff`).
        - MEM is for RAM.
        - MEM must entirely consist of one of the following:
            - *unplumbed cells*; or
            - *read/write cells* (RAM); or
            - *read/write cells* (RAM) followed by *unplumbed cells*.
        - If MEM is an *unplumbed zone* then it requires no physical RAM at all.
        - Note: a VM may provide additional RAM via VOL.
    - **BLK** is the 4 GiB from cell 1073741824 (`0x40000000`) to cell 2147483647 (`0x7fffffff`).
        - BLK is for persistent storage (retained between runtimes).
        - BLK must entirely consist of one of the following:
            - *unplumbed cells*; or
            - *read/write cells* (persistent storage); or
            - *read/write cells* (persistent storage) followed by *unplumbed cells*.
        - If BLK is an *unplumbed zone* then it requires no physical persistent storage at all.
        - Note: a VM may provide additional persistent storage via VOL.
1. **All zones which end below cell 0 are volatile zones:**
    - **SYS** is the 64 MiB from cell -1 (`0xffffffff`) to cell -16777216 (`0xff000000`).
        - SYS is reserved for system use. It provides standard system services to the VM:
            - the optional log stream **`stdlog`** as the *volatile write-only cell* -2 (`0xfffffffe`).
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
    - **VOL** is the 8128 MiB from cell -16777217 (`0xfeffffff`) to cell -2147483648 (`0x80000000`).
        - The default implementation of VOL is as an *unplumbed zone* which does nothing and requires no resources.
        - VM implementors may add custom functionality within VOL in a reasonable and modular manner.
        - VOL is a *volatile zone* in which memory-mapped devices could be added to:
            - extend the connectivity of the VM; and/or
            - extend the I/O capabilities of the VM; and/or
            - extend the storage capabilities of the VM; and/or
            - extend the functionality of the VM; and all
            - to a virtually limitless extent.
1. **Data streams:**
    - The **`stdin`** and **`stdout`** streams, if available, are intended for use as data streams (not for user interaction).
    - Using **`stdin`** and **`stdout`** as data streams allows VM instances to be chained together as a processing pipeline.
    - Using **`stdin`** and **`stdout`** as data streams allows modular systems to be created by composition of VM instances.
    - The nature, behaviour and effect of the **`stdin`** and **`stdout`** streams may vary from environment to environment.
    - The log stream **`stdlog`**, if available, allows Freeputer programs to log information when logging is enabled.
    - The trace stream **`stdtrc`**, if available, outputs accurate trace information when tracing is enabled.
    - The information provided by **`stdtrc`** may reasonably differ between VM implementations.
    - There is no means for a Freeputer program to itself write to the **`stdtrc`** stream.
    - Note: a VM may provide additional data streams via VOL.
1. **User interfaces:**
    - The **`usrin`** and **`usrout`** streams, if available, must represent some kind of user interface.
    - The user interface represented by **`usrin`** and **`usrout`** can be highly sophisticated and customized.
    - The nature, behaviour and effect of the **`usrin`** and **`usrout`** streams may vary from environment to environment.
    - The **`grdin`** and **`grdout`** streams, if available, represent the standard textual user interface known as the grid.
    - The logical behaviour of the **`grdin`** and **`grdout`** streams does not vary between environments.
    - It is best practice never to use **`stdin`** and **`stdout`** to interact with the user.
    - Note: a VM may provide additional user interfaces via VOL.
1. **If the VM is running as an process:**
      - The **`stdin`** and **`stdout`** streams, if available:
          - *may be connected to stdin and stdout of the VM process*; or
          - *may instead reasonably be connected to named pipes or other conduits.*
      - The **`usrin`** and **`usrout`** streams, if available:
          - *may directly drive a custom user interface bundled in the VM process*; or
          - *may (instead of **`stdin`** and **`stdout`**) be interactively connected to stdin and stdout of the VM process*; or
          - *may reasonably be connected to a custom user interface via named pipes or other conduits.*
      - The **`grdin`** and **`grdout`** streams, if available:
          - *may directly drive a grid implementation bundled in the VM process*; or
          - *may reasonably be connected to a grid via named pipes or other conduits.*
      - It is best practice to use a grid (via **`grdin`** and **`grdout`**) for all command-line interaction.
          - In that case, **`stdin`** and **`stdout`** may simply be connected to stdin and stdout of the VM process.
      - Less desirable is terminal interaction via **`usrin`** and **`usrout`** connected to stdin and stdout of the VM process.
          - In that case, **`stdin`** and **`stdout`** would reasonably be connected to named pipes or other conduits.
      - Terminal interaction should not be done via **`stdin`** and **`stdout`** directly.
          - This is because **`stdin`** and **`stdout`** are intended for use as data streams.
      - Note: the VM design requires neither file system nor operating system.
1. **Examples of VM sizings:**
    - For a small microcontroller (to run a small program not using BLK):
        - 1 KiB of RAM in MEM; no BLK; PRG entirely ROM (using very little physical ROM):
            - `RAMa` 16777216, `RAMz` 16777471, `BLKz` -1, `VOLz` -1
    - For a powerful server (to run that same program or a much larger program):
        - 4 GiB of RAM (the whole of PRG and MEM); 4 GiB of BLK:
            - `RAMa` 0, `RAMz` 1073741823, `BLKz` 2147483647, `VOLz` -1
1. **Load and store is wordwise:**
    - The only load and store instructions are @ and !
    - That is, all loads and all stores are nominally of whole words not individual bytes.
    - To compensate for this, fast 'bytewise' logical operators facilitate byte manipulation.
1. **Some I/O is bytewise:**
    - Some volatile cells are dedicated to byte transfer of their least significant byte.
    - These always include the SYS cells for **`stdlog`**, **`stdin`**, **`stdout`**, **`grdin`**, **`grdout`**, **`usrin`** and **`usrout`**.
    - The other bytes of such cells are unaffected by @ and ! and are always zero.
1. **Stack Assertions:**
    - There is a minimal set of fast stack-assertion instructions that branch on failure if:
        - there is not a minimum of free space (1,2,3 or 4 cells) available on the data stack;
        - there is not a minimum amount of data (1,2,3 or 4 elements) present on the data stack.
    - This minimal set of stack assertions exists to:
        - allow programs to quickly deduce the cause of subsequent instruction failures;
        - allow programs to quickly branch when stack state is inappropriate.
    - This minimal set is logically sufficient.
1. **Plumbed Assertions:**
    - There is a minimal set of safe plumbed-assertion instructions that branch on failure if:
        - a cell will certainly never (during this runtime) support the @ instruction;
        - a cell will certainly never (during this runtime) support the ! instruction.
    - These plumbed assertions cause no side-effects whatsoever.
1. **Instruction set:**
    - The instruction set for Freeputer 2.0:
        - has not yet been *fully* decided upon;
        - will be broadly similar to that of Freeputer 1.0;
        - is likely to omit several very CISC-like instructions;
        - is certain to have no complex (2-word) instructions.
    - The prototype will use an experimental subset of proposed instructions.
    - The prototype will at first have very few instructions as its implementation gradually proceeds.


---

Copyright © Robert Gollagher 2017  

This document was written by Robert Gollagher.  
This document was created on 3 March 2017.  
This document was last updated on 1 October 2017 at 00:54  
This document is licensed under a [Creative Commons Attribution-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-sa/4.0/).

[![](doc/img/80x15.png)](http://creativecommons.org/licenses/by-sa/4.0/)


The official Freeputer website is [freeputer.net](http://www.freeputer.net).  
Robert Gollagher may be reached at

![](doc/img/abc.png)

---

