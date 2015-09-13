/*
                        FREEPUTER VIRTUAL MACHINE

Program:    fvm
Copyright © Robert Gollagher 2015
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20150329
Updated:    20150913:1413
Version:    1.0.0.2 for FVM 1.0

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
[at your option] any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

                              This Edition:
                          x86 Assembly Language
                           using GNU Assembler
                            for 32-bit Linux

                               ( ) [ ] { }


============================================================================*/
#                         ASSEMBLER DIRECTIVES
# ============================================================================
.equiv DEFINED, 0 # Arbitrary value for defined symbols here

# For faster performance comment out the next line. The assembled FVM will
# then not check (when it executes each instruction in a program) whether or
# not it should produce trace output. The improvement in performance is
# typically very significant. The drawback is that TRON cannot then be
# usefully used to debug programs since it will have no effect.
.equiv TRON_ENABLED, DEFINED

# ============================================================================
.section .data #                CONSTANTS
# ============================================================================

  # Version stamp for FVM binary
  version: .asciz "fvm x86-asm-linux-32 version 1.0.0.2 for FVM 1.0"

  # The following example VM sizings are arbitrary.
  # You can have a much larger or much smaller VM instance as desired.
  # However, maximum address space (in which RAM, ROM and MAP must fit)
  # is 2147483648 bytes. Maximum STDBLK_SIZE is 2147483648 bytes.
  # This would of course be addressed as 0 to 2147483647.

  # --------------------------------------------------------------------------
  # This section specifies a medium-sized VM instance ('fvm16-16MB') having:
  #   16 MB RAM, 16 MB ROM, 16 MB stdblk, no memory-mapped devices.
  # Suitable for:
  #   * Running the Freelang compiler; or
  #   * Running the FreeLine text editor; or
  #   * Running other programs having moderate requirements.
  # Has:
  #   ARBITRARY_MEMORY_SIZE = 16777216 bytes
  #   STDBLK_SIZE = 16777216 bytes
  # Uncomment the next four lines to use this size of VM instance:
  # .equ ARBITRARY_MEMORY_SIZE,           0b00000001000000000000000000000000
  # .equ PROG_MEMORY_MASK,                0b11111110000000000000000000000000
  # .equ STDBLK_SIZE,                     0b00000001000000000000000000000000
  # .equ STDBLK_MEMORY_MASK,              0b11111110000000000000000000000000
  # Create std.blk file at Linux command line by:
  #   head -c 16777216 /dev/zero > std.blk
  # --------------------------------------------------------------------------

  # --------------------------------------------------------------------------
  # This section specifies a medium-sized VM instance ('fvm16-0MB') having:
  #   16 MB RAM, 16 MB ROM, 0-sized stdblk, no memory-mapped devices.
  # Suitable for:
  #   * Running the Freelang compiler; or
  #   * Running other programs that do not use stdblk.
  # Has:
  #   ARBITRARY_MEMORY_SIZE = 16777216 bytes
  #   STDBLK_SIZE = 0 bytes
  # Uncomment the next four lines to use this size of VM instance:
  # .equ ARBITRARY_MEMORY_SIZE,           0b00000001000000000000000000000000
  # .equ PROG_MEMORY_MASK,                0b11111110000000000000000000000000
  # .equ STDBLK_SIZE,                     0b00000000000000000000000000000000
  # .equ STDBLK_MEMORY_MASK,              0b11111111111111111111111111111111
  # Create 0-sized std.blk file at Linux command line by:
  #   touch std.blk
  # --------------------------------------------------------------------------

  # --------------------------------------------------------------------------
  # This section specifies a small VM instance ('fvm16-0kB') having:
  #   16 kB RAM, 16 kB ROM, 0-sized stdblk, no memory-mapped devices.
  # Suitable for:
  #   * Running the Freelang decompiler (far more than sufficient!); or
  #   * Running other programs having small requirements.
  # Has:
  #   ARBITRARY_MEMORY_SIZE = 16384 bytes
  #   STDBLK_SIZE = 0 bytes
  # Uncomment the next four lines to use this size of VM instance:
     .equ ARBITRARY_MEMORY_SIZE,           0b00000000000000000100000000000000
     .equ PROG_MEMORY_MASK,                0b11111111111111111000000000000000
     .equ STDBLK_SIZE,                     0b00000000000000000000000000000000
     .equ STDBLK_MEMORY_MASK,              0b11111111111111111111111111111111
  # Create 0-sized std.blk file at Linux command line by:
  #   touch std.blk
  # --------------------------------------------------------------------------

  # The following do not change
  .equ HALF_WORD_SIZE, 2                # bytes
  .equ WORD_SIZE, 4                     # bytes
  .equ TWO_WORDS_SIZE, 8                # bytes
  .equ THREE_WORDS_SIZE, 12             # bytes
  .equ FOUR_WORDS_SIZE, 16              # bytes
  .equ MAX_DEPTH_DS, 32                 # elements (words) (power of 2)
  .equ MAX_DEPTH_RS, 32                 # elements (words) (power of 2)
  .equ MAX_DEPTH_SS, 32                 # elements (words) (power of 2)
  .equ NEG_INT_MAX, -2147483648         # highest possible negative word value
  # This VM implementation happens to make ROM and RAM the same size
  #   as each other but that is not at all mandatory. Furthermore,
  #   STDBLK_SIZE does not have to be related to these sizes
  #   but must be a word-multiple or zero.
  .equ ROM_SIZE, ARBITRARY_MEMORY_SIZE  # ROM, RAM, MAP must be word-multiples
  .equ RAM_SIZE, ARBITRARY_MEMORY_SIZE
  .equ ROM_SIZE_WDS, ROM_SIZE / WORD_SIZE
  # This VM implementation does not provide any memory-mapped device,
  #   therefore we set MAP_SIZE to 0 here
  .equ MAP_SIZE, 0                      # MAP immediately follows RAM
  .equ LOWEST_WRITABLE_BYTE, ROM_SIZE   # RAM immediately follows ROM
  .equ HIGHEST_WRITABLE_BYTE, ROM_SIZE + RAM_SIZE + MAP_SIZE - 1
  .equ HIGHEST_WRITABLE_WORD, ROM_SIZE + RAM_SIZE + MAP_SIZE - WORD_SIZE
  .equ STDBLK, 0
  .equ STDIN, 1
  .equ STDIMP, 2
  .equ STDOUT, -1
  .equ STDEXP, -2
  .equ FALSE, 0
  .equ TRUE, 1
  .equ LOWEST_SIMPLE_OPCODE, EXIT
  .equ OPCODE_MASK, 0b11111111111111111111111100000000
  .equ LAST_RESTART_CODE_RESET, 1     # Indicates program-requested RESET
  .equ LAST_RESTART_CODE_REBOOT, 2    # Indicates program-requested REBOOT
  stdblkFilename: .asciz "std.blk"    # Name of file for standard block
  romFilename: .asciz "rom.fp"        # Name of file for system ROM
  stdtrcFilename: .asciz "std.trc"    # Name of file for standard trace
  stdexpFilename: .asciz "std.exp"    # Name of file for standard export
  stdimpFilename: .asciz "std.imp"    # Name of file for standard import
  # Error messages for traps (these go to stdtrc)
  msgTrapIllegalOpcode: .asciz      "ILLEGAL OPCODE    "
  msgTrapMathOverflow: .asciz       "MATH OVERFLOW     "
  msgTrapDsOverflow: .asciz         "DS OVERFLOW       "
  msgTrapDsUnderflow: .asciz        "DS UNDERFLOW      "
  msgTrapRsOverflow: .asciz         "RS OVERFLOW       "
  msgTrapRsUnderflow: .asciz        "RS UNDERFLOW      "
  msgTrapSsOverflow: .asciz         "SS OVERFLOW       "
  msgTrapSsUnderflow: .asciz        "SS UNDERFLOW      "
  msgTrapXsBitshift: .asciz         "XS BITSHIFT       "
  msgTrapMemBounds: .asciz          "BEYOND MEM BOUNDS "
  msgTrapRAMBounds: .asciz          "BEYOND RAM BOUNDS "
  msgTrapCantOpenStdblk: .asciz     "CAN'T OPEN STDBLK "
  msgTrapCantCloseStdblk: .asciz    "CAN'T CLOSE STDBLK"
  msgTrapCantOpenRom: .asciz        "CAN'T OPEN ROM    "
  msgTrapCantCloseRom: .asciz       "CAN'T CLOSE ROM   "
  msgTrapCantReadRom: .asciz        "CAN'T READ ROM    "
  msgTrapCantOpenStdexp: .asciz     "CAN'T OPEN STDEXP "
  msgTrapCantCloseStdexp: .asciz    "CAN'T CLOSE STDEXP"
  msgTrapCantOpenStdimp: .asciz     "CAN'T OPEN STDIMP "
  msgTrapCantCloseStdimp: .asciz    "CAN'T CLOSE STDIMP"
  msgTrapDivideByZero: .asciz       "DIVIDE BY ZERO    "
  msgTrapWall: .asciz               "HIT WALL          "
  msgTrapData: .asciz               "HIT DATA          "
  msgTrapPcOverflow: .asciz         "PC OVERFLOW       "
  msgBefore: .asciz                 " just before:     "

# ============================================================================
.section .bss #                 VARIABLES
# ============================================================================
  # WARNING: Do not change the order of the declaration of these
  #   variables without carefully considering the effect of doing so
  #   on systemHardReset and systemSoftReset!

  .lcomm sMark, WORD_SIZE            # Merely marks start of FVM memory space

  .lcomm rchannel, WORD_SIZE            # Channel for READOR
  .lcomm wchannel, WORD_SIZE            # Channel for WRITOR
  .lcomm gchannel, WORD_SIZE            # Channel for GETOR
  .lcomm pchannel, WORD_SIZE            # Channel for PUTOR
  .lcomm readBuf, WORD_SIZE             # Tiny buffer for READOR
  .lcomm writeBuf, WORD_SIZE            # Tiny buffer for WRITOR
  .lcomm getBuf, WORD_SIZE              # Tiny buffer for GETOR
  .lcomm putBuf, WORD_SIZE              # Tiny buffer for PUTOR
  .lcomm stdblkHandle, WORD_SIZE        # Linux file handle for stdblk file
  .lcomm romHandle, WORD_SIZE           # Linux file handle for ROM file
  .lcomm stdtrcHandle, WORD_SIZE        # Linux file handle for stdtrc file
  .lcomm stdexpHandle, WORD_SIZE        # Linux file handle for stdexp file
  .lcomm stdimpHandle, WORD_SIZE        # Linux file handle for stdimp file

  .lcomm pcTmp, WORD_SIZE               # Only used when need to park %esi

  .lcomm rsp, WORD_SIZE                 # Return stack pointer
  .lcomm rs, MAX_DEPTH_RS * WORD_SIZE   # Return stack
  .lcomm rsStop, WORD_SIZE              # One-word of empty space for safety
  .equ RS_EMPTY, rsStop
  .equ RS_HAS_ONE, rsStop - WORD_SIZE
  .equ RS_FULL, rs

  .lcomm ssp, WORD_SIZE                 # Software stack pointer
  .lcomm ss, MAX_DEPTH_SS * WORD_SIZE   # Software stack
  .lcomm ssStop, WORD_SIZE              # One-word of empty space for safety
  .equ SS_EMPTY, ssStop
  .equ SS_HAS_ONE, ssStop - WORD_SIZE
  .equ SS_HAS_TWO, ssStop - WORD_SIZE * 2
  .equ SS_HAS_THREE, ssStop - WORD_SIZE * 3
  .equ SS_HAS_FOUR, ssStop - WORD_SIZE * 4
  .equ SS_FULL, ss
  .equ SS_ONE_LESS_FULL, ss + WORD_SIZE
  .equ SS_TWO_LESS_FULL, ss + WORD_SIZE * 2
  .equ SS_THREE_LESS_FULL, ss + WORD_SIZE * 3
  .equ SS_FOUR_LESS_FULL, ss + WORD_SIZE * 4

  .lcomm dsp, WORD_SIZE                 # Data stack pointer
  .lcomm ds, MAX_DEPTH_DS * WORD_SIZE   # Data stack
  .lcomm dsStop, WORD_SIZE              # One-word of empty space for safety
  .lcomm memory ROM_SIZE + RAM_SIZE + MAP_SIZE # System memory (power of 2)
  # Note that memory is the last real variable other than
  #   lastExitCode and lastRestartCode below
  .equ DS_EMPTY, dsStop
  .equ DS_HAS_ONE, dsStop - WORD_SIZE
  .equ DS_HAS_TWO, dsStop - WORD_SIZE * 2
  .equ DS_HAS_THREE, dsStop - WORD_SIZE * 3
  .equ DS_HAS_FOUR, dsStop - WORD_SIZE * 4
  .equ DS_ONE_LESS_FULL, ds + WORD_SIZE
  .equ DS_TWO_LESS_FULL, ds + WORD_SIZE * 2
  .equ DS_THREE_LESS_FULL, ds + WORD_SIZE * 3
  .equ DS_FOUR_LESS_FULL, ds + WORD_SIZE * 4
  .equ DS_FULL, ds

  .lcomm lastExitCode, WORD_SIZE    # Last automated exit code (if any),
                                    #   not preserved after a hard reset.
  .lcomm eMark, WORD_SIZE           # Merely marks end of FVM memory space
                                    #   except for lastRestartCode.
  .lcomm lastRestartCode, WORD_SIZE # Program-requested restart code (if any)
                                    #   preserved even after a hard reset.

# ============================================================================
#        VARIABLES FOR TRACING (optional for production VM)
# ============================================================================

  # Number of bits below most significant nibble of a word
  .equ NIBBLED_BITS, ((WORD_SIZE - 1) * 8) + 4
  .lcomm vmFlags, 1 # Flags -------1 = trace on

# ============================================================================
.section .text #    EXAMPLE OF INDIRECT THREADED PROGRAM
# ============================================================================
#  Uncomment this section, along with systemCopyProgram below, to copy
#  the below hardcoded example program into ROM rather than loading a program
#  into ROM from the ROM file (as systemLoadProgram normally does).
#  You must also comment out systemLoadProgram.
#
#  Note: either way, performance is the same since the actual program
#  (once copied or loaded into ROM) executes from ROM just the same.
#  To obtain faster performance one could in theory either write a
#  just-in-time compiler for Freeputer programs that would nicely compile
#  FVM instructions into native machine code or use hardcoded direct
#  threading rather than hardcoded indirect threading. However,
#  one must bear in mind that the safety and lack of undefined behaviour
#  of the FVM must be retained (not discarded in favour of speed).
/*
  .equ _COUNTER, 10
  itProg:
    .long LIT, _COUNTER
    countdown:
      .long LIT, 65, WRITOR, cantWrite - itProg
      .long DEC, BRGZ
      .long countdown - itProg
    finish:
      .long DROP, HALT
    cantWrite:
      .long HALT

  .equ PROGRAM_SIZE, . - itProg
*/

