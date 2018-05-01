<meta http-equiv="content-type" content="text/html;charset=utf-8">

# Freeputer 2.0

Freeputer ( ) \[ \] { } forever free

## Guidelines

1. VM **simpler and easier to implement**.
2. VM **even more portable**.
3. VM **standardized**.
3. VM **improved**.

## Discussion

Although Freeputer&nbsp;2.0 will not run unmodified Freeputer&nbsp;1.0 programs, the fundamental design remains similar: a stack machine which is easy for humans to understand and which favours correctness over robustness. It aims to be simpler and easier to implement so that it is more easily portable.

Freeputer&nbsp;2.0 will first be demonstrated in **JavaScript** so that it runs in popular web **browsers**. Later it may be ported to **WebAssembly** for greater performance. Freeputer&nbsp;2.0 will also continue to target **native x86** (assembly language on **Linux**) as well as **C** (via gcc) and **Java**. There will also be **bare-metal** support for **microcontrollers** via the Arduino IDE. So the same small Freeputer program could run on a powerful server or in the cloud or in a web browser or on a microcontroller. That is true software reuse.

The ability to run Freeputer on bare metal (requiring neither file system nor operating system) will always remain of paramount importance. Future platforms could include other microcontrollers or soft microprocessors implemented as reconfigurable logic in **FPGAs**.

### Reasons for Proposed Design

Although simplistic platforms (such as OISC machines and most MISC machines) may appear attractive at first glance, upon deeper examination they are found not to effectively partition complexity in a manner easily understood by humans and not to be suitable for general-purpose computing on platforms with extremely limited program memory. Therefore such an architecture was rejected for Freeputer&nbsp;2.0. 

Moderately simple register machines (see 'qmisc.c') which lack a stack pointer suffer from the same drawbacks to a degree sufficient to make them impractical. At least one stack pointer is necessary to make any such machine reasonably easy for humans to program, and as soon as a stack pointer has been added to such an architecture then a relatively complex compiler is required for productivity. This is the approach of C machines. That way freedom does not lie. The complexity of such a compiler outweighs the simplicity of the machine itself (which is not so simple anyway once it contains a stack pointer and therefore the attendant runtime complexities of stack operation, including stack overflow and stack underflow, must be addressed). In other words, once you add one stack pointer to an architecture you are better off adding two stack pointers to it and making it a stack machine since a stack machine is much easier for humans to understand and can use a very simple compiler (provided you do not need to run C software). This can be summarized by the following maxim: *Register machines are for compilers. Stack machines are for humans.*

So the fundamental approach of Freeputer&nbsp;1.0 was correct: a stack machine. That way freedom lies. However, although Freeputer&nbsp;1.0 is generally good, the prospect of porting it solo to two new platforms (JavaScript and ARM assembly language) proved sufficiently daunting not to be attempted. This indicated that ideally the virtual machine should be simplified in its 2.0 incarnation so as to be more easily portable. Furthermore, it was concluded that it would be wise to add some new features: to make robustness easier to achieve while maintaining correctness; to facilitate repetition; and to increase bytecode density while simplifying bytecode interpretation. Lastly, it was noted that the unfortunate dependency of the self-hosted Freelang compiler ('flc.fl') on megabytes of RAM when compiling itself from source was highly undesirable with respect to hardware freedom and that it would be better to provide at least one language whose compiler required only kilobytes of RAM to compile itself. For all of these reasons a decision was made to design and create Freeputer&nbsp;2.0 before creating any more software modules for the Freeputer platform.

Considering these matters in depth, it was concluded that the Freeputer&nbsp;1.0 design philosophy was strong in having three stacks rather than two. That is, the so-called software stack (ss) was very convenient and worthwhile in addition to the data stack (ds) and the return stack (rs). Although having three stacks made the machine more complex it made software more efficient and easier for humans to design, create, understand and debug. A large part of this benefit was because there was never any need to push temporary values to the return stack, so the state of the return stack could always be easily understood: it was simply a stack of return addresses. Temporary values were instead pushed to the software stack (ss). Freeputer&nbsp;2.0 will retain this third stack and will rename it the temporary stack (ts). Furthermore, Freeputer&nbsp;2.0 will add a fourth stack dedicated to the purpose of holding loop counters and supported by a repeat (rpt) instruction; this stack shall be called the counter stack (cs). This four-stack design greatly reduces stack-juggling compared to a two-stack design and is more efficient and easier to understand.

Although Freeputer&nbsp;1.0 used von Neumann architecture, currently for Freeputer&nbsp;2.0 a Harvard architecture is proposed. This might seem surprising but has several advantages, including potentially making it easy to compile programs to native code. This does not prevent the dynamic loading and running of programs since ultimately the intention is to virtualize the FVM within the FVM; this isolation might aid security and robustness. The tentative size limits of 2^28 words are due to quirks of C array indexing; perhaps these could be relaxed to a maximum of 2^32 words.

The proposed I/O design is intended to allow the creation of software modules which can be reused forever in an extremely portable manner, long after current hardware and operating systems are no longer available. Therefore simplicity, portability and standardization are incomparably more important than performance. It is also important to understand that Freeputer is primarily intended to be used to perform correct computations (that is, to run reliable algorithms) which are independent of real elapsed time. All matters pertaining to real elapsed time are the responsibility of the *environ* in which Freeputer runs and are largely irrelevant to Freeputer itself. Accordingly, there are no interrupts in Freeputer I/O. Furthermore, all I/O is to and from the data stack and is done by means of dedicated I/O instructions. There is no memory-mapped I/O. This design allows Freeputer to safely and reliably perform correct calculations, largely without interruption and outside interference.

