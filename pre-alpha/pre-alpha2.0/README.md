<meta http-equiv="content-type" content="text/html;charset=utf-8">

# Freeputer&nbsp;2.0

Freeputer ( ) \[ \] { } smaller simpler better

## Values

1. VM **simpler and easier to implement**.
2. VM **even more portable**.
3. VM **entirely correct**.
4. VM **standardized**.
5. VM **smaller**.
6. VM **faster**.

## Discussion

Although Freeputer 2.0 will not run unmodified Freeputer 1.0 programs, the fundamental design remains similar: a stack machine which is easy for human beings to understand and which favours correctness over robustness. The architecture enforces correctness by trapping. Robustness is the responsibility of the programmer and can be aided by self-virtualization of virtual machine instances.

Freeputer 2.0 will be implemented in JavaScript, which for the first time should make it easy to run Freeputer in popular web browsers available on billions of consumer devices. Furthermore, Freeputer 2.0 will continue to support targeting x86, C, Linux, and Java. There will also be new bare-metal support for targeting Arduino (ARM) and chipKIT (PIC32) boards via the Arduino IDE. So the same small Freeputer program could run on a powerful server or in a web browser or on a microcontroller. This is true software reuse.

### Proposed Design

Experiments are in progress to radically simplify the design (see 'fvm2.js' and 'prg.js').

Therefore the most recently proposed design, shown below, is no longer current.

- Stack machine with 4 stacks: data (ds), temporary (ts), return (rs), counter (cs).
- Harvard architecture ensures ease of native implementation:
    - program logic is entirely independent of instruction encoding;
    - program memory (pm) <= 2^24 *instructions* (as reported by the pmi instruction);
    - data memory (dm) <= 2^30 *words* (as reported by the dmw instruction).
- Words and stack elements are 32-bit and arithmetic is two's complement.
- Non-native implementations use fixed-width 32-bit instructions (FW32):
    - literals 1:31 (bit 31 *literal bit*, 30..0 *literal value*);
    - other 8:24 (bits 31..24 *opcode*, 23..0 *instruction number*, *metadata*, or unused).
- The VM traps to fail fast and finally. This includes:
    - arithmetic traps (from add, sub, inc, dec, mul, div, mod); and
    - all other traps as seen in FVM 1.0; but
    - without any reset capability.
- However, it is possible to achieve robustness using:
    - the catch instruction which:
        - branches if the previously executed instruction trapped;
        - otherwise functions as a no-op.
    - several convenient metadata instructions which:
        - make it easy to conditionally branch if prerequisites are not met;
        - include the pmi, dmw, dsa, dse, tsa, tse, rsa, rse, csa and cse instructions.
- The instruction set will include:
    - significantly fewer instructions than FVM 1.0;
    - no instructions prematurely optimizing performance;
    - a sufficiently large set of convenient RISC instructions;
    - no CISC instructions (instead there will be RISC instructions plus rpt);
    - the instructions: halt, fail.
- The I/O strategy is profoundly simple:
    - the only I/O instructions are in, out; and
    - these branch on failure but usually succeed; and
    - they simply move 1 word to/from the data stack; and
    - there is only 1 input stream (stdin) for the in instruction; and
    - there is only 1 output stream (stdout) for the out instruction; and
    - it is the responsibility of the *environ* to make these streams intelligent; and
    - there is no limit to how intelligent an *environ* and these streams can be; and
    - the protocol an *environ* uses is explicit in a program written for it; and
    - correct outcomes require deployment to an appropriate *environ*.
- The design might possibly include a ground state such as:
    - a REPL fallback after a trap ends execution; or
    - a hypervisor.
- Support for tracing is optional.


---

Copyright © Robert Gollagher 2017, 2018  

This document was written by Robert Gollagher.  
This document was created on 3 March 2017.  
This document was last updated on 11 April 2018 at 00:48  
This document is licensed under a [Creative Commons Attribution-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-sa/4.0/).

[![](doc/img/80x15.png)](http://creativecommons.org/licenses/by-sa/4.0/)


The official Freeputer website is [freeputer.net](http://www.freeputer.net).  
Robert Gollagher may be reached at

![](doc/img/abc.png)

---