# ============================================================================
#          MACROS for tracing (optional for production VM)
# ============================================================================

  # Trace if trace flag set in vmFlags
  .macro optTrace
    movl vmFlags, %eax
    andl $0b0000000000000000000000000000001, %eax
    jz 9f
      call traceInfo
      call traceStacks
    9:
  .endm

# ============================================================================
#                                MACROS
# ============================================================================

  # Increments the program counter
  .macro incPc
    addl $WORD_SIZE, %esi             # Increment %esi (the program counter)
    movl %esi, %edx
    andl $PROG_MEMORY_MASK, %edx
    jnz trapPcOverflow
  .endm

  # Executes the next instruction --------------------------------------------
  .macro next # --------------------------------------------------------------

  .ifdef TRON_ENABLED
  # Uncomment only 1 of the next 2 lines:
  # call optionalTrace # (1) Gives a slow but 20% smaller executable than (2).
    optTrace # (2) Gives a larger but 20% faster executable than (1).
    # Note: for production deployment it is normal to comment out the
    #  definition of TRON_ENABLED (see far above) so that the assembled FVM
    #  will retain the small size of option (1) while delivering
    #  about 40% better performance. However, the TRON instruction
    #  will then have no effect, making debugging difficult.
  .endif # .ifdef TRON_ENABLED

    movl memory(,%esi,1), %eax        # (%esi) points at the next command
    incPc                             # Increment the program counter
    # Ensure opcode is between 0 and highest legal opcode
    movl %eax, %edx
    andl $0b11111111111111111111111100000000, %edx
    jnz trapIllegalOpcode
      # Find address of command to execute by (o*WORD_SIZE) + vectorTableAddr
      leal vectorTable(,%eax,WORD_SIZE), %eax
      jmp *(%eax)                       # Execute command
	.endm # .macro next --------------------------------------------------------
  # --------------------------------------------------------------------------

  # Puts word at current program cell into %eax
  .macro wordAtPc
    movl memory(,%esi,1), %eax
  .endm

  # Branches to address in the next program cell
  .macro branch
    movl memory(,%esi,1), %esi
    # Ensure branch address lies within program memory
      movl %esi, %edx
      andl $PROG_MEMORY_MASK, %edx
      jnz trapPcOverflow
   next
	.endm

  branchNow:
    branch

  # Skips the next program cell (rather than branching)
  .macro dontBranch
      incPc
      next
	.endm

  # Pushes value in %eax onto the return stack
  .macro pushRs
    movl rsp, %edi
    cmpl $RS_FULL, %edi
    jle trapRsOverflow
    subl $WORD_SIZE, %edi
    movl %eax, (%edi)
    movl %edi, rsp
  .endm

  # Pushes value in %eax onto the software stack
  .macro pushSs
    movl ssp, %edi
    cmpl $SS_FULL, %edi
    jle trapSsOverflow
    subl $WORD_SIZE, %edi
    movl %eax, (%edi)
    movl %edi, ssp
  .endm

  # Pushes value in %eax onto the data stack
  .macro pushDs
    movl dsp, %edi
    cmpl $DS_FULL, %edi
    jle trapDsOverflow
    subl $WORD_SIZE, %edi
    movl %eax, (%edi)
    movl %edi, dsp
  .endm

  # Peeks at the top of the data stack into %eax
  .macro peekDs
    cmpl $DS_HAS_ONE, dsp
    jg trapDsUnderflow
    movl dsp, %edi
    movl (%edi), %eax
  .endm

  # Peeks at the second element from the top of the data stack into %ebx
  .macro peekSecondDs
    cmpl $DS_HAS_TWO, dsp
    jg trapDsUnderflow
    movl dsp, %ebx
    addl $WORD_SIZE, %ebx
    movl (%ebx), %ebx
  .endm

  # Peeks at the third element from the top of the data stack into %ecx
  .macro peekThirdDs
    cmpl $DS_HAS_THREE, dsp
    jg trapDsUnderflow
    movl dsp, %ecx
    addl $TWO_WORDS_SIZE, %ecx
    movl (%ecx), %ecx
  .endm

  # Peeks twice from the data stack, first into %eax and second into %ebx.
  # WARNING: THIS IS INTENTIONALLY OPPOSITE ORDER TO twoPopDS.
  .macro twoPeekDs
    cmpl $DS_HAS_TWO, dsp
    jg trapDsUnderflow
    movl dsp, %edx
    movl (%edx), %eax
    addl $WORD_SIZE, %edx
    movl (%edx), %ebx
  .endm

  # Peeks thrice from the data stack, first into %eax, second into %ebx
  # and third into %ecx.
  # WARNING: THIS IS INTENTIONALLY OPPOSITE ORDER TO threePopDS.
  .macro threePeekDs
    cmpl $DS_HAS_THREE, dsp
    jg trapDsUnderflow
    movl dsp, %edx
    movl (%edx), %eax
    addl $WORD_SIZE, %edx
    movl (%edx), %ebx
    addl $WORD_SIZE, %edx
    movl (%edx), %ecx
  .endm

  # Peeks four times from the data stack, first into %eax, second into %ebx,
  # third into %ecx and fourth into %edx.
  # WARNING: THIS IS INTENTIONALLY OPPOSITE ORDER TO fourPopDS.
  .macro fourPeekDs
    cmpl $DS_HAS_FOUR, dsp
    jg trapDsUnderflow
    movl dsp, %edx
    movl (%edx), %eax
    addl $WORD_SIZE, %edx
    movl (%edx), %ebx
    addl $WORD_SIZE, %edx
    movl (%edx), %ecx
    addl $WORD_SIZE, %edx
    movl (%edx), %edx
  .endm

  # Replaces the value on top of the data stack with value in %eax
  .macro replaceDs
    cmpl $DS_HAS_ONE, dsp
    jg trapDsUnderflow
    movl dsp, %edi
    movl %eax, (%edi)
  .endm

  # Replaces the second value from top of the data stack with value in %ebx
  .macro replaceSecondDs
    cmpl $DS_HAS_TWO, dsp
    jg trapDsUnderflow
    movl dsp, %eax
    addl $WORD_SIZE, %eax
    movl %ebx, (%eax)
  .endm

  # Replaces the third value from top of the data stack with value in %ecx
  .macro replaceThirdDs
    cmpl $DS_HAS_THREE, dsp
    jg trapDsUnderflow
    movl dsp, %eax
    addl $TWO_WORDS_SIZE, %eax
    movl %ecx, (%eax)
  .endm

  # Replaces the value on top of the data stack with value in %eax
  #   and second-top value on the data stack with value in %ebx
  # WARNING: THIS IS INTENTIONALLY OPPOSITE ORDER TO twoPopDS.
  .macro twoReplaceDs
    cmpl $DS_HAS_TWO, dsp
    jg trapDsUnderflow
    movl dsp, %edx
    movl %eax, (%edx)
    addl $WORD_SIZE, %edx
    movl %ebx, (%edx)
  .endm

  # Pushes first %ebx and second %eax onto the data stack
  # WARNING: THIS IS INTENTIONALLY SAME ORDER TO twoPopSS.
  .macro twoPushDs
    cmpl $DS_TWO_LESS_FULL, dsp
    jl trapDsOverflow
    movl dsp, %edi
    subl $WORD_SIZE, %edi
    movl %ebx, (%edi)
    subl $WORD_SIZE, %edi
    movl %eax, (%edi)
    movl %edi, dsp
  .endm

  # Pushes first %ebx and second %eax onto the software stack
  # WARNING: THIS IS INTENTIONALLY SAME ORDER TO twoPopSS.
  .macro twoPushSs
    cmpl $SS_TWO_LESS_FULL, ssp
    jl trapSsOverflow
    movl ssp, %edi
    subl $WORD_SIZE, %edi
    movl %ebx, (%edi)
    subl $WORD_SIZE, %edi
    movl %eax, (%edi)
    movl %edi, ssp
  .endm

  # Pushes first %ecx and second %ebx and third %eax onto the data stack
  # WARNING: THIS IS INTENTIONALLY SAME ORDER TO threePopSS.
  .macro threePushDs
    cmpl $DS_THREE_LESS_FULL, dsp
    jl trapDsOverflow
    movl dsp, %edi
    subl $WORD_SIZE, %edi
    movl %ecx, (%edi)
    subl $WORD_SIZE, %edi
    movl %ebx, (%edi)
    subl $WORD_SIZE, %edi
    movl %eax, (%edi)
    movl %edi, dsp
  .endm

  # Pushes first %edx and second %ecx and third %ebx and fourth %eax
  #   onto the data stack
  # WARNING: THIS IS INTENTIONALLY SAME ORDER AS fourPopSS.
  .macro fourPushDs
    cmpl $DS_FOUR_LESS_FULL, dsp
    jl trapDsOverflow
    movl dsp, %edi
    subl $WORD_SIZE, %edi
    movl %edx, (%edi)
    subl $WORD_SIZE, %edi
    movl %ecx, (%edi)
    subl $WORD_SIZE, %edi
    movl %ebx, (%edi)
    subl $WORD_SIZE, %edi
    movl %eax, (%edi)
    movl %edi, dsp
  .endm


  # Pushes first %ecx and second %ebx and third %eax onto the software stack
  # WARNING: THIS IS INTENTIONALLY SAME ORDER AS threePopSS.
  .macro threePushSs
    cmpl $SS_THREE_LESS_FULL, ssp
    jl trapSsOverflow
    movl ssp, %edi
    subl $WORD_SIZE, %edi
    movl %ecx, (%edi)
    subl $WORD_SIZE, %edi
    movl %ebx, (%edi)
    subl $WORD_SIZE, %edi
    movl %eax, (%edi)
    movl %edi, ssp
  .endm

  # Pushes first %edx and second %ecx and third %ebx and fourth %eax
  #   onto the software stack
  # WARNING: THIS IS INTENTIONALLY SAME ORDER AS fourPopSS.
  .macro fourPushSs
    cmpl $SS_FOUR_LESS_FULL, ssp
    jl trapSsOverflow
    movl ssp, %edi
    subl $WORD_SIZE, %edi
    movl %edx, (%edi)
    subl $WORD_SIZE, %edi
    movl %ecx, (%edi)
    subl $WORD_SIZE, %edi
    movl %ebx, (%edi)
    subl $WORD_SIZE, %edi
    movl %eax, (%edi)
    movl %edi, ssp
  .endm

  # Replaces the value on top of the data stack with value in %ecx
  #   and second-top value on the data stack with value in %eax
  #   and third-top value on the data stack with value in %ebx
  # WARNING: THIS IS INTENTIONALLY DIFFERENT TO threePeekDs.
  .macro threeReplaceDs
    cmpl $DS_HAS_THREE, dsp
    jg trapDsUnderflow
    movl dsp, %edx
    movl %ecx, (%edx)
    addl $WORD_SIZE, %edx
    movl %eax, (%edx)
    addl $WORD_SIZE, %edx
    movl %ebx, (%edx)
  .endm

  # Replaces the value on top of the data stack with value in %ebx
  #   and second-top value on the data stack with value in %ecx
  #   and third-top value on the data stack with value in %eax
  # WARNING: THIS IS INTENTIONALLY DIFFERENT TO threeReplaceDs.
  .macro threeRReplaceDs
    cmpl $DS_HAS_THREE, dsp
    jg trapDsUnderflow
    movl dsp, %edx
    movl %ebx, (%edx)
    addl $WORD_SIZE, %edx
    movl %ecx, (%edx)
    addl $WORD_SIZE, %edx
    movl %eax, (%edx)
  .endm

  # Pop from the data stack into %eax.
  .macro popDs
    movl dsp, %edi
    cmpl $DS_HAS_ONE,  %edi
    jg trapDsUnderflow
    movl (%edi), %eax
    addl $WORD_SIZE,  %edi
    movl %edi, dsp
  .endm

  # Pop from the return stack into %eax.
  .macro popRs
    movl rsp, %edi
    cmpl $RS_HAS_ONE, %edi
    jg trapRsUnderflow
    movl (%edi), %eax
    addl $WORD_SIZE, %edi
    movl %edi, rsp
  .endm

  # Peek from the software stack into %eax.
  .macro peekSs
    movl ssp, %edi
    cmpl $SS_HAS_ONE, %edi
    jg trapSsUnderflow
    movl (%edi), %eax
  .endm

  # Peeks twice from the software stack, first into %ebx and second into %eax.
  .macro twoPeekSs
    cmpl $SS_HAS_TWO, ssp
    jg trapSsUnderflow
    movl ssp, %edi
    movl (%edi), %ebx
    addl $WORD_SIZE,  %edi
    movl (%edi), %eax
  .endm

  # Peeks thrice from the software stack, first into %ecx and second into %ebx
  # and third into %eax.
  .macro threePeekSs
    cmpl $SS_HAS_THREE, ssp
    jg trapSsUnderflow
    movl ssp, %edi
    movl (%edi), %ecx
    addl $WORD_SIZE,  %edi
    movl (%edi), %ebx
    addl $WORD_SIZE,  %edi
    movl (%edi), %eax
  .endm


  # Peeks 4 times from the software stack, first into %edx and 2nd into %ecx
  # and third into %ebx and fourth into %eax.
  .macro fourPeekSs
    cmpl $SS_HAS_FOUR, ssp
    jg trapSsUnderflow
    movl ssp, %edi
    movl (%edi), %edx
    addl $WORD_SIZE,  %edi
    movl (%edi), %ecx
    addl $WORD_SIZE,  %edi
    movl (%edi), %ebx
    addl $WORD_SIZE,  %edi
    movl (%edi), %eax
  .endm


  # Pop from the software stack into %eax.
  .macro popSs
    movl ssp, %edi
    cmpl $SS_HAS_ONE, %edi
    jg trapSsUnderflow
    movl (%edi), %eax
    addl $WORD_SIZE, %edi
    movl %edi, ssp
  .endm

  # Pops twice from the software stack, first into %ebx and second into %eax.
  .macro twoPopSs
    cmpl $SS_HAS_TWO, ssp
    jg trapSsUnderflow
    movl ssp, %edi
    movl (%edi), %ebx
    addl $WORD_SIZE,  %edi
    movl %edi, ssp
    movl ssp, %edi
    movl (%edi), %eax
    addl $WORD_SIZE,  %edi
    movl %edi, ssp
  .endm

  # Pops thrice from the software stack, first into %ecx and second into %ebx
  # and third into %eax.
  .macro threePopSs
    cmpl $SS_HAS_THREE, ssp
    jg trapSsUnderflow
    movl ssp, %edi
    movl (%edi), %ecx
    addl $WORD_SIZE,  %edi
    movl %edi, ssp
    movl ssp, %edi
    movl (%edi), %ebx
    addl $WORD_SIZE,  %edi
    movl %edi, ssp
    movl ssp, %edi
    movl (%edi), %eax
    addl $WORD_SIZE,  %edi
    movl %edi, ssp
  .endm


  # Pops 4 times from the software stack, first into %edx and second into %ecx
  # and third into %ebx and fourth into %eax.
  .macro fourPopSs
    cmpl $SS_HAS_FOUR, ssp
    jg trapSsUnderflow
    movl ssp, %edi
    movl (%edi), %edx
    addl $WORD_SIZE,  %edi
    movl %edi, ssp
    movl ssp, %edi
    movl (%edi), %ecx
    addl $WORD_SIZE,  %edi
    movl %edi, ssp
    movl ssp, %edi
    movl (%edi), %ebx
    addl $WORD_SIZE,  %edi
    movl %edi, ssp
    movl ssp, %edi
    movl (%edi), %eax
    addl $WORD_SIZE,  %edi
    movl %edi, ssp
  .endm


  # Pops twice from the data stack, first into %ebx and second into %eax.
  # WARNING: INTENTIONALLY OPPOSITE ORDER TO twoPeekDS and twoReplaceDs.
  .macro twoPopDs
    cmpl $DS_HAS_TWO, dsp
    jg trapDsUnderflow
    movl dsp, %edi
    movl (%edi), %ebx
    addl $WORD_SIZE,  %edi
    movl %edi, dsp
    movl dsp, %edi
    movl (%edi), %eax
    addl $WORD_SIZE,  %edi
    movl %edi, dsp
  .endm

  # Pops thrice from the data stack, first into %ecx and second into %ebx
  # and third into %eax.
  # WARNING: INTENTIONALLY OPPOSITE ORDER TO threeReplaceDs.
  .macro threePopDs
    cmpl $DS_HAS_THREE, dsp
    jg trapDsUnderflow
    movl dsp, %edi
    movl (%edi), %ecx
    addl $WORD_SIZE,  %edi
    movl %edi, dsp
    movl dsp, %edi
    movl (%edi), %ebx
    addl $WORD_SIZE,  %edi
    movl %edi, dsp
    movl dsp, %edi
    movl (%edi), %eax
    addl $WORD_SIZE,  %edi
    movl %edi, dsp
  .endm


  # Pops 4 times from the data stack, first into %edx and second into %ecx
  # and third into %ebx and fourth into %eax.
  # WARNING: INTENTIONALLY OPPOSITE ORDER TO threeReplaceDs.
  .macro fourPopDs
    cmpl $DS_HAS_FOUR, dsp
    jg trapDsUnderflow
    movl dsp, %edi
    movl (%edi), %edx
    addl $WORD_SIZE,  %edi
    movl %edi, dsp
    movl dsp, %edi
    movl (%edi), %ecx
    addl $WORD_SIZE,  %edi
    movl %edi, dsp
    movl dsp, %edi
    movl (%edi), %ebx
    addl $WORD_SIZE,  %edi
    movl %edi, dsp
    movl dsp, %edi
    movl (%edi), %eax
    addl $WORD_SIZE,  %edi
    movl %edi, dsp
  .endm

  # Drops from the data stack
  .macro dropDs
    cmpl $DS_HAS_ONE, dsp
    jg trapDsUnderflow
    addl $WORD_SIZE, dsp
  .endm

  # Drops 2 elements from the data stack
  .macro twoDropDs
    cmpl $DS_HAS_TWO, dsp
    jg trapDsUnderflow
    addl $TWO_WORDS_SIZE, dsp
  .endm

  # Drops 3 elements from the data stack
  .macro threeDropDs
    cmpl $DS_HAS_THREE, dsp
    jg trapDsUnderflow
    addl $THREE_WORDS_SIZE, dsp
  .endm

  # Drops 4 elements from the data stack
  .macro fourDropDs
    cmpl $DS_HAS_FOUR, dsp
    jg trapDsUnderflow
    addl $FOUR_WORDS_SIZE, dsp
  .endm

  .macro ensureByteAddrWritable reg
        cmpl $LOWEST_WRITABLE_BYTE, \reg
        jl trapRAMBounds
        cmpl $HIGHEST_WRITABLE_BYTE, \reg
        jg trapRAMBounds
  .endm

  .macro ensureWordAddrWritable reg
        cmpl $LOWEST_WRITABLE_BYTE, \reg
        jl trapRAMBounds
        cmpl $HIGHEST_WRITABLE_WORD, \reg
        jg trapRAMBounds
  .endm

  .macro ensureByteAddressable reg
        cmpl $0, \reg
        jl trapMemBounds
        cmpl $HIGHEST_WRITABLE_BYTE, \reg
        jg trapMemBounds
  .endm

  .macro ensureWordAddressable reg
        cmpl $0, \reg
        jl trapMemBounds
        cmpl $HIGHEST_WRITABLE_WORD, \reg
        jg trapMemBounds
  .endm

  # Temporarily save the program counter (%esi) into pcTmp
  .macro parkPc
    movl %esi, pcTmp
  .endm

  # Restore the program counter (%esi) from pcTmp
  .macro unparkPc
    movl pcTmp, %esi
  .endm