## Proposed Design

- Stack machine with 4 stacks: data (*ds*), temporary (*ts*), return (*rs*), counter (*cs*).
- Harvard architecture ensures ease of native implementation:
    - program logic is entirely independent of instruction encoding;
    - program memory (*pm*) <= 2^24 *instructions* (as reported by the **`pmi`** instruction);
    - data memory (*dm*) <= 2^28 *words* (as reported by the **`dmw`** instruction);
    - rom memory (*rm*) <= 2^28 *words* (as reported by the **`rmw`** instruction).
- Words and stack elements are 32-bit and arithmetic is two's complement.
- Non-native implementations use fixed-width 32-bit instructions (FW32):
    - literals 1:31 (bit 31 *literal bit*, 30..0 *literal value*);
    - other 8:24 (bits 31..24 *opcode*, 23..0 *instruction number*, *metadata*, or unused).
- The VM traps to fail fast and finally. This includes:
    - arithmetic traps (from **`ADD, SUB, INC, DEC, MUL, DIV, MOD`**); and
    - all other traps as seen in FVM 1.0; but
    - without any reset capability.
- However, it is possible to achieve robustness using:
    - the **`CATCH`** instruction which:
        - branches if the previously executed instruction trapped;
        - otherwise functions as a no-op.
    - several convenient metadata instructions which:
        - make it easy to conditionally branch if prerequisites are not met.
- The proposed instruction set includes:  
    1. **`NOP`**
    1. **`CALL`**
    1. **`RET`**
    1. **`RPT`**
    1. **`CPUSH`**
    1. **`CPOP`**
    1. **`CPEEK`**
    1. **`CDROP`**
    1. **`TPUSH`**
    1. **`TPOP`**
    1. **`TPEEK`**
    1. **`TPOKE`**
    1. **`TDROP`**
    1. **`LIT`**
    1. **`DROP`**
    1. **`SWAP`**
    1. **`OVER`**
    1. **`ROT`**
    1. **`DUP`**
    1. **`GET`**
    1. **`PUT`**
    1. **`GETI`**
    1. **`PUTI`**
    1. **`ROM`**
    1. **`ADD`**
    1. **`SUB`**
    1. **`MUL`**
    1. **`DIV`**
    1. **`MOD`**
    1. **`INC`**
    1. **`DEC`**
    1. **`OR`**
    1. **`AND`**
    1. **`XOR`**
    1. **`FLIP`**
    1. **`NEG`**
    1. **`SHL`**
    1. **`SHR`**
    1. **`HOLD`**
    1. **`GIVE`**
    1. **`IN`**
    1. **`OUT`**
    1. **`JUMP`**
    1. **`JMPZ`**
    1. **`JMPE`**
    1. **`JMPG`**
    1. **`JMPL`**
    1. **`HALT`**
    1. **`FAIL`**
    1. **`CATCH`**
    1. **`DSA`**
    1. **`DSE`**
    1. **`TSA`**
    1. **`TSE`**
    1. **`CSA`**
    1. **`CSE`**
    1. **`RSA`**
    1. **`RSE`**
    1. **`PMI`**
    1. **`DMW`**
    1. **`RMW`**
    1. **`HW`**
    1. **`TRON`**
    1. **`TROFF`**
- The I/O strategy is simple, portable and standardized:
    - the outside world is known as the *environ*;
    - there are only three I/O devices:
        1. `stdin` (input stream):
            - reads a byte to the *ds*, blocking, branches on failure;
            - public, connected to the *environ*.
        2. `stdout` (output stream):
            - writes a byte from the *ds*, blocking, branches on failure;
            - public, connected to the *environ*.
        3. `stdhold` (word-indexed persistent local storage):
            - known as *standard hold* or simply the *hold*;
            - capacity 0 to 2^28 *words* (as reported by the **`hw`** instruction);
            - gets a word to the *ds*, blocking, branches on failure;
            - puts a word from the *ds*, blocking, branches on failure;
            - private, not connected to the *environ*.
    - the *environ* determines the exact behaviour of `stdin` and `stdout`;
    - the *environ* determines the exact protocol it uses for communication;
    - such a protocol is then explicitly used by suitable Freeputer programs;
    - all inputs (such as from keyboards, pipes, networks) are mediated by the *environ* via `stdin`;
    - all outputs (such as to displays, pipes, networks) are mediated by the *environ* via `stdout`;
    - in effect, `stdin` and `stdout` convey a *conversation* with messages and questions;
    - otherwise, there are three possible *immediate signals* from the *environ*:
        1. `start`: gracefully starts a Freeputer instance;
        2. `pause`: gracefully pauses a Freeputer instance;
        3. `kill`: kills a Freeputer instance.
    - some consideration may also be given to allowing **`IN`** and **`OUT`** by word;
    - tracing may well be replaced by debugging.
- The design might possibly include a ground state such as:
    - a REPL fallback after a trap ends execution; or
    - a hypervisor.


---

Copyright © Robert Gollagher 2017, 2018  

This document was written by Robert Gollagher.  
This document was created on 3 March 2017.  
This document was last updated on 1 May 2018 at 16:04  
This document is licensed under a [Creative Commons Attribution-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-sa/4.0/).

[![](doc/img/80x15.png)](http://creativecommons.org/licenses/by-sa/4.0/)


The official Freeputer website is [freeputer.net](http://www.freeputer.net).  
Robert Gollagher may be reached at

![](doc/img/abc.png)

---