# ============================================================================
#                  TRACING (optional for production VM)
# ============================================================================

  # Instruction: iTROFF
  iTROFF:
    andl $0b11111111111111111111111111111110, %eax
    movl %eax, vmFlags
    next

  # Instruction: iTRON
  iTRON:
    orl $0b00000000000000000000000000000001, %eax
    movl %eax, vmFlags
    next

.ifdef TRON_ENABLED
  # Trace if trace flag set in vmFlags
  optionalTrace:
    optTrace
  ret

  trapTraceBounds:
    movl $48, %ebx                   # exitCode for trace beyond memory bounds
    jmp systemReset

  trapTraceOverflow:
    movl $49, %ebx                    # exitCode for trace overflow
    jmp systemReset

  # Write ascii length=8 to standard out -----
  traceStr8:
    pusha
    movl stdtrcHandle, %ebx           # Linux file handle for stdtrc
    movl %eax, %ecx                   # Tiny output buffer (ascii location)
    movl $0x8, %edx                   # Write 8 characters only
    movl $0x4, %eax                   # Linux call ID for write
    int $0x80                         # Linux interrupt for system call
    popa
    ret
  # ------------------------------------------

  # Write one word in %eax to stdtrc as hexadecimal ----
  traceWord:
    pusha
    movl %eax, %edi                   # word saved in %edi
    movl $0x8, %ecx                   # counter
    traceNextNibble:
      movl %edi, %eax
      call traceNibble
      shll $0x4, %edi
      loop traceNextNibble
    popa
    ret
  # ----------------------------------------------------

  # Write one word in %eax as hexadecimal followed by space ----
  traceWordSpc:
      call traceWord
      call traceSpc
      ret
  # ------------------------------------------------------------

  # Write highest nibble in %eax to stdtrc as hexadecimal ----
  traceNibble:
    pusha
    shrl $NIBBLED_BITS, %eax
    cmpl $0x9, %eax
    jg alphaDigit
      addl $0x30, %eax                # 0
      call traceChar
      popa
      ret
    alphaDigit:
      #shrl $28, %eax
      addl $0x57, %eax                # a
      call traceChar
      popa
      ret
  # --------------------------------------------------------

  # Write one character, followed by a space, to stdtrc ----
  traceCharSpc:
    call traceChar
    call traceSpc
    ret
  # --------------------------------------------------------

  # Write one space to stdtrc --------
  traceSpc:
    push %eax
    movl $0x20, %eax
    call traceChar
    pop %eax
    ret
  # ----------------------------------

  # Write %ecx spaces to stdtrc --------
  traceSpaces:
    movl %ecx, %esi                 # save counter
    movl $0x20, %eax
    call traceChar
    movl %esi, %ecx                 # restore counter
    loopl traceSpaces
    ret
  # ----------------------------------

  # Write tracing information (part 1) ----
  traceInfo:
    parkPc
    # Trace pc
    movl %esi, %eax
    call traceWordSpc
    # Trace command about to be executed
    movl memory(,%esi,1), %eax        # (%esi) points at the next command
    # call traceWordSpc               # Uncomment to see raw opcode
    imull $8, %eax
    leal traceTable(,%eax,1), %eax
    call traceStr8
    call traceSpc
    # If it is a complex command, trace the following program cell
    movl memory(,%esi,1), %eax
    cmpl $LOWEST_SIMPLE_OPCODE, %eax
    jge simpleInstr
      movl %esi, %edi
      addl $WORD_SIZE, %edi
      movl %edi, %edx
      andl $PROG_MEMORY_MASK, %edx
      jnz trapPcOverflow
      movl memory(,%edi,1), %eax
      call traceWordSpc
      unparkPc
      ret
    simpleInstr:
      movl $9, %ecx
      call traceSpaces
      unparkPc
      ret
  # ---------------------------------------

  # Write tracing information (part 2) ----
  traceStacks:
    parkPc
    # Trace data stack elements
    traceDs:
      movl $'(', %eax
      call traceCharSpc
      movl dsp, %eax
      cmp $DS_HAS_ONE, %eax
      jg finishTraceDs
      movl $DS_HAS_ONE, %ecx
      traceDsElements:
        movl (%ecx), %eax
        call traceWordSpc
        subl $WORD_SIZE, %ecx
        cmpl dsp, %ecx
        jge traceDsElements
      finishTraceDs:
        movl $')', %eax
        call traceCharSpc

    # Trace software stack elements
    traceSs:
      movl $'[', %eax
      call traceCharSpc
      movl ssp, %eax
      cmp $SS_HAS_ONE, %eax
      jg finishTraceSs
      movl $SS_HAS_ONE, %ecx
      traceSsElements:
        movl (%ecx), %eax
        call traceWordSpc
        subl $WORD_SIZE, %ecx
        cmpl ssp, %ecx
        jge traceSsElements
      finishTraceSs:
        movl $']', %eax
        call traceCharSpc

    # Trace return stack elements
    traceRs:
      movl $'{', %eax
      call traceCharSpc
      movl rsp, %eax
      cmp $RS_HAS_ONE, %eax
      jg finishTraceRs
      movl $RS_HAS_ONE, %ecx
      traceRsElements:
        movl (%ecx), %eax
        call traceWordSpc
        subl $WORD_SIZE, %ecx
        cmpl rsp, %ecx
        jge traceRsElements
      finishTraceRs:
        movl $'}', %eax
        call traceCharSpc

    # Finished writing tracing information
    finishTraceLine:
      call traceNewline
    unparkPc
    ret
.endif # .ifdef TRON_ENABLED

  # Write one character to stdtrc --------
  traceChar:
    pusha
    movl %eax, writeBuf
    movl stdtrcHandle, %ebx # (nb Linux file handle for stdout would be 1)
    movl $writeBuf, %ecx              # Tiny output buffer
    movl $0x1, %edx                   # Write one character only
    movl $0x4, %eax                   # Linux call ID for write
    int $0x80                         # Linux interrupt for system call
    testl %eax, %eax
    js trapCantWriteToStdtrc          # Write failed for some reason
    popa
    ret
  # ----------------------------------------

  # Write ascii length=18 to standard out -----
  traceStr18:
    pusha
    movl stdtrcHandle, %ebx           # Linux file handle for stdtrc
    movl %eax, %ecx                   # Tiny output buffer (ascii location)
    movl $18, %edx                    # Write 18 characters only
    movl $0x4, %eax                   # Linux call ID for write
    int $0x80                         # Linux interrupt for system call
    popa
    ret
  # ------------------------------------------

  # Write one newline to stdtrc ------
  traceNewline:
    movl $0xa, %eax
    call traceChar
    ret
  # ----------------------------------

  # --------------------------------------------------------------------------

# ============================================================================
#                            INSTRUCTION SET
# ============================================================================

iRCHAN:
  popDs
  movl %eax, rchannel
  next

iWCHAN:
  popDs
  movl %eax, wchannel
  next

iGCHAN:
  popDs
  movl %eax, gchannel
  next

iPCHAN:
  popDs
  movl %eax, pchannel
  next

.macro setIOdefaults
  movl $0, gchannel
  movl $0, pchannel
  movl $1, rchannel
  movl $-1, wchannel
.endm

iHOMIO:
  setIOdefaults
  next

iECODE:
  movl lastExitCode, %eax
  pushDs
  next

iRCODE:
  movl lastRestartCode, %eax
  pushDs
  next

iROM:
  movl $ROM_SIZE, %eax
  pushDs
  next

iRAM:
  movl $RAM_SIZE, %eax
  pushDs
  next

iMAP:
  movl $MAP_SIZE, %eax
  pushDs
  next

iSTDBLK:
  movl $STDBLK_SIZE, %eax
  pushDs
  next

iDS:
  movl $MAX_DEPTH_DS, %eax
  pushDs
  next

iSS:
  movl $MAX_DEPTH_SS, %eax
  pushDs
  next

iRS:
  movl $MAX_DEPTH_RS, %eax
  pushDs
  next

iDSN:
  movl $dsStop, %eax
  subl dsp, %eax
  cmp $0, %eax
  jle 1f
    # data stack is not empty
    xorl %edx, %edx
    movl $WORD_SIZE, %ebx
    idivl %ebx
    jmp 2f
  1: # data stack is empty
    movl $0, %eax
  2:
  pushDs
  next

iSSN:
  movl $ssStop, %eax
  subl ssp, %eax
  cmp $0, %eax
  jle 1f
    # software stack is not empty
    xorl %edx, %edx
    movl $WORD_SIZE, %ebx
    idivl %ebx
    jmp 2f
  1: # software stack is empty
    movl $0, %eax
  2:
  pushDs
  next

iRSN:
  movl $rsStop, %eax
  subl rsp, %eax
  cmp $0, %eax
  jle 1f
    # return stack is not empty
    xorl %edx, %edx
    movl $WORD_SIZE, %ebx
    idivl %ebx
    jmp 2f
  1: # return stack is empty
    movl $0, %eax
  2:
  pushDs
  next

iREV: # ( n1 n2 n3 -- n3 n2 n1 )
    cmpl $DS_HAS_THREE, dsp
    jg trapDsUnderflow
    movl dsp, %ebx
    addl $TWO_WORDS_SIZE, %ebx
    movl (%ebx), %eax                 # n1
    movl dsp, %edx
    movl (%edx), %ecx                 # n3
    movl %ecx, (%ebx)
    movl %eax, (%edx)
    next

iGETOR:
    xorl %ebx, %ebx
    movl %ebx, getBuf                 # Zero-fill the 1-word get buffer

    movl pchannel, %eax
    cmpl $STDBLK, %eax
    jne branchNow                     # Unsupported channel, cannot get

    popDs
    movl %eax, %edx
    andl $STDBLK_MEMORY_MASK, %edx
    jnz branchNow                     # Outside of block bounds, cannot get

    # Seek position (in %eax from popDs) in stdblk file
    movl stdblkHandle, %ebx           # Linux file handle for stdblk
    movl %eax, %ecx                   # Position (ie, addr in %eax from popDs)
    movl $0x0, %edx                   # Linux SEEK_SET code for positioning
    movl $19, %eax                    # Linux call ID for lseek
    int $0x80                         # Linux interrupt for system call
      test %eax, %eax                 # Will be negative if seek failed
      js branchNow                    # Seek failed for some reason

    # Now that position has been set, read from stdblk file
    movl stdblkHandle, %ebx           # Linux file handle for stdblk
    movl $getBuf, %ecx                # Tiny input buffer
    movl $WORD_SIZE, %edx             # Read one word only
    movl $0x3, %eax                   # Linux call ID for read
    int $0x80                         # Linux interrupt for system call
      test %eax, %eax                 # Will be negative if read failed
      js branchNow                    # Read failed for some reason
    movl getBuf, %eax
    pushDs
    dontBranch

iGETORB:
    xorl %ebx, %ebx
    movl %ebx, getBuf                 # Zero-fill the 1-word get buffer

    movl pchannel, %eax
    cmpl $STDBLK, %eax
    jne branchNow                     # Unsupported channel, cannot get

    popDs
    movl %eax, %edx
    andl $STDBLK_MEMORY_MASK, %edx
    jnz branchNow                     # Outside of block bounds, cannot get

    # Seek position (in %eax from popDs) in stdblk file
    movl stdblkHandle, %ebx           # Linux file handle for stdblk
    movl %eax, %ecx                   # Position (ie, addr in %eax from popDs)
    movl $0x0, %edx                   # Linux SEEK_SET code for positioning
    movl $19, %eax                    # Linux call ID for lseek
    int $0x80                         # Linux interrupt for system call
      test %eax, %eax                 # Will be negative if seek failed
      js branchNow                    # Seek failed for some reason

    # Now that position has been set, read from stdblk file
        movl %ebx, readBuf            # Zero-fill the 1-word read buffer
    movl stdblkHandle, %ebx           # Linux file handle for stdblk
    movl $getBuf, %ecx                # Tiny input buffer
    movl $1, %edx                     # Read 1 byte only
    movl $0x3, %eax                   # Linux call ID for read
    int $0x80                         # Linux interrupt for system call
      test %eax, %eax                 # Will be negative if read failed
      js branchNow                    # Read failed for some reason
    movl getBuf, %eax
    pushDs
    dontBranch

iPUTOR:
    movl pchannel, %eax
    cmpl $STDBLK, %eax
    jne branchNow                     # Unsupported channel, cannot put

    twoPopDs                          # Address is in %ebx
    movl %eax, putBuf                 # Value to store is in putBuf

    movl %ebx, %edx
    andl $STDBLK_MEMORY_MASK, %edx
    jnz branchNow                     # Outside of block bounds, cannot put

    # Seek position (in %ebx from twoPopDs) in stdblk file
    movl %ebx, %ecx                   # Position (ie, addr in %ebx from popDs)
    movl stdblkHandle, %ebx           # Linux file handle for stdblk
    movl $0x0, %edx                   # Linux SEEK_SET=0 mode for positioning
    movl $19, %eax                    # Linux call ID for lseek
    int $0x80                         # Linux interrupt for system call
      test %eax, %eax                 # Will be negative if seek failed
      js branchNow                    # Seek failed for some reason

    # Now that position has been set, write to stdblk file
    movl stdblkHandle, %ebx           # Linux file handle for stdblk
    movl $putBuf, %ecx                # Tiny output buffer
    movl $WORD_SIZE, %edx             # Write one word only
    movl $0x4, %eax                   # Linux call ID for write
    int $0x80                         # Linux interrupt for system call
      test %eax, %eax                 # Will be negative if write failed
      js branchNow                    # Write failed for some reason
    dontBranch

iPUTORB:
    movl pchannel, %eax
    cmpl $STDBLK, %eax
    jne branchNow                     # Unsupported channel, cannot put

    twoPopDs                          # Address is in %ebx
    movl %eax, putBuf                 # Value to store is in putBuf

    movl %ebx, %edx
    andl $STDBLK_MEMORY_MASK, %edx
    jnz branchNow                     # Outside of block bounds, cannot put

    # Seek position (in %ebx from twoPopDs) in stdblk file
    movl %ebx, %ecx                   # Position (ie, addr in %ebx from popDs)
    movl stdblkHandle, %ebx           # Linux file handle for stdblk
    movl $0x0, %edx                   # Linux SEEK_SET=0 mode for positioning
    movl $19, %eax                    # Linux call ID for lseek
    int $0x80                         # Linux interrupt for system call
      test %eax, %eax                 # Will be negative if seek failed
      js branchNow                    # Seek failed for some reason

    # Now that position has been set, write to stdblk file
    movl stdblkHandle, %ebx           # Linux file handle for stdblk
    movl $putBuf, %ecx                # Tiny output buffer
    movl $1, %edx                     # Write 1 byte only
    movl $0x4, %eax                   # Linux call ID for write
    int $0x80                         # Linux interrupt for system call
      test %eax, %eax                 # Will be negative if write failed
      js branchNow                    # Write failed for some reason
    dontBranch

iREADOR:
    xorl %ebx, %ebx
    movl %ebx, readBuf                # Zero-fill the 1-word read buffer

    movl rchannel, %eax
    cmpl $STDIN, %eax
      je readStdin
      cmpl $STDIMP, %eax
        je readStdimp
    jne branchNow                     # Unsupported channel, cannot read
    readStdin:
        movl $0x0, %ebx               # Linux file handle for stdin
        jmp readReady
    readStdimp:
        movl stdimpHandle, %ebx       # Linux file handle for stdin
        jmp readReady
    readReady:
        movl $readBuf, %ecx           # Tiny input buffer
        movl $WORD_SIZE, %edx         # Read 1 word only (wd size in bytes)
        movl $0x3, %eax               # Linux call ID for read
        int $0x80                     # Linux interrupt for system call
          testl %eax, %eax            # Unless error, is num of bytes read
          js branchNow                # Read failed for some reason
          jz branchNow                # Read returned zero bytes
        movl readBuf, %eax
        pushDs
        dontBranch

iREADORB:
    xorl %ebx, %ebx
    movl %ebx, readBuf                # Zero-fill the 1-word read buffer

    movl rchannel, %eax
    cmpl $STDIN, %eax
      je readbStdin
      cmpl $STDIMP, %eax
        je readbStdimp
    jne branchNow                     # Unsupported channel, cannot read
    readbStdin:
        movl $0x0, %ebx               # Linux file handle for stdin
        jmp readbReady
    readbStdimp:
        movl stdimpHandle, %ebx       # Linux file handle for stdin
        jmp readbReady
    readbReady:
        movl $readBuf, %ecx           # Tiny input buffer
        movl $1, %edx                 # Read 1 byte only
        movl $0x3, %eax               # Linux call ID for read
        int $0x80                     # Linux interrupt for system call
          testl %eax, %eax            # Unless error, is num of bytes read
          js branchNow                # Read failed for some reason
          jz branchNow                # Read returned zero bytes
        movl readBuf, %eax
        pushDs
        dontBranch

iWRITOR:
    movl wchannel, %eax
    cmpl $STDOUT, %eax
      je writStdout
      cmpl $STDEXP, %eax
        je writStdexp
    jmp branchNow                     # Unsupported channel, cannot write
    writStdout:
      movl $0x1, %ebx                 # Linux file handle for stdout
      jmp writReady
    writStdexp:
      movl stdexpHandle, %ebx         # Linux file handle for our stdexp file
      jmp writReady
    writReady:
      popDs
      movl %eax, writeBuf
      movl $writeBuf, %ecx            # Tiny output buffer
      movl $WORD_SIZE, %edx           # Write 1 word only (wd size in bytes)
      movl $0x4, %eax                 # Linux call ID for write
      int $0x80                       # Linux interrupt for system call
        testl %eax, %eax
        js branchNow                  # Write failed for some reason
      dontBranch

iWRITORB:
    movl wchannel, %eax
    cmpl $STDOUT, %eax
      je writbStdout
      cmpl $STDEXP, %eax
        je writbStdexp
    jmp branchNow                     # Unsupported channel, cannot write
    writbStdout:
      movl $0x1, %ebx                 # Linux file handle for stdout
      jmp writbReady
    writbStdexp:
      movl stdexpHandle, %ebx         # Linux file handle for our stdexp file
      jmp writbReady
    writbReady:
      popDs
      movl %eax, writeBuf
      movl $writeBuf, %ecx            # Tiny output buffer
      movl $1, %edx                   # Write 1 byte only
      movl $0x4, %eax                 # Linux call ID for write
      int $0x80                       # Linux interrupt for system call
        testl %eax, %eax
        js branchNow                  # Write failed for some reason
      dontBranch

iTRACOR:
    popDs
    movl %eax, writeBuf
    movl stdtrcHandle, %ebx     # (nb Linux file handle for stdout would be 1)
    movl $writeBuf, %ecx              # Tiny output buffer
    movl $WORD_SIZE, %edx             # Write 1 word only (wd size in bytes)
    movl $0x4, %eax                   # Linux call ID for write
    int $0x80                         # Linux interrupt for system call
    testl %eax, %eax
      js branchNow                    # Write failed for some reason
    dontBranch

iTRACORB:
    popDs
    movl %eax, writeBuf
    movl stdtrcHandle, %ebx     # (nb Linux file handle for stdout would be 1)
    movl $writeBuf, %ecx              # Tiny output buffer
    movl $0x1, %edx                   # Write 1 byte only
    movl $0x4, %eax                   # Linux call ID for write
    int $0x80                         # Linux interrupt for system call
    testl %eax, %eax
      js branchNow                    # Write failed for some reason
    dontBranch

iFIND: # ( numWords n src -- wordIndex ) find 1st instance of n from src
       #     onwards but look at no more than numWords
    threePopDs
    parkPc
    movl %eax, %edx                   # numWords is now in %edx
    push %edx                         # preserved numWords for use below
    movl %ecx, %edi                   # src is now in %edi
                                      # note: n is in %ebx
      # ensure src is within memory
      ensureWordAddressable %ecx
      # ensure src won't overflow and will stay within RAM
      movl %edi, %eax
        imull $WORD_SIZE, %edx
        jo trapMemBounds
        addl %edx, %eax
        jo trapMemBounds
        ensureWordAddressable %eax
      pop %edx                        # retrieved numWords for use below
      # find
      movl %edx, %ecx                 # numWords is now in %edx and %ecx
      movl %ebx, %eax                 # n is now in %eax
        test %ecx, %ecx
        jns ascFind                   # If numWords negative, descending
          std                         # Set DF flag (ensures descending)
          neg %ecx                    # Use positive counter
          neg %edx                    # Make original numWords positive also
          jmp doFind
        ascFind:                      # numWords is positive, ascending
          cld                         # Clear DF flag (ensures ascending)
        doFind:
        leal memory(,%edi,1), %edi
        repne scasl
          jne notfound
          found:
            subl %ecx, %edx           # %edx contains original abs(numWords)
            decl %edx                 # Adjust down 1
            movl %edx, %eax
            pushDs
            unparkPc
            next
          notfound:
            movl $-1, %eax            # -1 indicates b not found in src
            pushDs
            unparkPc
            next

iFINDB: # ( numBytes b src -- byteIndex ) find 1st instance of b from src
        #     onwards but look at no more than numBytes
    threePopDs
    parkPc
    movl %eax, %edx                   # numBytes is now in %edx
    movl %ecx, %edi                   # src is now in %edi
                                      # note: b is in %ebx
      # ensure src is within memory
      ensureByteAddressable %ecx
      # ensure src won't overflow and will stay within RAM
      movl %edi, %eax
        addl %edx, %eax
        jo trapMemBounds
        ensureByteAddressable %eax
      # find
      movl %edx, %ecx                 # numBytes is now in %edx and %ecx
      movl %ebx, %eax                 # n is now in %eax
        test %ecx, %ecx
        jns ascFindB                  # If numBytes negative, descending
          std                         # Set DF flag (ensures descending)
          neg %ecx                    # Use positive counter
          neg %edx                    # Make original numWords positive also
          jmp doFindB
        ascFindB:                     # numWords is positive, ascending
          cld                         # Clear DF flag (ensures ascending)
        doFindB:
        leal memory(,%edi,1), %edi
        repne scasb
          jne notfoundb
          foundb:
            subl %ecx, %edx           # %edx contains original numBytes
            decl %edx                 # Adjust down 1
            movl %edx, %eax
            pushDs
            unparkPc
            next
          notfoundb:
            movl $-1, %eax            # -1 indicates b not found in src
            pushDs
            unparkPc
            next

iFILL: # ( numWords n dest -- ) fill numWords with n at dest onwards
    threePopDs
    parkPc
    movl %eax, %edx                   # numWords is now in %edx
    cmpl $0, %edx
    jg 1f
      next                            # nothing to do if numWords not > 0
    1:
    push %edx                         # preserved numWords for use below
    movl %ebx, %esi                   # n is now in %esi
    movl %ecx, %edi                   # dest is in now in %edi
      # ensure dest is within memory
      ensureWordAddrWritable %ecx
      # ensure dest won't overflow and will stay within RAM
      movl %edi, %eax
        imull $WORD_SIZE, %edx
        jo trapMemBounds
        addl %edx, %eax
        jo trapMemBounds
        ensureWordAddrWritable %eax
      pop %edx                        # retrieved numWords for use below
      # fill
      movl %edx, %ecx                 # numWords is now in %edx
      movl %esi, %eax                 # n is now in %eax
      filling:
        movl %eax, memory(,%edi,1)    # Store n to dest in %edi
        addl $WORD_SIZE, %edi
      loopl filling
      unparkPc
      next

iFILLB: # ( numBytes b dest -- ) fill numBytes with b at dest onwards
    threePopDs
    parkPc
    movl %eax, %edx                   # numBytes is now in %edx
    cmpl $0, %edx
    jg 1f
      next                            # nothing to do if numWords not > 0
    1:
    movl %ebx, %esi                   # b is now in %esi
    movl %ecx, %edi                   # dest is in now in %edi
      # ensure dest is within memory
      ensureByteAddrWritable %ecx
      # ensure dest won't overflow and will stay within RAM
      movl %edi, %eax
        addl %edx, %eax
        jo trapMemBounds
        ensureByteAddrWritable %eax
      # fill
      movl %edx, %ecx                 # numBytes is now in %edx
      movl %esi, %eax                 # b is now in %eax
      fillingb:
        movb %al, memory(,%edi,1)     # Store b to dest in %edi
        addl $1, %edi
      loopl fillingb
      unparkPc
      next

iMATCH: # ( numWords src dest -- TRUE/FALSE ) see if strings match
    threePopDs
    parkPc
    movl %eax, %edx                   # numWords is now in %edx

    # Do nothing if numWords <= 0
    cmpl $0, %edx
    jle matched

    push %edx                         # preserved numWords for use below
    movl %ebx, %esi                   # src is now in %esi
    movl %ecx, %edi                   # dest is in now in %edi
    # Do nothing if src = dest
    cmpl %esi, %edi
    je matched
      # ensure src is within memory
      ensureWordAddressable %ebx
      # ensure src won't overflow and will stay within system memory
      movl %esi, %eax
        imull $WORD_SIZE, %edx
        jo trapMemBounds
        addl %edx, %eax
        jo trapMemBounds
        ensureWordAddressable %eax
      # ensure dest is within memory
      ensureWordAddressable %ecx
      # ensure dest won't overflow and will stay within system memory
      movl %edi, %eax
        imull $WORD_SIZE, %edx
        jo trapMemBounds
        addl %edx, %eax
        jo trapMemBounds
        ensureWordAddressable %eax
      pop %edx                        # retrieved numWords for use below
      # Translate source and destination to absolute addresses
      leal memory(,%esi,1), %esi
      leal memory(,%edi,1), %edi
      # compare 'strings'
      movl %edx, %ecx                 # numWords is now in %ecx
        cld                           # Clear DF flag (ensures ascending)
        repe cmpsl
        je matched
      different:
        movl $FALSE, %eax
        pushDs
        unparkPc
        next
      matched:
        movl $TRUE, %eax
        pushDs
        unparkPc
        next

iMATCHB: #  ( numBytes src dest -- TRUE/FALSE ) see if strings match
    threePopDs
    parkPc
    movl %eax, %edx                   # numBytes is now in %edx

    # Do nothing if numWords <= 0
    cmpl $0, %edx
    jle matchedb

    movl %ebx, %esi                   # src is now in %esi
    movl %ecx, %edi                   # dest is in now in %edi
    # Do nothing if src = dest
    cmpl %esi, %edi
    je matchedb
      # ensure src is within memory
      ensureByteAddressable %ebx
      # ensure src won't overflow and will stay within system memory
      movl %esi, %eax
        addl %edx, %eax
        jo trapMemBounds
        ensureByteAddressable %eax
      # ensure dest is within memory
      ensureByteAddressable %ecx
      # ensure dest won't overflow and will stay within system memory
      movl %edi, %eax
        addl %edx, %eax
        jo trapMemBounds
        ensureByteAddressable %eax
      # Translate source and destination to absolute addresses
      leal memory(,%esi,1), %esi
      leal memory(,%edi,1), %edi
      # compare 'strings'
      movl %edx, %ecx                 # numBytes is now in %ecx
        cld                           # Clear DF flag (ensures ascending)
        repe cmpsb
        je matchedb
      differentb:
        movl $FALSE, %eax
        pushDs
        unparkPc
        next
      matchedb:
        movl $TRUE, %eax
        pushDs
        unparkPc
        next

iMOVEB: #  ( numBytes src dest -- ) copy byte at src addr to dest addr
    threePopDs
    parkPc
    movl %eax, %edx                   # numBytes is now in %edx
    movl %ebx, %esi                   # src is now in %esi
    movl %ecx, %edi                   # dest is in now in %edi
    # Do nothing if src = dest
    cmpl %esi, %edi
    je finishedMoveB
      # ensure src is within memory
      ensureByteAddressable %ebx
      # ensure src won't overflow and will stay within system memory
      movl %esi, %eax
        addl %edx, %eax
        jo trapMemBounds
        ensureByteAddressable %eax
      # ensure dest is within memory
      ensureByteAddressable %ecx
      # ensure dest won't overflow and will stay within RAM
      movl %edi, %eax
        addl %edx, %eax
        jo trapMemBounds
        ensureByteAddrWritable %eax
      # copy bytes from source to destination
      movl %edx, %ecx                 # numBytes is now in %ecx
        test %ecx, %ecx
        jns ascMoveB                  # If numBytes negative, descending copy
          std                         # Set DF flag (ensures descending copy)
          neg %ecx                    # Use positive counter
          jmp doMoveB
        ascMoveB:                     # numWords is positive, ascending copy
          cld                         # Clear DF flag (ensures ascending copy)
        doMoveB:
        leal memory(,%esi,1), %esi
        leal memory(,%edi,1), %edi
        rep movsb
    finishedMoveB:
      unparkPc
      next

iMOVE: #  ( numWords src dest -- ) copy word at src addr to dest addr
    threePopDs
    parkPc
    movl %eax, %edx                   # numWords is now in %edx
    push %edx
    push %edx
    movl %ebx, %esi                   # src is now in %esi
    movl %ecx, %edi                   # dest is in now in %edi
    # Do nothing if src = dest
    cmpl %esi, %edi
    je finishedMove
      # ensure src is within memory
      ensureWordAddressable %ebx
      # ensure src won't overflow and will stay within system memory
      movl %esi, %eax
        imull $WORD_SIZE, %edx        # Note: %edx clobbered
        jo trapMemBounds
        addl %edx, %eax
        jo trapMemBounds
        ensureWordAddressable %eax
      # ensure dest is within memory
      ensureWordAddrWritable %ecx
      pop %edx                        # Note: %edx restored
      # ensure dest won't overflow and will stay within RAM
      movl %edi, %eax
        imull $WORD_SIZE, %edx        # Note: %edx clobbered
        jo trapMemBounds
        addl %edx, %eax
        jo trapMemBounds
        ensureWordAddrWritable %eax
      pop %edx                        # Note: %edx restored
      # copy bytes from source to destination
      movl %edx, %ecx                 # numWords is now in %ecx
        test %ecx, %ecx
        jns ascMove                   # If numWords negative, descending copy
          std                         # Set DF flag (ensures descending copy)
          neg %ecx                    # Use positive counter
          jmp doMove
        ascMove:                      # numWords is positive, ascending copy
          cld                         # Clear DF flag (ensures ascending copy)
        doMove:
        leal memory(,%esi,1), %esi
        leal memory(,%edi,1), %edi
        rep movsl
    finishedMove:
      unparkPc
      next

iLOADB: # ( a -- byte )
    popDs
    movl %eax, %edx
    ensureByteAddressable %edx
    movzbl memory(,%eax,1), %eax    # Retrieve byte starting at specified byte
    pushDs
    next

iRLOADB: # ( a -- a byte )
    peekDs
    movl %eax, %edx
    ensureByteAddressable %edx
    movzbl memory(,%eax,1), %eax    # Retrieve byte starting at specified byte
    pushDs
    next

iLOAD: # ( a -- word )
    popDs
    movl %eax, %edx
    ensureWordAddressable %edx
    movl memory(,%eax,1), %eax      # Retrieve word starting at specified byte
    pushDs
    next

iRLOAD: # ( a -- a word )
    peekDs
    movl %eax, %edx
    ensureWordAddressable %edx
    movl memory(,%eax,1), %eax      # Retrieve word starting at specified byte
    pushDs
    next

iPLOAD: # ( p -- word ) # Like load but assumes address is a pointer
    popDs               #   and therefore loads word from address stored at p
    movl %eax, %edx
    ensureWordAddressable %edx
    movl memory(,%eax,1), %eax      # Retrieve addr starting at specified byte
    movl %eax, %edx
    ensureWordAddressable %edx
    movl memory(,%eax,1), %eax
    pushDs
    next

iRPLOAD: # ( p -- p word ) # Like rload but assumes address is a pointer
    peekDs                 # and therefore loads word from address stored at p
    movl %eax, %edx
    ensureWordAddressable %edx
    movl memory(,%eax,1), %eax      # Retrieve addr starting at specified byte
    movl %eax, %edx
    ensureWordAddressable %edx
    movl memory(,%eax,1), %eax
    pushDs
    next

iPLOADB: # ( p -- p word ) # Like loadb but assumes address is a pointer
    popDs                  # and therefore loads byte from address stored at p
    movl %eax, %edx
    ensureByteAddressable %edx
    movl memory(,%eax,1), %eax      # Retrieve addr starting at specified byte
    movl %eax, %edx
    ensureByteAddressable %edx
    movzbl memory(,%eax,1), %eax    # Retrieve byte starting at addr
    pushDs
    next

iRPLOADB: # ( p -- p word ) # Like rloadb but assumes address is a pointer
    peekDs                 # and therefore loads byte from address stored at p
    movl %eax, %edx
    ensureByteAddressable %edx
    movl memory(,%eax,1), %eax      # Retrieve addr starting at specified byte
    movl %eax, %edx
    ensureByteAddressable %edx
    movzbl memory(,%eax,1), %eax    # Retrieve byte starting at addr
    pushDs
    next

iSTORE: # ( n a -- )
    popDs
    movl %eax, %ebx                 # Address is in %ebx
    popDs                           # Value to store is in %eax
    movl %ebx, %edx
    ensureWordAddrWritable %edx
    movl %eax, memory(,%ebx,1)      # Store word starting at specified byte
    next

iSTOREB: # ( n a -- )
    popDs
    movl %eax, %ebx                 # Address is in %ebx
    popDs                           # Value to store is in %eax
    movl %ebx, %edx
    ensureByteAddrWritable %edx
    movb %al, memory(,%ebx,1)       # Store byte starting at specified byte
    next

iPSTORE: # ( n p -- )   # Like STORE but assumes p is a pointer
    popDs               #   and therefore stores word to address stored at p
    movl %eax, %ebx                 # p is in %ebx
    popDs                           # Value to store is in %eax
    movl %ebx, %edx
    ensureWordAddressable %edx
    movl memory(,%ebx,1), %ebx
    movl %ebx, %edx
    ensureWordAddrWritable %edx
    movl %eax, memory(,%ebx,1)      # Store word starting at specified addr
    next

iPSTOREB: # ( n p -- )   # Like STOREB but assumes p is a pointer
    popDs               #   and therefore stores word to address stored at p
    movl %eax, %ebx                 # p is in %ebx
    popDs                           # Value to store is in %eax
    movl %ebx, %edx
    ensureWordAddressable %edx
    movl memory(,%ebx,1), %ebx
    movl %ebx, %edx
    ensureByteAddrWritable %edx
    movb %al, memory(,%ebx,1)       # Store byte starting at specified addr
    next

# Instruction: iREBOOT
iREBOOT:
    movl $LAST_RESTART_CODE_REBOOT, lastRestartCode
    jmp systemHardReset             # Program requested hard reset

# Instruction: iRESET
iRESET:
    movl $LAST_RESTART_CODE_RESET, lastRestartCode
    jmp systemSoftReset             # Program requested soft reset

iCALL: # (  -- )
  movl %esi, %eax                   # %eax now contains return address
  addl $WORD_SIZE, %eax
  pushRs                            # Return address is now on return stack
  branch

iDCALL: # ( a -- )
  popDs
  movl %eax, %ebx                   # %ebx now contains call address
  # Ensure dynamic call address lies within program memory
    movl %ebx, %edx
    andl $PROG_MEMORY_MASK, %edx
    jnz trapPcOverflow
  movl %esi, %eax                   # %eax now contains return address
  pushRs                            # Return address is now on return stack
  movl %ebx, %esi                   # pc (%esi) now contains call address
  next

iRDCALL: # ( a -- a ) ONLY SAFE FOR NON-CONSUMING FUNCTIONS!
  peekDs
  movl %eax, %ebx                   # %ebx now contains call address
  # Ensure dynamic call address lies within program memory
    movl %ebx, %edx
    andl $PROG_MEMORY_MASK, %edx
    jnz trapPcOverflow
  movl %esi, %eax                   # %eax now contains return address
  pushRs                            # Return address is now on return stack
  movl %ebx, %esi                   # pc (%esi) now contains call address
  next

iEXIT:
    popRs
    # Ensure return address lies within program memory
      movl %eax, %edx
      andl $PROG_MEMORY_MASK, %edx
      jnz trapPcOverflow
    movl %eax, %esi
    next

iSPOP: # ( -- n ) [ n -- ]
  popSs
  pushDs
  next

iSPEEK: # ( -- n ) [ n -- n ]
  peekSs
  pushDs
  next

iSPEEK2: # ( -- n1 n2 ) [ n1 n2 -- n1 n2 ] Note: NOT same as spop spop
  twoPeekSs
  movl %eax, %edx
  movl %ebx, %eax
  movl %edx, %ebx
  twoPushDs
  next

iSPEEK3: # ( -- n1 n2 n3 ) [ n1 n2 n3 -- n1 n2 n3 ] NOT same as spop spop spop
  threePeekSs
  movl %ecx, %edx
  movl %eax, %ecx
  movl %edx, %eax
  threePushDs
  next

iSPEEK4: # ( -- n1 n2 n3 n4 ) [ n1 n2 n3 n4 -- n1 n2 n3 n4 ] NOT like spop x 4
  fourPeekSs
  xchgl %edx, %eax
  xchgl %ecx, %ebx
  fourPushDs
  next

iSPOP2: # ( -- n1 n2 ) [ n1 n2 -- ] Note: NOT same as spop spop
  twoPopSs
  movl %eax, %edx
  movl %ebx, %eax
  movl %edx, %ebx
  twoPushDs
  next

iSPOP3: # ( -- n1 n2 n3 ) [ n1 n2 n3 -- ] Note: NOT same as spop spop spop
  threePopSs
  movl %ecx, %edx
  movl %eax, %ecx
  movl %edx, %eax
  threePushDs
  next

iSPOP4: # ( -- n1 n2 n3 n4 ) [ n1 n2 n3 n4 -- ] Note: NOT like spop x 4
  fourPopSs
  xchgl %edx, %eax
  xchgl %ecx, %ebx
  fourPushDs
  next

iSPUSH: # ( n -- ) [ -- n ]
  popDs
  pushSs
  next

iSPUSH2: # ( n1 n2 -- ) [ -- n1 n2 ] Note: NOT same as spush spush
  twoPopDs
  # Reverse %eax, %ebx 'order'
  movl %eax, %edx
  movl %ebx, %eax
  movl %edx, %ebx
  twoPushSs
  next

iSPUSH3: # ( n1 n2 n3 -- ) [ -- n1 n2 n3 ] Note: NOT same as spush spush spush
  threePopDs
  # Reverse %eax, %ebx, %ecx 'order'
  movl %eax, %edx
  movl %ecx, %eax
  movl %edx, %ecx
  threePushSs
  next

iSPUSH4: # ( n1 n2 n3 n4 -- ) [ -- n1 n2 n3 n4 ] Note: NOT like spush x 4
  fourPopDs
  xchgl %edx, %eax
  xchgl %ecx, %ebx
  fourPushSs
  next

iHOLD: # ( n -- n ) [ -- n ]
  peekDs
  pushSs
  next

iHOLD2: # ( n1 n2 -- n1 n2 ) [ -- n1 n2 ]
  twoPeekDs
  twoPushSs
  next

iHOLD3: # ( n1 n2 n3 -- n1 n2 n3 ) [ -- n1 n2 n3 ]
  threePeekDs
  threePushSs
  next

iHOLD4: # ( n1 n2 n3 n4 -- n1 n2 n3 n4 ) [ -- n1 n2 n3 n4 ]
  fourPeekDs
  fourPushSs
  next

iRPOP: # ( -- n ) { n -- }
  popRs
  pushDs
  next

iRPUSH: # ( n -- ) { -- n }
  popDs
  pushRs
  next

iBRGZ: # ( n -- n )
    peekDs
    cmpl $0x0, %eax
    jle 1f
      branch
    1:
      dontBranch

iBRGEZ: # ( n -- n )
    peekDs
    cmpl $0x0, %eax
    jl 1f
      branch
    1:
      dontBranch

iBRLZ: # ( n -- n )
    peekDs
    cmpl $0x0, %eax
    jge 1f
      branch
    1:
      dontBranch

iBRLEZ: # ( n -- n )
    peekDs
    cmpl $0x0, %eax
    jg 1f
      branch
    1:
      dontBranch

iBRNZ: # ( n -- n )
    peekDs
    test %eax, %eax
    jz 1f
      branch
    1:
      dontBranch

iBRZ: # ( n -- n )
    peekDs
    test %eax, %eax
    jnz 1f
      branch
    1:
      dontBranch

iBRG: # ( n1 n2 -- n1 )
    peekSecondDs
    popDs
    cmpl %eax, %ebx
    jle 1f
      branch
    1:
      dontBranch

iBRGE: # ( n1 n2 -- n1 )
    peekSecondDs
    popDs
    cmpl %eax, %ebx
    jl 1f
      branch
    1:
      dontBranch

iBRL: # ( n1 n2 -- n1 )
    peekSecondDs
    popDs
    cmpl %eax, %ebx
    jge 1f
      branch
    1:
      dontBranch

iBRLE: # ( n1 n2 -- n1 )
    peekSecondDs
    popDs
    cmpl %eax, %ebx
    jg 1f
      branch
    1:
      dontBranch
iBRNE: # ( n1 n2 -- n1 )
    peekSecondDs
    popDs
    cmpl %eax, %ebx
    je 1f
      branch
    1:
      dontBranch

iBRE: # ( n1 n2 -- n1 )
    peekSecondDs
    popDs
    cmpl %eax, %ebx
    jne 1f
      branch
    1:
      dontBranch

iJGZ: # ( n -- )
  popDs
  cmpl $0x0, %eax
  jle 1f
    branch
  1:
    dontBranch

iJGEZ: # ( n -- )
  popDs
  cmpl $0x0, %eax
  jl 1f
    branch
  1:
    dontBranch

iJLZ: # ( n -- )
  popDs
  cmpl $0x0, %eax
  jge 1f
    branch
  1:
    dontBranch

iJLEZ: # ( n -- )
  popDs
  cmpl $0x0, %eax
  jg 1f
    branch
  1:
    dontBranch

iJZ: # ( n -- )
    popDs
    test %eax, %eax
    jnz 1f
      branch
    1:
      dontBranch

iJNZ: # ( n -- )
    popDs
    test %eax, %eax
    jz 1f
      branch
    1:
      dontBranch

iJG: # ( n1 n2 -- )
    twoPopDs
    cmpl %ebx, %eax
    jle 1f
      branch
    1:
      dontBranch

iJGE: # ( n1 n2 -- )
    twoPopDs
    cmpl %ebx, %eax
    jl 1f
      branch
    1:
      dontBranch

iJL: # ( n1 n2 -- )
    twoPopDs
    cmpl %ebx, %eax
    jge 1f
      branch
    1:
      dontBranch

iJLE: # ( n1 n2 -- )
    twoPopDs
    cmpl %ebx, %eax
    jg 1f
      branch
    1:
      dontBranch

iJE: # ( n1 n2 -- )
    twoPopDs
    cmpl %ebx, %eax
    jne 1f
      branch
    1:
      dontBranch

iJNE: # ( n1 n2 -- )
    twoPopDs
    cmpl %ebx, %eax
    je 1f
      branch
    1:
      dontBranch

iJMP: # ( -- )
  branch

iDJMP: # ( a -- )
  popDs                             # %eax now contains jump address
  # Ensure dynamic jump address lies within program memory
    movl %eax, %edx
    andl $PROG_MEMORY_MASK, %edx
    jnz trapPcOverflow
  movl %eax, %esi                   # pc (%esi) now contains jump address
  next

iDEC: # ( n -- n-1 )
  peekDs
  subl $1, %eax
  jo trapMathOverflow
  replaceDs
  next

iDECW: # ( n -- n-WORD_SIZE )
  peekDs
  subl $WORD_SIZE, %eax
  jo trapMathOverflow
  replaceDs
  next

iDEC2W: # ( n -- n-TWO_WORDS_SIZE )
  peekDs
  subl $TWO_WORDS_SIZE, %eax
  jo trapMathOverflow
  replaceDs
  next

iINC: # ( n -- n+1 )
  peekDs
  addl $1, %eax
  jo trapMathOverflow
  replaceDs
  next

iINCW: # ( n -- n+WORD_SIZE )
  peekDs
  addl $WORD_SIZE, %eax
  jo trapMathOverflow
  replaceDs
  next

iINC2W: # ( n -- n+TWO_WORDS_SIZE )
  peekDs
  addl $TWO_WORDS_SIZE, %eax
  jo trapMathOverflow
  replaceDs
  next

iDROP: # ( n -- )
  dropDs
  next

iDROP2: # ( n1 n2 -- )
  twoDropDs
  next

iDROP3: # ( n1 n2 n3 -- )
  threeDropDs
  next

iDROP4: # ( n1 n2 n3 n4 -- )
  fourDropDs
  next

iDUP: # ( n -- n n )
  peekDs
  pushDs
  next

iDUP2: # ( n1 n2 -- n1 n2 n1 n2 )
  twoPeekDs
  twoPushDs
  next

iDUP3: # ( n1 n2 n3 -- n1 n2 n3 n1 n2 n3 )
  threePeekDs
  threePushDs
  next

iDUP4: # ( n1 n2 n3 n4 -- n1 n2 n3 n4 n1 n2 n3 n4 )
  fourPeekDs
  fourPushDs
  next

iSWAP: # ( n1 n2 -- n2 n1 )
  twoPeekDs
  xchgl %ebx, %eax
  twoReplaceDs
  next

iOVER: # ( n1 n2 -- n1 n2 n1 )
  peekSecondDs
  movl %ebx, %eax
  pushDs
  next

iNIP: # ( n1 n2 -- n2 )
  movl dsp, %edi
  cmpl $DS_HAS_TWO, %edi
  jg trapDsUnderflow
  peekDs
  addl $WORD_SIZE,  %edi
  movl %eax, (%edi)
  movl %edi, dsp
  next

iTUCK: # ( n1 n2 -- n2 n1 n2 )
  twoPeekDs
  xchgl %ebx, %eax
  twoReplaceDs
  peekSecondDs
  movl %ebx, %eax
  pushDs
  next

iROT: # ( n1 n2 n3 -- n2 n3 n1 )
  threePeekDs
  threeReplaceDs
  next

iLEAP: # ( n1 n2 n3 -- n1 n2 n3 n1 )
  peekThirdDs
  movl %ecx, %eax
  pushDs
  next

iTOR: # ( n1 n2 n3 -- n3 n1 n2 )
  threePeekDs
  threeRReplaceDs
  next

iHALT: # ( -- )
  jmp exitSuccess

iWALL: # ( -- )
  jmp trapWall

iDATA: # ( -- )
  jmp trapData

iLIT: # ( -- n )
  wordAtPc
  pushDs
  incPc
  next

iAND: # ( n1 n2 -- n1&n2 )
  twoPopDs
  andl %ebx, %eax
  pushDs
  next

iRAND: # ( n1 n2 -- n1 n2 n2&n1 )
  twoPeekDs
  andl %ebx, %eax
  pushDs
  next

iOR: # ( n1 n2 -- n1|n2 )
  twoPopDs
  orl %ebx, %eax
  pushDs
  next

iROR: # ( n1 n2 -- n1 n2 n2|n1 )
  twoPeekDs
  orl %ebx, %eax
  pushDs
  next

iXOR: # ( n1 n2 -- n1^n2 )
  twoPopDs
  xorl %ebx, %eax
  pushDs
  next

iRXOR: # ( n1 n2 -- n1 n2 n2^n1 )
  twoPeekDs
  xorl %ebx, %eax
  pushDs
  next

iADD: # ( n1 n2 -- n1+n2 )
  twoPopDs
  addl %ebx, %eax
  jo trapMathOverflow
  pushDs
  next

iRADD: # ( n1 n2 -- n1 n2+n1 )
  twoPeekDs
  addl %ebx, %eax
  jo trapMathOverflow
  twoReplaceDs
  next

iDIV: # ( n1 n2 -- n1/n2 )
  twoPopDs
  # Do not allow division by zero
  test %ebx, %ebx
  je trapDivideByZero
  # Do not allow division of NEG_INT_MAX by -1
  cmpl $NEG_INT_MAX, %eax
  jne 1f
    cmpl $-1, %ebx
    je trapMathOverflow
  1:
  cdq                    # MUST widen %eax here to %edx:eax or (neg) div wrong
  idivl %ebx               # %edx:eax is the implied dividend
  pushDs
  next

iRDIV: # ( n1 n2 -- n1 n2/n1 )
  twoPeekDs
  # Do not allow division by zero
  test %ebx, %ebx # n1
  je trapDivideByZero
  # Do not allow division of NEG_INT_MAX by -1
  cmpl $NEG_INT_MAX, %eax # n2
  jne 1f
    cmpl $-1, %ebx
    je trapMathOverflow
  1:
  cdq                    # MUST widen %eax here to %edx:eax or (neg) div wrong
  idivl %ebx               # %edx:eax is the implied dividend
  twoReplaceDs
  next

iMOD: # ( n1 n2 -- n1%n2 )
  twoPopDs
  # Do not allow division by zero
  test %ebx, %ebx
  je trapDivideByZero
  # Do not allow division of NEG_INT_MAX by -1
  cmpl $NEG_INT_MAX, %eax
  jne 1f
    cmpl $-1, %ebx
    je trapMathOverflow
  1:
  cdq                    # MUST widen %eax here to %edx:eax or (neg) div wrong
  idivl %ebx               # %edx:eax is the implied dividend
  movl %edx, %eax
  pushDs
  next

iRMOD: # ( n1 n2 -- n1 n2%n1 )
  twoPeekDs
  # Do not allow division by zero
  test %ebx, %ebx
  je trapDivideByZero
  # Do not allow division of NEG_INT_MAX by -1
  cmpl $NEG_INT_MAX, %eax
  jne 1f
    cmpl $-1, %ebx
    je trapMathOverflow
  1:
  cdq                    # MUST widen %eax here to %edx:eax or (neg) div wrong
  idivl %ebx               # %edx:eax is the implied dividend
  movl %edx, %eax
  twoReplaceDs
  next

iDIVMOD: # ( n1 n2 -- n1/n2 n1%n2 )
  twoPopDs
  # Do not allow division by zero
  test %ebx, %ebx
  je trapDivideByZero
  # Do not allow division of NEG_INT_MAX by -1
  cmpl $NEG_INT_MAX, %eax
  jne 1f
    cmpl $-1, %ebx
    je trapMathOverflow
  1:
  cdq                    # MUST widen %eax here to %edx:eax or (neg) div wrong
  idivl %ebx               # %edx:eax is the implied dividend
  pushDs
  movl %edx, %eax
  pushDs
  next

iRDIVMOD: # ( n1 n2 -- n1 n2/n1 n2%n1 )
  twoPeekDs
  # Do not allow division by zero
  test %ebx, %ebx
  je trapDivideByZero
  # Do not allow division of NEG_INT_MAX by -1
  cmpl $NEG_INT_MAX, %eax
  jne 1f
    cmpl $-1, %ebx
    je trapMathOverflow
  1:
  cdq                    # MUST widen %eax here to %edx:eax or (neg) div wrong
  idivl %ebx               # %edx:eax is the implied dividend
  replaceDs
  movl %edx, %eax
  pushDs
  next

iMUL: # ( n1 n2 -- n1*n2 )
  twoPopDs
  imull %ebx, %eax
  jo trapMathOverflow
  pushDs
  next

iRMUL: # ( n1 n2 -- n1 n2*n1 )
  twoPeekDs
  imull %ebx, %eax
  jo trapMathOverflow
  twoReplaceDs
  next

iSUB: # ( n1 n2 -- n1-n2 )
  twoPopDs
  subl %ebx, %eax
  jo trapMathOverflow
  pushDs
  next

iRSUB: # ( n1 n2 -- n1 n2-n1 )
  twoPeekDs
  subl %ebx, %eax
  jo trapMathOverflow
  twoReplaceDs
  next

iSHL: # ( n1 n2 -- n1<<n2 )
  popDs
  cmp $0x1f, %eax
  jg trapXsBitshift
  movl %eax, %ecx
  popDs
  shll %cl, %eax
  pushDs
  next

iRSHL: # ( n1 n2 -- n1 n2 n1<<n2 )
  twoPeekDs
  cmp $0x1f, %eax
  jg trapXsBitshift
  movl %eax, %ecx
  movl %ebx, %eax
  shll %cl, %eax
  pushDs
  next

iSHR: # ( n1 n2 -- n1>>n2 )
  popDs
  cmp $0x1f, %eax
  jg trapXsBitshift
  movl %eax, %ecx
  popDs
  shrl %cl, %eax
  pushDs
  next

iRSHR: # ( n1 n2 -- n1 n2 n1>>n2 )
  twoPeekDs
  cmp $0x1f, %eax
  jg trapXsBitshift
  movl %eax, %ecx
  movl %ebx, %eax
  shrl %cl, %eax
  pushDs
  next

iNEG: # ( n1 -- n1*-1 )
  popDs
  # Do not allow negation of NEG_INT_MAX
  cmpl $NEG_INT_MAX, %eax
  je trapMathOverflow
  neg %eax
  pushDs
  next

iABS: # ( n1 -- |n1| )
  peekDs
  test %eax, %eax
  jns 1f
    # Do not allow negation of NEG_INT_MAX
    cmpl $NEG_INT_MAX, %eax
    je trapMathOverflow
    neg %eax
    replaceDs
  1:
    next

# ============================================================================
#           OPCODE TABLE AND VECTOR TABLE FOR VM INSTRUCTION SET
# ============================================================================
opcodeTable:
    haltOpcode: # (1)
      .equ WALL, 0 # WALL must be zero
    complexOpcodes: # (37)
      .equ LIT, 1
      .equ CALL, 2
      .equ JMP, 3
      .equ BRGZ, 4
      .equ BRGEZ, 5
      .equ BRZ, 6
      .equ BRNZ, 7
      .equ BRLEZ, 8
      .equ BRLZ, 9
      .equ BRG, 10
      .equ BRGE, 11
      .equ BRE, 12
      .equ BRNE, 13
      .equ BRLE, 14
      .equ BRL, 15
      .equ JGZ, 16
      .equ JGEZ, 17
      .equ JZ, 18
      .equ JNZ, 19
      .equ JLEZ, 20
      .equ JLZ, 21
      .equ JG, 22
      .equ JGE, 23
      .equ JE, 24
      .equ JNE, 25
      .equ JLE, 26
      .equ JL, 27
      .equ READOR, 28
      .equ WRITOR, 29
      .equ TRACOR, 30
      .equ GETOR, 31
      .equ PUTOR, 32
      .equ READORB, 33
      .equ WRITORB, 34
      .equ TRACORB, 35
      .equ GETORB, 36
      .equ PUTORB, 37
    complexOpcodesEnd:
    simpleOpcodes: # (111)
      .equ EXIT, 145
      .equ DCALL, 146
      .equ RDCALL, 147
      .equ DJMP, 148
      .equ SWAP, 149
      .equ OVER, 150
      .equ ROT, 151
      .equ TOR, 152
      .equ LEAP, 153
      .equ NIP, 154
      .equ TUCK, 155
      .equ REV, 156
      .equ RPUSH, 157
      .equ RPOP, 158
      .equ DROP, 159
      .equ DROP2, 160
      .equ DROP3, 161
      .equ DROP4, 162
      .equ DUP, 163
      .equ DUP2, 164
      .equ DUP3, 165
      .equ DUP4, 166
      .equ HOLD, 167
      .equ HOLD2, 168
      .equ HOLD3, 169
      .equ HOLD4, 170
      .equ SPEEK, 171
      .equ SPEEK2, 172
      .equ SPEEK3, 173
      .equ SPEEK4, 174
      .equ SPUSH, 175
      .equ SPUSH2, 176
      .equ SPUSH3, 177
      .equ SPUSH4, 178
      .equ SPOP, 179
      .equ SPOP2, 180
      .equ SPOP3, 181
      .equ SPOP4, 182
      .equ DEC, 183
      .equ DECW, 184
      .equ DEC2W, 185
      .equ INC, 186
      .equ INCW, 187
      .equ INC2W, 188
      .equ LOAD, 189
      .equ STORE, 190
      .equ RLOAD, 191
      .equ LOADB, 192
      .equ STOREB, 193
      .equ RLOADB, 194
      .equ PLOAD, 195
      .equ PSTORE, 196
      .equ RPLOAD, 197
      .equ PLOADB, 198
      .equ PSTOREB, 199
      .equ RPLOADB, 200
      .equ ADD, 201
      .equ SUB, 202
      .equ MUL, 203
      .equ DIV, 204
      .equ MOD, 205
      .equ DIVMOD, 206
      .equ RADD, 207
      .equ RSUB, 208
      .equ RMUL, 209
      .equ RDIV, 210
      .equ RMOD, 211
      .equ RDIVMOD, 212
      .equ NEG, 213
      .equ ABS, 214
      .equ AND, 215
      .equ OR, 216
      .equ XOR, 217
      .equ RAND, 218
      .equ ROR, 219
      .equ RXOR, 220
      .equ SHL, 221
      .equ SHR, 222
      .equ RSHL, 223
      .equ RSHR, 224
      .equ MOVE, 225
      .equ FILL, 226
      .equ FIND, 227
      .equ MATCH, 228
      .equ MOVEB, 229
      .equ FILLB, 230
      .equ FINDB, 231
      .equ MATCHB, 232
      .equ HOMIO, 233
      .equ RCHAN, 234
      .equ WCHAN, 235
      .equ GCHAN, 236
      .equ PCHAN, 237
      .equ ECODE, 238
      .equ RCODE, 239
      .equ ROM, 240
      .equ RAM, 241
      .equ MAP, 242
      .equ STDBLK, 243
      .equ DS, 244
      .equ SS, 245
      .equ RS, 246
      .equ DSN, 247
      .equ SSN, 248
      .equ RSN, 249
      .equ TRON, 250
      .equ TROFF, 251
      .equ RESET, 252
      .equ REBOOT, 253
      .equ HALT, 254
      .equ DATA, 255
    simpleOpcodesEnd:
opcodeTableEnd:
vectorTable: # Must be in same order as opcodeTable
  .long iWALL               # WALL must be zero
  .long iLIT                # 37 complex instructions
  .long iCALL
  .long iJMP
  .long iBRGZ
  .long iBRGEZ
  .long iBRZ
  .long iBRNZ
  .long iBRLEZ
  .long iBRLZ
  .long iBRG
  .long iBRGE
  .long iBRE
  .long iBRNE
  .long iBRLE
  .long iBRL
  .long iJGZ
  .long iJGEZ
  .long iJZ
  .long iJNZ
  .long iJLEZ
  .long iJLZ
  .long iJG
  .long iJGE
  .long iJE
  .long iJNE
  .long iJLE
  .long iJL
  .long iREADOR
  .long iWRITOR
  .long iTRACOR
  .long iGETOR
  .long iPUTOR
  .long iREADORB
  .long iWRITORB
  .long iTRACORB
  .long iGETORB
  .long iPUTORB # What follows next is space for future extensibility (107)
  .long iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE
  .long iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE
  .long iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE
  .long iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE
  .long iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE
  .long iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE
  .long iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE
  .long iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE
  .long iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE
  .long iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE
  .long iNONE, iNONE, iNONE, iNONE, iNONE, iNONE, iNONE
  .long iEXIT               # 111 simple instructions
  .long iDCALL
  .long iRDCALL
  .long iDJMP
  .long iSWAP
  .long iOVER
  .long iROT
  .long iTOR
  .long iLEAP
  .long iNIP
  .long iTUCK
  .long iREV
  .long iRPUSH
  .long iRPOP
  .long iDROP
  .long iDROP2
  .long iDROP3
  .long iDROP4
  .long iDUP
  .long iDUP2
  .long iDUP3
  .long iDUP4
  .long iHOLD
  .long iHOLD2
  .long iHOLD3
  .long iHOLD4
  .long iSPEEK
  .long iSPEEK2
  .long iSPEEK3
  .long iSPEEK4
  .long iSPUSH
  .long iSPUSH2
  .long iSPUSH3
  .long iSPUSH4
  .long iSPOP
  .long iSPOP2
  .long iSPOP3
  .long iSPOP4
  .long iDEC
  .long iDECW
  .long iDEC2W
  .long iINC
  .long iINCW
  .long iINC2W
  .long iLOAD
  .long iSTORE
  .long iRLOAD
  .long iLOADB
  .long iSTOREB
  .long iRLOADB
  .long iPLOAD
  .long iPSTORE
  .long iRPLOAD
  .long iPLOADB
  .long iPSTOREB
  .long iRPLOADB
  .long iADD
  .long iSUB
  .long iMUL
  .long iDIV
  .long iMOD
  .long iDIVMOD
  .long iRADD
  .long iRSUB
  .long iRMUL
  .long iRDIV
  .long iRMOD
  .long iRDIVMOD
  .long iNEG
  .long iABS
  .long iAND
  .long iOR
  .long iXOR
  .long iRAND
  .long iROR
  .long iRXOR
  .long iSHL
  .long iSHR
  .long iRSHL
  .long iRSHR
  .long iMOVE
  .long iFILL
  .long iFIND
  .long iMATCH
  .long iMOVEB
  .long iFILLB
  .long iFINDB
  .long iMATCHB
  .long iHOMIO
  .long iRCHAN
  .long iWCHAN
  .long iGCHAN
  .long iPCHAN
  .long iECODE
  .long iRCODE
  .long iROM
  .long iRAM
  .long iMAP
  .long iSTDBLK
  .long iDS
  .long iSS
  .long iRS
  .long iDSN
  .long iSSN
  .long iRSN
  .long iTRON
  .long iTROFF
  .long iRESET
  .long iREBOOT
  .long iHALT
  .long iDATA
vectorTableEnd:
.ifdef TRON_ENABLED
  traceTable: # Must be in same order as opcodeTable
    .ascii "===     "
    .ascii "lit     "
    .ascii "call    "
    .ascii "go      "
    .ascii "go[>0]  "
    .ascii "go[>=0] "
    .ascii "go[==0] "
    .ascii "go[!=0] "
    .ascii "go[<=0] "
    .ascii "go[<0]  "
    .ascii "go[>]   "
    .ascii "go[>=]  "
    .ascii "go[==]  "
    .ascii "go[!=]  "
    .ascii "go[<=]  "
    .ascii "go[<]   "
    .ascii "go>0    "
    .ascii "go>=0   "
    .ascii "go==0   "
    .ascii "go!=0   "
    .ascii "go<=0   "
    .ascii "go<0    "
    .ascii "go>     "
    .ascii "go>=    "
    .ascii "go==    "
    .ascii "go!=    "
    .ascii "go<=    "
    .ascii "go<     "
    .ascii "reador  "
    .ascii "writor  "
    .ascii "tracor  "
    .ascii "getor   "
    .ascii "putor   "
    .ascii "readorb "
    .ascii "writorb "
    .ascii "tracorb "
    .ascii "getorb  "
    .ascii "putorb  "
    .ascii "        " # Start of 107 empty fillers in blocks of 10...
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "           # 10 empty fillers
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "           # 10 empty fillers
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "           # 10 empty fillers
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "           # 10 empty fillers
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "           # 10 empty fillers
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "           # 10 empty fillers
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "           # 10 empty fillers
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "           # 10 empty fillers
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "           # 10 empty fillers
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "           #  7 empty fillers
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        "
    .ascii "        " # End of 107 empty fillers in blocks of 10
    .ascii "ret     "
    .ascii "invoke  "
    .ascii "[invoke]"
    .ascii "fly     "
    .ascii "swap    "
    .ascii "over    "
    .ascii "rot     "
    .ascii "tor     "
    .ascii "leap    "
    .ascii "nip     "
    .ascii "tuck    "
    .ascii "rev     "
    .ascii "rpush   "
    .ascii "rpop    "
    .ascii "drop    "
    .ascii "drop2   "
    .ascii "drop3   "
    .ascii "drop4   "
    .ascii "dup     "
    .ascii "dup2    "
    .ascii "dup3    "
    .ascii "dup4    "
    .ascii "hold    "
    .ascii "hold2   "
    .ascii "hold3   "
    .ascii "hold4   "
    .ascii "speek   "
    .ascii "speek2  "
    .ascii "speek3  "
    .ascii "speek4  "
    .ascii "spush   "
    .ascii "spush2  "
    .ascii "spush3  "
    .ascii "spush4  "
    .ascii "spop    "
    .ascii "spop2   "
    .ascii "spop3   "
    .ascii "spop4   "
    .ascii "dec     "
    .ascii "decw    "
    .ascii "dec2w   "
    .ascii "inc     "
    .ascii "incw    "
    .ascii "inc2w   "
    .ascii "@       "
    .ascii "!       "
    .ascii "[@]     "
    .ascii "@b      "
    .ascii "!b      "
    .ascii "[@b]    "
    .ascii "@@      "
    .ascii "@!      "
    .ascii "[@@]    "
    .ascii "@@b     "
    .ascii "@!b     "
    .ascii "[@@b]   "
    .ascii "+       "
    .ascii "-       "
    .ascii "*       "
    .ascii "/       "
    .ascii "%       "
    .ascii "/%      "
    .ascii "[+]     "
    .ascii "[-]     "
    .ascii "[*]     "
    .ascii "[/]     "
    .ascii "[%]     "
    .ascii "[/%]    "
    .ascii "neg     "
    .ascii "abs     "
    .ascii "&       "
    .ascii "|       "
    .ascii "^       "
    .ascii "[&]     "
    .ascii "[|]     "
    .ascii "[^]     "
    .ascii "<<      "
    .ascii ">>      "
    .ascii "[<<]    "
    .ascii "[>>]    "
    .ascii "move    "
    .ascii "fill    "
    .ascii "find    "
    .ascii "match   "
    .ascii "moveb   "
    .ascii "fillb   "
    .ascii "findb   "
    .ascii "matchb  "
    .ascii "homio   "
    .ascii "rchan   "
    .ascii "wchan   "
    .ascii "gchan   "
    .ascii "pchan   "
    .ascii "ecode?  "
    .ascii "rcode?  "
    .ascii "rom?    "
    .ascii "ram?    "
    .ascii "map?    "
    .ascii "stdblk? "
    .ascii "ds?     "
    .ascii "ss?     "
    .ascii "rs?     "
    .ascii "dsn?    "
    .ascii "ssn?    "
    .ascii "rsn?    "
    .ascii "tron    "
    .ascii "troff   "
    .ascii "reset   "
    .ascii "reboot  "
    .ascii "halt    "
    .ascii "data    "
  traceTableEnd:
.endif # .ifdef TRON_ENABLED

# ============================================================================
#                           PRIVATE SERVICES
# ============================================================================

  _openFile: # Pass in address of asciz filename in %ebx
    movl $5, %eax                   # Linux call ID for open
    movl $02, %ecx                  # Linux file mode code for read/write
    movl $0644, %edx                # Linux 644 permissions (-rw-r--r--)
    int $0x80                       # Linux interrupt for system call
    ret # Returns file handle in %eax or negative if open failed

  _openFileRead: # Pass in address of asciz filename in %ebx
    movl $5, %eax                   # Linux call ID for open
    movl $00, %ecx                  # Linux file mode code for read
    movl $0444, %edx                # Linux 444 permissions (-r--r--r--)
    int $0x80                       # Linux interrupt for system call
    ret # Returns file handle in %eax or negative if open failed

  # Note: Linux file access codes 0=read, 1=write, 2=read/write (not trunc)
  #   See /usr/include/asm-generic/fcntl.h  03101 = write only, trunc, create
  #   02101 = write only, append, create

  _openFileAppend: # Pass in address of asciz filename in %ebx
    movl $5, %eax                   # Linux call ID for open
    movl $02101, %ecx                  # Linux file mode code for write
    movl $0644, %edx                # Linux 644 permissions (-rw--r--r-)
    int $0x80                       # Linux interrupt for system call
    ret # Returns file handle in %eax or negative if open failed

  _openAndTruncFile: # Pass in address of asciz filename in %ebx
    movl $5, %eax                   # Linux call ID for open
    movl $03101, %ecx               # Linux file mode code for write
    movl $0644, %edx                # Linux 644 permissions (-rw-r--r--)
    int $0x80                       # Linux interrupt for system call
    ret # Returns file handle in %eax or negative if open failed

  _closeFile: # Pass in Linux file handle for file to close in %ebx
    movl $6, %eax                   # Linux call ID for close
    int $0x80                       # Linux interrupt for system call
    ret # Returns negative in %eax if close failed

  _openStdtrc: # Open stdtrc
    movl $stdtrcFilename, %ebx
      # The next line should normally be: call _openAndTruncFile 
      #   except that for fvm16-16MB-sr-append for fvmtest it
      #   should be: call _openFileAppend
      call _openAndTruncFile
      test %eax, %eax               # Will be negative if open failed
      js trapCantOpenStdtrc         # Trap if open failed
      movl %eax, stdtrcHandle       # Linux file handle is now in stdtrcHandle
    ret

  _closeStdtrc: # Close stdtrc
    movl stdtrcHandle, %ebx
      call _closeFile
      test %eax, %eax               # Will be negative if close failed
      js trapCantCloseStdtrc        # Trap if close failed
    ret

  _openStdexp: # Open stdexp
    movl $stdexpFilename, %ebx
      call _openAndTruncFile
      test %eax, %eax               # Will be negative if open failed
      js trapCantOpenStdexp         # Trap if open failed
      movl %eax, stdexpHandle       # Linux file handle is now in stdexpHandle
    ret

  _openStdimp: # Open stdimp
    movl $stdimpFilename, %ebx
      call _openFileRead
      test %eax, %eax               # Will be negative if open failed
      js trapCantOpenStdimp         # Trap if open failed
      movl %eax, stdimpHandle       # Linux file handle is now in stdimpHandle
    ret

  _closeStdimp: # Close stdimp
    movl stdimpHandle, %ebx
      call _closeFile
      test %eax, %eax               # Will be negative if close failed
      js trapCantCloseStdimp        # Trap if close failed
    ret

  _closeStdexp: # Close stdexp
    movl stdexpHandle, %ebx
      call _closeFile
      test %eax, %eax               # Will be negative if close failed
      js trapCantCloseStdexp        # Trap if close failed
    ret

  _openRom:
    movl $romFilename, %ebx
      call _openFileRead
      test %eax, %eax               # Will be negative if open failed
      js trapCantOpenRom            # Trap if open failed
      movl %eax, romHandle          # Linux file handle is now in romHandle
    ret

  _closeRom:
    movl romHandle, %ebx
      call _closeFile
      test %eax, %eax               # Will be negative if close failed
      js trapCantCloseRom           # Trap if close failed
    ret

  _openStdblk:
    movl $stdblkFilename, %ebx
      call _openFile
      test %eax, %eax               # Will be negative if open failed
      js trapCantOpenStdblk         # Trap if open failed
      movl %eax, stdblkHandle       # Linux file handle is now in stdblkHandle
    ret

  _closeStdblk:
    movl stdblkHandle, %ebx
      call _closeFile
      test %eax, %eax               # Will be negative if close failed
      js trapCantCloseStdblk        # Trap if close failed
    ret
# ============================================================================
#                             RESET POINTS
# ============================================================================
systemHardReset:  # Wipe entire FVM memory, stacks and variables ----
  leal sMark, %edx                  # Address of first FVM variable (sMark)
  leal eMark, %eax                  # Address of marker for end of FVM memory
  subl %edx, %eax                   # Total FVM memory space excluding eMark
  addl $WORD_SIZE, %eax             # Total FVM variable space in bytes
  movl %eax, %ecx
  xorl %eax, %eax
  movl $-0x1, %edx                  # Offset by -1
  hardResetZeroFill:                # Zero bytes from sMark to eMark inclusive
    movb %al, sMark(%edx,%ecx,1)    # Note: this could be done faster in words
    loopl hardResetZeroFill
  jmp systemInitDevices

systemSoftReset:  # Wipe FVM stacks and variables but not memory ----
  leal sMark, %edx                  # Address of first FVM variable (sMark)
  leal memory, %eax                 # Address of last normal FVM variable
  subl %edx, %eax                   # Total FVM variable space
  movl %eax, %ecx
  xorl %eax, %eax
  movl $-0x1, %edx                  # Offset by -1
  softResetZeroFill:                # Zero bytes from and including sMark
    movb %al, sMark(%edx,%ecx,1)    #   to start of memory
    loopl softResetZeroFill         # Note: this could be done faster in words
  jmp systemInitDevices

# ============================================================================
#                              ENTRY POINT
# ============================================================================
.globl _start
_start:
    jmp systemHardReset
systemInitDevices:
    call _openStdtrc
    call _openStdblk
    call _openStdexp
    call _openStdimp
    setIOdefaults

# ----------------------------------------------------------------------------
# Uncomment systemCopyProgram to run the example program hardcoded above
# (see "EXAMPLE OF INDIRECT THREADED PROGRAM" section above).
# This is an (unusual) alternative to systemLoadProgram below.
# You must of course also comment out systemLoadProgram.
/*
systemCopyProgram:   # Copy program into system memory
    movl $PROGRAM_SIZE, %ecx
      shr $HALF_WORD_SIZE, %ecx       # Number of words to copy
      cld                             # Clear DF flag (ensures ascending copy)
      leal itProg, %esi
      leal memory, %edi
      rep movsl
*/
# ----------------------------------------------------------------------------

# ----------------------------------------------------------------------------
# Uncomment systemLoadProgram to load program from ROM file (as usual).
# This is an alternative to (the much more unusual) systemCopyProgram above.
# You must of course also comment out systemCopyProgram.
#/*
systemLoadProgram:
    call _openRom
      movl $ROM_SIZE_WDS, %esi
      leal memory, %edi
    1:
        movl romHandle, %ebx        # Linux file handle for ROM file
        movl $readBuf, %ecx         # Tiny input buffer
        movl $WORD_SIZE, %edx       # Read 1 word only
        movl $0x3, %eax             # Linux call ID for read
        int $0x80                   # Linux interrupt for system call
        testl %eax, %eax            # Unless error, is num of bytes read
          js trapCantReadRom        # Read failed for some reason
          jz 2f                     # Read returned zero bytes
          movl readBuf, %eax
      movl %eax, (%edi)
      addl $WORD_SIZE, %edi
      decl %esi
      cmpl $0, %esi
      jg 1b
    2:
    call _closeRom
#*/
# ----------------------------------------------------------------------------

systemInitCore:
    movl $DS_EMPTY, dsp             # Initialize data stack pointer
    movl $RS_EMPTY, rsp             # Initialize return stack pointer
    movl $SS_EMPTY, ssp             # Initialize software stack pointer
    movl $0, %esi                   # Set program counter to start of program
      next                          # Begin Freeputer program execution

# ============================================================================
#                              SYSTEM RESET
# ============================================================================
systemReset:
  movl %ebx, lastExitCode     # Save as closeStdblk clobbers %ebx
  call _closeStdblk           # Close the standard block device
  call _closeStdexp           # Close stdexp
  call _closeStdimp           # Close stdimp
  call _closeStdtrc           # Close stdtrc
  movl lastExitCode, %ebx     # Restore exit code into %ebx
  # The next line should normally be the uncommented one for most FVMs
  jmp exitFail             # Uncomment for exit with specific failure code
# jmp exitFailGeneric      # Uncomment for exit with generic failure code
  # The next line should be uncommented for fvm16-16MB-sr-append for fvmtest
#  jmp systemSoftReset      # Uncomment for soft reset
# jmp systemHardReset      # Uncomment for hard reset

# ============================================================================
#                              EXIT POINTS
# ============================================================================
exitSuccess:
  movl $0x0, %ebx             # exitCode for success (Linux standard)
  jmp systemExit
exitFail:
  cmpl $0x0, %ebx
  jne systemExit
  exitFailGeneric:
    movl $0x1, %ebx           # exitCode for failure (Linux standard)
    jmp systemExit
systemExit:                   # Exit to Linux using exitCode in %ebx
  movl $0x1, %eax             # Linux call ID for exit
  int $0x80                   # Linux interrupt for system call

# ============================================================================
#                              EXIT TRACING
# ============================================================================
# Send an error message (whose address is in %eax) to stdtrc
# along with information regarding current program state.
# WARNINGS: Do NOT call this if the Freeputer program is not yet running or
#            has fully finished running, since the attempt to trace program
#            state would result in meaningless behaviour.
#           Do NOT call this during VM startup or VM shutdown.
#           Do NOT call when PC is out of bounds (would cause segfault).
#           Do NOT call this until and unless stdtrc is working.
#           Don't call this when %esi (normally the PC) does not hold the PC.
traceExit:
  call traceStr18
  call traceNewline
.ifdef TRON_ENABLED
  leal msgBefore, %eax
  call traceStr18
  call traceNewline
  call traceInfo
  call traceStacks
.endif # .ifdef TRON_ENABLED
  ret

# Send the error message whose address is in %eax to stdtrc.
# WARNING: Do NOT call this until and unless stdtrc is working
traceExitMsg:
  call traceStr18
  call traceNewline
.ifdef TRON_ENABLED
    # We show two possible values for the approximate position
    # of the program counter. Depending on how and when the trap occurred
    # which brought us here it can be difficult to display the PC value.
    # Thus this shows something like: PC 01000008 or 0000a470
    movl $'P', %eax
    call traceChar
    movl $'C', %eax
    call traceChar
    call traceSpc
    movl %esi, %eax     # Trace pc
    call traceWordSpc
    movl $'o', %eax
    call traceChar
    movl $'r', %eax
    call traceChar
    call traceSpc
    unparkPC
    movl %esi, %eax
    call traceWordSpc
    call traceNewline
.endif # .ifdef TRON_ENABLED
  ret

# ============================================================================
#                                 TRAPS
# ============================================================================

# ----------------------------------------------------------------------------
#                          TRAPS: ILLEGAL PROGRAM FLOW
# ----------------------------------------------------------------------------
trapWall:
  movl $1, %ebx
  leal msgTrapWall, %eax
  call traceExit
  jmp systemReset
trapData:
  movl $2, %ebx
  leal msgTrapData, %eax
  call traceExit
  jmp systemReset
trapPcOverflow:
  movl $3, %ebx
  leal msgTrapPcOverflow, %eax
  call traceExitMsg
  jmp systemReset
# ----------------------------------------------------------------------------
#                          TRAPS: ILLEGAL OPCODES
# ----------------------------------------------------------------------------
iNONE:
trapIllegalOpcode:
  movl $11, %ebx
  leal msgTrapIllegalOpcode, %eax
  call traceExit
  jmp systemReset
# ----------------------------------------------------------------------------
#                          TRAPS: ILLEGAL MATHEMATICAL OPERATIONS
# ----------------------------------------------------------------------------
trapMathOverflow:
  movl $21, %ebx
  leal msgTrapMathOverflow, %eax
  call traceExit
  jmp systemReset
trapDivideByZero:
  movl $22, %ebx
  leal msgTrapDivideByZero, %eax
  call traceExit
  jmp systemReset
trapXsBitshift:
  movl $23, %ebx
  leal msgTrapXsBitshift, %eax
  call traceExit
  jmp systemReset
# ----------------------------------------------------------------------------
#                          TRAPS: ILLEGAL STACK OPERATIONS
# ----------------------------------------------------------------------------
trapDsUnderflow:
  movl $31, %ebx
  leal msgTrapDsUnderflow, %eax
  call traceExit
  jmp systemReset
trapDsOverflow:
  movl $32, %ebx
  leal msgTrapDsOverflow, %eax
  call traceExit
  jmp systemReset
trapRsUnderflow:
  movl $33, %ebx
  leal msgTrapRsUnderflow, %eax
  call traceExit
  jmp systemReset
trapRsOverflow:
  movl $34, %ebx
  leal msgTrapRsOverflow, %eax
  call traceExit
  jmp systemReset
trapSsUnderflow:
  movl $35, %ebx
  leal msgTrapSsUnderflow, %eax
  call traceExit
  jmp systemReset
trapSsOverflow:
  movl $36, %ebx
  leal msgTrapSsOverflow, %eax
  call traceExit
  jmp systemReset
# ----------------------------------------------------------------------------
#                          TRAPS: ILLEGAL MEMORY ACCESS
# ----------------------------------------------------------------------------
trapMemBounds:
  movl $41, %ebx
  leal msgTrapMemBounds, %eax
  call traceExitMsg
  jmp systemReset
trapRAMBounds:
  movl $42, %ebx
  leal msgTrapRAMBounds, %eax
  call traceExitMsg
  jmp systemReset
# ----------------------------------------------------------------------------
#                          TRAPS: ROM
# ----------------------------------------------------------------------------
# Note: a ROM file ('rom.fp') can be created using a Freelang compiler
trapCantOpenRom:
  movl $51, %ebx
  leal msgTrapCantOpenRom, %eax
  call traceExitMsg
  jmp exitFail
trapCantCloseRom:
  movl $52, %ebx
  leal msgTrapCantCloseRom, %eax
  call traceExitMsg
  jmp exitFail
trapCantReadRom:
  movl $53, %ebx
  leal msgTrapCantReadRom, %eax
  call traceExitMsg
  jmp exitFail
# ----------------------------------------------------------------------------
#                          TRAPS: STDBLK
# ----------------------------------------------------------------------------
# Note: a suitable zero-filled stdblk file ('std.blk') can be created on Linux
# by the following command (assuming STDBLK_SIZE is 16777216 bytes):
#            head -c 16777216 /dev/zero > std.blk
# Note: to create a 'std.blk' file of 0 size on Linux simply use:
#            touch std.blk
trapCantOpenStdblk:
  movl $61, %ebx
  leal msgTrapCantOpenStdblk, %eax
  call traceExitMsg
  jmp exitFail
trapCantCloseStdblk:
  movl $62, %ebx
  leal msgTrapCantCloseStdblk, %eax
  call traceExitMsg
  jmp exitFail
# ----------------------------------------------------------------------------
#                          TRAPS: STREAMS
# ----------------------------------------------------------------------------
# Note: this FVM will automatically create (or recreate) a 'std.trc' file
# as it starts up; any previous data in that file will be lost
trapCantOpenStdtrc:
  movl $71, %ebx
  jmp exitFail
trapCantCloseStdtrc:
  movl $72, %ebx
  jmp exitFail
trapCantWriteToStdtrc:
  movl $73, %ebx
  jmp exitFail

# Note: this FVM will automatically create (or recreate) a 'std.exp' file
# as it starts up; any previous data in that file will be lost
trapCantOpenStdexp:
  movl $74, %ebx
  leal msgTrapCantOpenStdexp, %eax
  call traceExitMsg
  jmp exitFail
trapCantCloseStdexp:
  movl $75, %ebx
  leal msgTrapCantCloseStdexp, %eax
  call traceExitMsg
  jmp exitFail

# Note: to create a 'std.imp' file of 0 size on Linux simply use:
#            touch std.imp
trapCantOpenStdimp:
  movl $77, %ebx
  leal msgTrapCantOpenStdimp, %eax
  call traceExitMsg
  jmp exitFail
trapCantCloseStdimp:
  movl $78, %ebx
  leal msgTrapCantCloseStdimp, %eax
  call traceExitMsg
  jmp exitFail
# ============================================================================

