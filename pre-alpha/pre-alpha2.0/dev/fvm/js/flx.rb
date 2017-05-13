#!/usr/bin/ruby
# ============================================================================
VERSION = "flx.rb Freelang cross compiler version pre-alpha-0.0.0.1 for FVM 2.0"
# ============================================================================
#
# Copyright © 2017, Robert Gollagher.
# SPDX-License-Identifier: GPL-3.0+
# 
# Program:    flx.rb
# Copyright © Robert Gollagher 2015
# Author :    Robert Gollagher   robert.gollagher@freeputer.net
# Created:    20150329
# Updated:    20170514-0023
# Version:    pre-alpha-0.0.0.2 for FVM 2.0
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# ============================================================================
# 
# WARNING: This is pre-alpha software and as such may well be incomplete,
# unstable and unreliable. It is considered to be suitable only for
# experimentation and nothing more.
#
# ============================================================================
# Enhancements compared to flx.rb 1.0.0.0
# =======================================
#
# (1) Not only a label reference but alternatively a constant reference or
#     a slot reference can be used in complex instructions; this can be
#     quite useful in advanced programming techniques.
#
# (2) A total of 19 new instructions have been added to the FVM instruction
#     set in FVM 1.1 compared to FVM 1.0. That is, FVM 1.1 bytecode is
#     a superset of FVM 1.0 bytecode. This version of flx supports
#     those new instructions. The 19 new instructions are mostly concerned
#     with providing runtime metadata upon demand but also contain additional
#     stack-manipulation instructions designed to better facilitate working
#     with the top 4 data stack elements (so as to make the instruction set
#     more symmetrical in its support of the top 4 elements) and some new
#     instructions allowing greater control of application termination.
#
#     There are 7 new complex instructions:
#               math    trap      die
#               read?   write?    get?     put?
#
#     There are 12 new simple instructions:
#               trace?  rchan?    wchan?   gchan?   pchan?    pc?
#               [fly]   swap2     rev4     tor4     rot4      zoom
#
#     Of course, although this version of flx supports compiling Freelang
#     programs that use those new instructions, you cannot actually run
#     such compiled programs without an FVM 1.1 instance.
#
# IMPORTANT: Point (2) above is irrelevant to FVM 2.0 since it lists
# tentative new instructions in FVM 1.1 which are not necessarily going
# to be included in FVM 2.0. This version of 'flx.rb' is a hack based
# on the pre-alpha-0.0.0.1 for FVM 1.1. This hacked version is
# being used to generate binaries to be run on the JavaScript prototype
# of FVM 2.0 currently in development (see 'fvmui.html' and 'fvm2.js').
#
# TODO Needs refactoring for DRY
#
# ============================================================================
# Instructions
# ============
#
# USAGE: AS A FREELANG COMPILER
# =============================
#
# To compile a Freelang program for Freeputer using this 'flx.rb' compiler,
# specify the name of the Freelang source file followed by your desired name
# for the compiled program file (typically always 'rom.fp'). For example,
# if the Freelang source code is in 'source.fl' then:
#
#     ./flx.rb source.fl rom.fp
#
# Will produce:
#
#     rom.fp       = the newly compiled Freeputer program
#     map.info     = a very helpful map of the compiled program
#     debug.flx    = unimportant, only used to debug this compiler
#
# The procedure for deployment of your compiled program depends on
# the implementation of the Freeputer Virtual Machine (FVM) to which you
# are deploying. For example, the deployment process to an FVM running
# on bare metal would be different to that for an FVM running on
# a host operating system such as Linux. The latter typically requires
# that in the same directory as your FVM executable you place:
#   (1) your compiled program, always called 'rom.fp'
#   (2) an empty file for "standard import", always called 'std.imp'
#   (3) an empty or zero-filled file for "standard block", always called
#       'std.blk' and exactly the size required by your FVM instance.
# Your program will be automatically loaded into ROM and run, starting at
# ROM address 0, when the FVM starts up. You will notice that the FVM
# (if applicable) will automatically create the files 'std.exp' for
# "standard export" and 'std.trc' for "standard trace".
#
# USAGE: AS A FREEPUTER DECOMPILER
# ================================
#
# If you pass only one argument to this 'flx.rb' script then it will
# function as a decompiler (that is, disassembler) rather than a compiler.
# For example, if 'rom.fp' is a compiled Freeputer program then:
#
#     ./flx.rb rom.fp
#
# Will produce:
#
#     rom.fp.fpd  = a plain-text representation of the compiled program,
#                   obtained by decompiling 'rom.fp'
#
# FURTHER INFORMATION
# ===================
#
# This script 'flx.rb' is a Freelang compiler written in Ruby 1.9.3.
# It is known as a "cross compiler" because it does not run on Freeputer
# but it can be used to compile programs that do run on Freeputer.
#
# You should mainly use flc, the self-hosted Freelang compiler (as compiled
# from 'flc.fl') to compile Freeputer programs. It is written in Freelang and
# runs on Freeputer. It is located in the 'apps/compiler' directory.
# However, there are times when you might wish to use a cross compiler,
# such as when compiling a very large program when you do not have a
# suitably large Freeputer Virtual Machine (FVM) instance available to run
# the self-hosted compiler configured for very large memory use, or when
# for some other reason the self-hosted compiler is unavailble.
#
# Please be aware that the self-hosted compiler is more actively
# maintained and more thoroughly tested through actual use than is this
# cross compiler. Where the behaviour of the self-hosted compiler differs
# from that of this cross compiler, the behaviour of the self-hosted compiler
# should be taken to be the correct behaviour unless obviously a bug.
#
# This 'flx.rb' script also functions as a decompiler (that is, disassembler)
# as described above. The terms "decompile" and "disassemble" are essentially
# synonyms on the Freeputer platform. The output of the decompiler is
# not Freelang source code but rather a precise, human-readable representation
# of the exact structure of a compiled Freeputer program.
#
# KNOWN LIMITATIONS
# =================
#
# * Unlike the self-hosted compiler, this cross compiler does not support
#   the declaration of multiple-line string literals. That is, this
#   cross compiler requires that a string literal must be entirely declared
#   on a single line; otherwise compiling will fail with some seemingly
#   unrelated error message. TODO (Note: Freelang never permits
#   string literals to be longer than 80 characters.)
#
# * Unlike the self-hosted compiler, this cross compiler supports neither
#   the declaration of multiple string literals on a single line nor
#   having more than one anonymous string on a single line. TODO
#   Note: it is in any case considered best practice not to declare
#   more than one named or anonymous string per line.
#
# * Unlike the self-hosted compiler, this cross compiler requires that
#   the slotFloor declaration must occur before the first slot declaration
#   occurs. (Note: in Freelang, the slotFloor declaration specifies the
#   address in RAM to which the lowest slot shall be allocated.
#   A slot is essentially a variable of fixed size.)
#   Note: it is in any case considered best practice to ensure that
#   the slotFloor declaration occurs near the beginning of the source code
#   and before the first slot declaration occurs.
#
# * Error messages, and other minor aspects of behaviour, differ between
#   this cross compiler and the self-hosted compiler. However, the end
#   result is the same. That is, both compilers should always produce
#   precisely the same compiled program as each other when used to
#   compile the same source code, except if the Freelang program
#   is too large for the self-hosted compiler to compile.
#
# * Unlike the self-hosted compiler, this cross compiler supports the
#   use of binary (e.g. 0b01111111111111111111111111111111)
#   hexadecimal (e.g. 0x7fffffff) literals. The self-hosted compiler
#   currently only supports decimal literals (e.g. 2147483647). TODO
#
# * The decompiler output produced by this 'flx.rb' script differs from
#   that produced by the self-hosted decompiler. However, these differences
#   are merely a matter of formatting and number base conversion.
#
# * There is a bug in this 'flx.rb' compiler with respect to Chinese
#   characters (and the like). It does not compile them properly.
#   You must instead use flc, the self-hosted compiler, to compile source
#   code that contains Chinese characters in String literals. TODO
#
# * There is a bug in this 'flx.rb' compiler with respect to a lone ;
#   semicolon floating in the source code without in fact being the end
#   of a word definition. It is not raising an appropriate error. TODO
#
# RUBY STYLE
# ==========
#
# This 'flx.rb' script was intentionally not written as idiomatic Ruby.
# It was written before the self-hosted Freelang compiler and its principal
# purpose was to bootstrap that compiler. As such it was written as a
# proof-of-concept that a Freelang compiler could easily be written
# in Freelang itself. Therefore, for example, this 'flx.rb' script uses
# none of the object-oriented features of Ruby. The main value of this
# script now is to serve to verify the output of the self-hosted compiler;
# as such it would be counterproductive to refactor it. Maximum line
# length is 78 characters in accordance with Freeputer convention;
# this allows this script to be conveniently edited in FreeLine,
# a text editor that runs on the Freeputer platform.
#
# ----------------------------------------------------------------------------
# REQUIREMENTS
# ============
#
#   This 'flx.rb' script is intended to be run on x86 32-bit Linux but
#   should also work on other 32-bit little-endian platforms
#   running Ruby 1.9.3. Use Ruby 1.9.3 or higher.
#
# DETAILS
# -------
#
#   This 'flx.rb' script is known to work correctly on:
#     * 32-bit Debian GNU/Linux 7.8 using
#     * ruby 1.9.3p194 (2012-04-20 revision 35410) [i486-linux]
# ============================================================================

# ============================================================================
# =======================  SCRIPT INITIALIZATION =============================
# ============================================================================

WORD_SIZE = 4 # bytes
$sourceFilename = ARGV[0]
$outputFilename = ARGV[1]
decompileOnly = false

if ($sourceFilename == nil ) then
  printf("\nTHIS FREELANG CROSS-COMPILER REQUIRES AT LEAST ONE ARGUMENT\n")
  printf("To compile  :    ./flx.rb sourceFilename binaryFilename\n")
  printf("To decompile:    ./flx.rb binaryFilename\n\n")
  abort
end
if ($outputFilename == nil) then
  decompileOnly = true
  printf("Decompiling only...\n")
  $disassembledFilename = ARGV[0] + ".fpd"
end

# Source file (i.e. input file for compiler or decompiler)
sourceFile = File.open($sourceFilename,"r")

if (decompileOnly == false ) then
  # Compiler debug file, only of use for debugging this compiler itself
  $debugFile = File.open("debug.flx", "w")
  printf($debugFile,"\nCompiling (FIRST PASS shown)...\n\n")

  # Map of compiled program, very useful for debugging compiled program
  $mapFile = File.open("map.info", "w")

  # Output file that will contain the newly compiled program
  $outputFile = File.open($outputFilename, "wb")
  lastPos = 0
  currentPos = 0
end

# ============================================================================
# =======================  FVM INSTRUCTION SET ===============================
# ============================================================================

# FVM instruction set (must be in order)
$iSet = [
  "===",
  "lit",
  "call",
  "go",
  "go[>0]",
  "go[>=0]",
  "go[==0]",
  "go[!=0]",
  "go[<=0]",
  "go[<0]",
  "go[>]",
  "go[>=]",
  "go[==]",
  "go[!=]",
  "go[<=]",
  "go[<]",
  "go>0",
  "go>=0",
  "go==0",
  "go!=0",
  "go<=0",
  "go<0",
  "go>",
  "go>=",
  "go==",
  "go!=",
  "go<=",
  "go<",
  "reador",
  "writor",
  "tracor",
  "getor",
  "putor",
  "readorb",
  "writorb",
  "tracorb",
  "getorb",
  "putorb",
  "math", # FIXME reconsider the order of new instructions
  "trap",
  "die",
  "read?",
  "write?",
  "get?",
  "put?",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "trace?", # FIXME reconsider the order of new instructions
  "zoom",
  "rchan?",
  "wchan?",
  "gchan?",
  "pchan?",
  "pc?",
  "[fly]",
  "swap2",
  "rev4",
  "tor4",
  "rot4",
  "ret",
  "invoke",
  "[invoke]",
  "fly",
  "swap",
  "over",
  "rot",
  "tor",
  "leap",
  "nip",
  "tuck",
  "rev",
  "rpush",
  "rpop",
  "drop",
  "drop2",
  "drop3",
  "drop4",
  "dup",
  "dup2",
  "dup3",
  "dup4",
  "hold",
  "hold2",
  "hold3",
  "hold4",
  "speek",
  "speek2",
  "speek3",
  "speek4",
  "spush",
  "spush2",
  "spush3",
  "spush4",
  "spop",
  "spop2",
  "spop3",
  "spop4",
  "dec",
  "decw",
  "dec2w",
  "inc",
  "incw",
  "inc2w",
  "@",
  "!",
  "[@]",
  "@b",
  "!b",
  "[@b]",
  "@@",
  "@!",
  "[@@]",
  "@@b",
  "@!b",
  "[@@b]",
  "+",
  "-",
  "*",
  "/",
  "%",
  "/%",
  "[+]",
  "[-]",
  "[*]",
  "[/]",
  "[%]",
  "[/%]",
  "neg",
  "abs",
  "&",
  "|",
  "^",
  "[&]",
  "[|]",
  "[^]",
  "<<",
  ">>",
  "[<<]",
  "[>>]",
  "move",
  "fill",
  "find",
  "match",
  "moveb",
  "fillb",
  "findb",
  "matchb",
  "homio",
  "rchan",
  "wchan",
  "gchan",
  "pchan",
  "ecode?",
  "rcode?",
  "rom?",
  "ram?",
  "map?",
  "stdblk?",
  "ds?",
  "ss?",
  "rs?",
  "dsn?",
  "ssn?",
  "rsn?",
  "tron",
  "troff",
  "reset",
  "reboot",
  "halt",
  "data"
]

# Some FVM instructions (must tally with the above)
ILIT = 1
ICALL = 2
IEXIT = 145
IWALL = 0
IHIGHEST_COMPLEX_OPCODE = 44 # Higher opcodes are simple not complex instrs
                             #   (or illegal if not > ILOWEST_SIMPLE_OPCODE)
ILOWEST_SIMPLE_OPCODE = 133  # This and higher opcodes are simple instrs
IDATA = 255                  # Note: breaks decompiler if wrong
IHIGHEST_OPCODE = 255        # Highest opcode in FVM instruction set

# ============================================================================
# =======================  DECOMPILER ========================================
# ============================================================================

def disassemble(filename)
  binaryFile = File.open(filename, "r")
  disasmFile = File.open($disassembledFilename, "w" )
  strContent = binaryFile.read()
  intContent = strContent.unpack("l<*") # Convert to little-endian 32-bit ints
  expectingLiteral = false
  inData = false
  inSize = false
  dataSizeCountdown = 0
  intContent.each_with_index do |cell, index|
    disasm = "---"
    # Calculate UTF_8 character corresponding to cell value.
    #   It will later be blanked out where irrelevant.
    charValue = ""
    if (cell > 0 and cell <= 31) then
      charValue = sprintf("", cell)
    elsif (cell > 0 and cell <= 0x4FF) then # Avoid falling over
      charUTF_32LE = cell.chr(Encoding::UTF_32LE)
      charValue = charUTF_32LE.ord.chr(Encoding::UTF_8)
    end
    # -----------------------------------------------------
    if (!expectingLiteral and !inData) then
      if (cell == 0) then # "iWALL" wall
        disasm = $iSet[cell]
        expectingLiteral = false
        charValue = ""
      elsif (cell <= IHIGHEST_COMPLEX_OPCODE) then
        disasm = $iSet[cell]
        expectingLiteral = true
        charValue = ""
      elsif (cell == IDATA) then
        disasm = $iSet[cell]
        inData = true
        inSize = true
        charValue = ""
      elsif (cell <= IHIGHEST_OPCODE and cell >= ILOWEST_SIMPLE_OPCODE) then
        disasm = $iSet[cell]
        expectingLiteral = false
        charValue = ""
      else
        expectingLiteral = false
        disasm = "???"
        charValue = ""
      end
    elsif (expectingLiteral and !inData) then
      disasm = cell
      expectingLiteral = false
      if (cell != ILIT) then
        charValue = ""
      end
    else # we are in a data block
      if (inSize) then
        disasm = cell
        dataSizeCountdown = cell * WORD_SIZE
        inSize = false
      else
        if (dataSizeCountdown > 0) then
          dataSizeCountdown -= 4
          disasm = cell
        else
          inData = false
          expectingLiteral = false
          disasm = $iSet[cell]
          charValue = ""
        end
      end
    end
    printf(disasmFile, "%09d ", index * WORD_SIZE)
    printf(disasmFile, "%08x ", index * WORD_SIZE)
    printf(disasmFile, "%-15s ", disasm)
    printf(disasmFile, " %-12s", charValue)
    printf(disasmFile, "0x%08x\n", cell)
  end
  binaryFile.close()
  disasmFile.close()
end

if (decompileOnly == true) then
  disassemble($sourceFilename)
  exit 0
end

# ============================================================================
# =======================  COMPILER ==========================================
# ============================================================================

# ============================================================================
#                          COMPILER: DECLARATIONS
# ============================================================================
TOKEN_MAX_LEN = 80  # Maximum legal token length (characters)
NAME_MAX_LEN = 30   # Maximum legal name length not including .#${}: markers

# Some reserved tokens
tokenComment = "("
tokenCommentEnd = ")"
tokenInlineComment = "\\"
tokenBComment = "((("
tokenBCommentEnd = ")))"
tokenWordDecl = ":"
tokenWordDeclEnd = ";"

# Regex pattern for words and the like
WDPTN = '[A-z_][0-9\?-z!]*'

# Switches
expectingAddr = false
$syntaxError = false
inComment = false
inBComment = false
inInlineComment = false
inWordDecl = false
expectingWordName = false
expectingConstantValue = false
expectingSlotSize = false
inStrLiteral = false # between ." and "
expectingStrConstValue = false
inData = false
expectingSlotFloorValue = false
expectingOnFailValue = false
anonStr = false

# Variables
wordDeclName = ""
currNspace = "" # current broad namespace
wordNspace = "" # namespace from which to retrieve the current word
currConstDeclKey = ""
currStrConstDeclKey = ""
currSlotDeclKey = ""

# Accumulators and floors
slotPointer = 0 # Slot allocation pushes this pointer higher in RAM
slotFloor = nil # slotPointer starts at slotFloor in RAM

# Support for branch on failure (new in FVM 2.0)
declaredFailAddr = 0 # To be used as declared failure address until next redefined
$effectiveFailAddr = 0 # To be used as effective failure address now
# FIXME compiler preferably should disallow onFail outside size of program itself

# Hashes
labelDecls = Hash.new()
labelRefs = []
wordDecls = Hash.new()
wordRefs = []
constDecls = Hash.new()
constRefs = []
strConstDecls = Hash.new()
strConstRefs = []
slotDecls = Hash.new()
slotRefs = []
referencedStrs = []
anonStrConstDecls = Hash.new()
anonStrConstRefs = []

# Statistical counters
numInstrs = 0
numSimpleInstrs = 0
numComplexInstrs = 0
numLabelDecls = 0
numLabelRefs = 0
numWordDecls = 0
numWordRefs = 0
numConstDecls = 0
numConstRefs = 0
numStrConstDecls = 0
numStrConstRefs = 0
numSlotDecls = 0
numSlotRefs = 0
numAnonStrConstDecls = 0
numAnonStrConstRefs = 0
numDataCells = 0
numStrCells = 0 # Only reference strings get compiled into program

# Some instruction ranges # FIXME make this work off HIGHEST_COMPLEX_OPCODE
addrInstrs = $iSet.find_index("call")..$iSet.find_index("put?")

# Prepared binary cellValues
opcodeLit = [$iSet.find_index("lit")].pack("l<")    # opcode for "lit"
opcodeCall = [$iSet.find_index("call")].pack("l<")  # opcode for "call"
opcodeReturn = [$iSet.find_index("ret")].pack("l<") # opcode for "return"
opcodeWall = [$iSet.find_index("===")].pack("l<")   # opcode for "wall"
opcodeData = [$iSet.find_index("data")].pack("l<")  # opcode for "data"

def writeLit(onFail)
  writeCell(ILIT, onFail)
  $effectiveFailAddr = 0
end

def writeCall(onFail)
  writeCell(ICALL, onFail)
  $effectiveFailAddr = 0
end

def writeReturn(onFail)
  writeCell(IEXIT, onFail)
end

def writeWall()
  writeCell(IWALL, 0)
end

def writeData(onFail)
  writeCell(IDATA, onFail)
  $effectiveFailAddr = 0
end

def asCell(lastPos)
  return lastPos / 4
end

def debugOpcode(opcode, onFail)
  ([(opcode | onFail << 8)].pack("l<")).bytes.to_a.each do |byte| printf($debugFile,"%d ",byte) end
end

# Write program cell to binary output file
def writeCell(cellValue, onFail) # Do not call this with a nil cellValue
  cellValue = cellValue | (onFail << 8)
  cellBinary = [cellValue].pack("l<") # Convert to little-endian 32-bit int
  $outputFile.write(cellBinary)
  return cellBinary
end

# Prints syntax error and returns true if integer value it not
#   within range of two's complement 32-bit integer;
#   otherwise returns false indicating OK
def enforceRange(lineNum, token, value)
  if (value < -2147483648 or value > 2147483647) then
    $syntaxError = true
    printf("%d Literal out of bounds: %s\n", lineNum, token)
    return trueinData
  else
    return false
  end
end

# Prints syntax error and returns true if integer value it not positive and
#   within range of two's complement 32-bit integer;
#   otherwise returns false indicating OK
def enforcePosRange(lineNum, token, value)
  if (value < 0 or value > 2147483647) then
    $syntaxError = true
    printf("%d Literal illegally negative or out of bounds: %s\n",
      lineNum, token)
    return true
  else
    return false
  end
end

# ============================================================================
#                          COMPILER: FIRST PASS
# ============================================================================
sourceFile.each_with_index do | line, lineNum0 |
  if ($syntaxError) then break end
  inInlineComment = false
  inStrLiteral = false
  lineNum = lineNum0 + 1 # Count from line 1 (not as 0)
  tokens = line.split(' ')
  # ==========================================================================
  tokens.each do |token|
  if ($syntaxError) then break end
    # ========================================================================
    # Check for token too long ===============================================
    if (token.length > TOKEN_MAX_LEN ) then
      $syntaxError = true
      printf("%d Token too long: %s\n", lineNum, token)
      break
    end
    # ========================================================================
    # Check for comments =====================================================
    if (!inStrLiteral and (token == tokenBCommentEnd)) then
      inBComment = false; next end
    if (inBComment) then next end
    if (!inStrLiteral and (token == tokenCommentEnd)) then
      inComment = false; next end
    if (inComment or inInlineComment) then next end
    if (!inStrLiteral and (token == tokenBComment)) then
      inBComment = true; next end
    if (!inStrLiteral and (token == tokenComment)) then
      inComment = true; next end
    if (!inStrLiteral and (token == tokenInlineComment)) then
      inInlineComment = true
      next
    end
    # ========================================================================
    # Check (at line not token level) for string constant value ==============
    # Note: this compiler has a limitation of only allowing 1-line strings
    # and only allowing 1 string declaration per line
    if (/(\."\s+[^"]+\s+"\s?)/ =~ line) then
      if (line.count("\"") > 2) then
        # Line contains similar to ." foo " ." bar "
        $syntaxError = true
        printf("%d Too many \" present on line.\n", lineNum)
        printf("This cross compiler only supports ")
        printf("1 string declaration per line.\n")
        printf("It also does not support multiple-line string literals.\n")
        break
      else
        # Line contains similar to ." Hello world! " ( this is a str decl )
        strStart = line.index('." ') + 3
        strEnd = line.index(' "') - 1
        stringLitValue = line[strStart..strEnd] # Hello world!
      end
    elsif (/(\."\s+[^"]*[^\s]+"\s?)/ =~ line) then
      # Line contains similar to ." Hello world!" (no spc before final ")
      $syntaxError = true
      printf("%d Final \" of string literal not preceded by space ", lineNum)
      break
    else
      stringLitValue = "" # Line contains no string literal value
    end
    # ========================================================================
    # Check for endData ======================================================
    if (token == "===" and inStrLiteral == false) then
      # If we happen to be in a data section, hitting wall ends that section
      if (inData) then
         printf($debugFile, "---inData===>")
         numDataCells += 1
         numSimpleInstrs -= 1 # Otherwise final iWALL gets counted twice
         inData = false
      else
        $syntaxError = true
        printf("%d === not allowed except to end a data section\n", lineNum)
        break
      end
    end
    # ========================================================================
    lastPos = currentPos
    debugPrefixStr = ""                       # Just for debugging ("lit")
    debugPrefixBinary = ""                    # Just for debugging (1 = lit)
    debugPostfixBinary = ""                   # Just for debugging (addr)
    wordNspace = currNspace # Any change to wordNspace lasts for 1 token only
    # ========================================================================
    # Deal with onFail definition ============================================
    if (!inStrLiteral and (token == "onFail")) then
        expectingOnFailValue = true
        next
    end
    if (expectingOnFailValue) then
      if (/(\A[-\+]?[0-9]{1,10}\z)/ =~ token) then
        # Value is decimal numeric literal
        declaredFailAddr = token.to_i
        if (enforcePosRange(lineNum, token, declaredFailAddr)) then break end
        expectingOnFailValue = false
        next
      elsif (/(\A0x[a-f0-9]{1,8}\z)/ =~ token) then
        # Value is hexadecimal numeric literal (starting with 0x)
        declaredFailAddr = token.to_i(16)
        if (enforcePosRange(lineNum, token, declaredFailAddr)) then break end
        expectingOnFailValue = false
        next
      else
        $syntaxError = true
      printf("%d Invalid size of onFail declaration: %s\n", lineNum, token)
        break
      end
    end
    # ========================================================================
    # Deal with slotFloor definition =========================================
    if (!inStrLiteral and (token == "slotFloor")) then
      if (slotFloor == nil) then
        expectingSlotFloorValue = true
        next
      else
        $syntaxError = true
        printf("%d Multiple definition of slotFloor: %s\n", lineNum, token)
        break
      end
    end
    if (expectingSlotFloorValue) then
      if (/(\A[-\+]?[0-9]{1,10}\z)/ =~ token) then
        # Value is decimal numeric literal
        slotFloor = token.to_i
        if (enforcePosRange(lineNum, token,slotFloor)) then break end
        expectingSlotFloorValue = false
        next
      elsif (/(\A0x[a-f0-9]{1,8}\z)/ =~ token) then
        # Value is hexadecimal numeric literal (starting with 0x)
        slotFloor = token.to_i(16)
        if (enforcePosRange(lineNum, token,slotFloor)) then break end
        expectingSlotFloorValue = false
        next
      else
        $syntaxError = true
      printf("%d Invalid size of slotFloor declaration: %s\n", lineNum, token)
        break
      end
    end
    # ========================================================================
    # Deal with string literal declaration  ==================================
    if (inStrLiteral) then
      # We are between ." and " and thus reading string content.
      # Here we cheat, as this Ruby compiler only supports 1-line strings and
      #   it extracts their value up above, at line level; so here we just
      #   keep ignoring tokens until reach the " denoting end of string
      #   and then we cheat by using stringLitValue from above
      if (inData) then
        $syntaxError = true
        printf("%d String literal not permitted in data section: %s\n",
                      lineNum, token)
        break
      end
      if (token == "\"") then
        if (anonStr) then
          # String literal is anonymous
          anonStrConstDecls[currStrConstDeclKey] = [stringLitValue,0]
        else
          numStrConstDecls += 1
          strConstDecls[currStrConstDeclKey] = [stringLitValue,0]
        end
        inStrLiteral = false
        anonStr = false
        next; # We're finished processing the string literal
      else
        next; # Just keep ignoring tokens till we find end of string
      end
    end
    if (expectingStrConstValue) then
      if ( /(\A\."\z)/ =~ token ) then
        # Token is ." opening a string literal
        expectingStrConstValue = false
        inStrLiteral = true
        next
      else
        # Token is not ." therefore syntax error
        $syntaxError = true
        printf("%d Here expecting .\" so token illegal: %s\n", lineNum, token)
        break
      end
    end
    # ========================================================================
    # Deal with const value declaration following const name =================
    if (expectingConstantValue) then
      if (/(\A[-\+]?[0-9]{1,10}\z)/ =~ token) then
        # Value is decimal numeric literal
        numConstDecls += 1
        constDecls[currConstDeclKey] = token.to_i
        if (enforceRange(lineNum, token,token.to_i)) then break end
        expectingConstantValue = false
        next
      elsif (/(\A0x[a-f0-9]{1,8}\z)/ =~ token) then
        # Value is hexadecimal numeric literal (starting with 0x)
        numConstDecls += 1
        constDecls[currConstDeclKey] = token.to_i(16)
        if (enforceRange(lineNum, token,token.to_i(16))) then break end
        expectingConstantValue = false
        next
      elsif (/(\A0b[0-1]{1,32}\z)/ =~ token) then
        # Value is binary numeric literal (starting with 0b)
        numConstDecls += 1
        constDecls[currConstDeclKey] = token.to_i(2)
        if (enforceRange(lineNum, token,token.to_i(2))) then break end
        expectingConstantValue = false
        next
      elsif (/('.')/ =~ token) then
        # Value is character literal (1 char between single quotes)
        numConstDecls += 1
        charVal = token[1..-2]
        constDecls[currConstDeclKey] = charVal.ord
        expectingConstantValue = false
        next
      else
        $syntaxError = true
      printf("%d Invalid value of constant declaration: %s\n", lineNum, token)
        break
      end
    end
    # ========================================================================
    # Deal with slot size declaration following slot name ====================
    if (expectingSlotSize) then
      if (/(\A[-\+]?[0-9]{1,10}\z)/ =~ token) then
        # Value is decimal numeric literal.
        # We need to convert it to an absolute RAM address for the slot.
        numSlotDecls += 1
        slotSize = token.to_i
        if (slotSize < 1 ) then
          $syntaxError = true
          printf("%d Slot size below 1 byte: %s\n", lineNum, token)
          break
        end
        slotRAMaddr = slotFloor + slotPointer
        if (slotRAMaddr > 0x7FFFFFFF ) then
          $syntaxError = true
          printf("%d Slot size would exceed addressable space: %s\n",
            lineNum, token)
          break
        end
        slotDecls[currSlotDeclKey] = slotRAMaddr
        slotPointer = slotPointer + slotSize
        expectingSlotSize = false
        next
      elsif (/(\A0x[a-f0-9]{1,8}\z)/ =~ token) then
        # Value is hexadecimal numeric literal (starting with 0x).
        # We need to convert it to an absolute RAM address for the slot.
        numSlotDecls += 1
        slotSize = token.to_i(16)
        slotRAMaddr = slotFloor + slotPointer
        if (slotRAMaddr > 0x7FFFFFFF ) then
          $syntaxError = true
          printf("%d Slot size would exceed addressable space: %s\n",
            lineNum, token)
          break
        end
        slotDecls[currSlotDeclKey] = slotRAMaddr
        slotPointer = slotPointer + slotSize
        next
      else
        $syntaxError = true
        printf("%d Invalid size of slot declaration: %s\n", lineNum, token)
        break
      end
    end
    # ========================================================================
    # Check for namespace changes ============================================
    if (token.include?("{") or token.include?("}")) then
      if (/(\A[A-z_][0-9\?-z]*{\z)/ =~ token) then # ends with {
        currNspace = token[0..-2] # Start of namespace
        wordNspace = currNspace
        if (currNspace.length > NAME_MAX_LEN) then
          $syntaxError = true
        printf("%d Illegal namespace format (too long): %s\n", lineNum, token)
          break
        end
        next
      elsif (/(\A}[A-z_][0-9\?-z]*)/ =~ token) then # starts with }
        if (token[1..-1] != currNspace) then
          # Something like foo{ ... ... ... }bar occurred such that the
          #   name of the namespace ending doesn't match the current namespace
          $syntaxError = true
          printf("%d Namespace end mismatch: %s does not match %s{\n",
            lineNum, token, currNspace)
          break
        end
        currNspace = "" # End of namespace (Note: namespaces cannot be nested)
        wordNspace = ""
        next
      elsif (/(\A{[\.\#\$:]{0,2}[A-z_][0-9\?-z]*}\z)/ =~ token)\
        then # {tkn}
          wordNspace = "" # tkn is in global namespace
          token = token[1..-2] # tkn from {tkn}
      elsif
  (/(\A[A-z_][0-9\?-z]*{[\.\#\$:]{0,2}[A-z_][0-9\?-z]*}\z)/ =~ token)
        then # nspace{tkn}
          wordNspace = token[0..(token.index("{")-1)]
                                                  # nspace from nspace{tkn}
          token = token[(token.index("{")+1)..-2] # tkn from nspace{tkn}
      elsif (/(\A'[{}]'\z)/ =~ token) then
          # token is character literal '{' or '}' so do nothing here
      else
        $syntaxError = true
        printf("%d Illegal namespace format %s\n", lineNum, token)
        break
      end
    end
    # ========================================================================
    # Check for word declarations ============================================
    if (inWordDecl) then # we are already in a word definition
      if (token == tokenWordDecl) then
        # cannot nest word declarations
        $syntaxError = true
        printf("%d Symbol : within word declaration %s\n", lineNum, token)
        break
      elsif (expectingWordName) then
        # expecting the declaration of the name of the new word
        if (/#{'(\A' + WDPTN + '+\z)'}/ =~ token \
         and token.length <= NAME_MAX_LEN) then
          # token is the name of the new word
          if (wordDecls.has_key?([token,currNspace])) then
              $syntaxError = true
              printf("%d Word already defined: %s\n", lineNum, token)
              break
          elsif (token == 'lit' or token == 'call') then
              $syntaxError = true
          printf("%d Syntax error. Illegal instruction: %s\n", lineNum, token)
              break
          elsif ($iSet.include?(token)) then
              $syntaxError = true
      printf("%d Cannot use reserved word as word name: %s\n", lineNum, token)
              break
          else
            numWordDecls += 1
            wordDecls[[token,currNspace]]= asCell(lastPos) # Store addr of wd decl
            expectingWordName = false
            wordDeclName = token
            # Now we will also automatically create a label for the new word
              if (labelDecls.has_key?([token[0..-1],
                    wordDeclName,currNspace])) then
                $syntaxError = true
                printf("%d Word matches an existing label: %s\n",
                            lineNum, token)
                printf("Either delete the existing label or name the word
                          differently.")
                break
              else  # Store addr of label decl to match the new word
                    #   (automatic label will not be local to word)
                numLabelDecls += 1
                labelDecls[[token[0..-1],"",currNspace]] = asCell(lastPos)
              end
            next
          end
        else
          # token format not legal for the name of a new word
          $syntaxError = true
          printf("%d Invalid name for new word: %s\n", lineNum, token)
          break
        end
      elsif (token == tokenWordDeclEnd ) then
           # we hit end of word declaration, will immediately add a return
          numSimpleInstrs += 1
          writeReturn(declaredFailAddr)
          # Just for debugging the compiler
          printf($debugFile,"%11s to %3d: return (", token, asCell(currentPos))
          debugOpcode(IEXIT, declaredFailAddr);
          printf($debugFile,")\n")
          currentPos += WORD_SIZE
          inWordDecl = false
          wordDeclName = ""
          next
      end
    else # we are not already in a word declaration
      if (token == tokenWordDecl ) then
        # we hit a : word declaration start
        if (inData) then
          $syntaxError = true
          printf("%d Cannot declare words in data section. Found: %s\n",
                        lineNum, token)
          break
        end
          # Automatically insert a === "wall" before the word implementation
            numSimpleInstrs += 1
            $outputFile.write(opcodeWall)
            # Just for debugging the compiler
            printf($debugFile,"%11s to %3d: === (", token, asCell(currentPos))
            debugOpcode(IWALL, 0);
            printf($debugFile,")\n")
            currentPos += WORD_SIZE
        inWordDecl = true
        expectingWordName = true
        next
      end
    end
    # ========================================================================
    cellValue = $iSet.find_index(token)       # opcode (if any) in cellValue
    if (cellValue == nil or expectingAddr) then
    # ========================================================================
    # Token is not an instruction
    # ========================================================================
      if (/(\A[-\+]?[0-9]{1,10}\z)/ =~ token) then # A,z = start,end of string
        # Token is a numeric literal (n)
        cellValue = token.to_i
        if (enforceRange(lineNum, token,token.to_i)) then break end
        if (expectingAddr or inData) then
          # n follows an address instruction (so do nothing here)
          #    or is to be written into the program as data (ditto)
          currentPos += WORD_SIZE
          expectingAddr = false
        else
          # n needs to be expanded to "lit" followed by n
          numComplexInstrs += 1
          writeLit(declaredFailAddr)
          currentPos += WORD_SIZE * 2
          debugPrefixStr = "lit"
          debugPrefixBinary = [(ILIT | declaredFailAddr << 8)].pack("l<")
        end
      # ----------------------------------------------------------------------
      elsif (/(\A[-\+]?[0-9]+\z)/ =~ token) then
        # Token is an integer literal with too many digits
        $syntaxError = true
        printf("%d Integer literal too long: %s\n", lineNum, token)
        break
      # ----------------------------------------------------------------------
      elsif (/(\A0x[a-f0-9]{1,8}\z)/ =~ token) then # A,z = start,end of str
        # Token is a hexadecimal numeric literal (n)
        cellValue = token.to_i(16)
        if (enforceRange(lineNum, token,token.to_i(16))) then break end
        if (expectingAddr or inData) then
          # n follows an address instruction (so do nothing here)
          #    or is to be written into the program as data (ditto)
          currentPos += WORD_SIZE
          expectingAddr = false
        else
          # n needs to be expanded to "lit" followed by n
          numComplexInstrs += 1
          writeLit(declaredFailAddr)
          currentPos += WORD_SIZE * 2
          debugPrefixStr = "lit"
          debugPrefixBinary = [(ILIT | declaredFailAddr << 8)].pack("l<")
        end
      # ----------------------------------------------------------------------
      elsif (/(\A0b[0-1]{1,32}\z)/ =~ token) then # A,z = start,end of string
        # Token is a binary numeric literal (n)
        cellValue = token.to_i(2)
        if (enforceRange(lineNum, token,token.to_i(2))) then break end
        if (expectingAddr or inData) then
          # n follows an address instruction (so do nothing here)
          #    or is to be written into the program as data (ditto)
          currentPos += WORD_SIZE
          expectingAddr = false
        else
          # n needs to be expanded to "lit" followed by n
          numComplexInstrs += 1
          writeLit(declaredFailAddr)
          currentPos += WORD_SIZE * 2
          debugPrefixStr = "lit"
          debugPrefixBinary = [(ILIT | declaredFailAddr << 8)].pack("l<")
        end
      # ----------------------------------------------------------------------
      elsif (/('.')/ =~ token) then
        # Token is a character literal (1 char between single quotes)
        charVal = token[1..-2]
        cellValue = charVal.ord
        if (expectingAddr) then
          # c follows an address instruction (nonsense)
            $syntaxError = true
            printf(
         "%d Char literal not allowed here (expected label or address): %s\n",
                    lineNum, token)
            break
        else
          if (inData) then
            # n is to be written into the program as data (so do nothing here)
            currentPos += WORD_SIZE
            expectingAddr = false
          else
            # n needs to be expanded to "lit" followed by c
            numComplexInstrs += 1
            writeLit(declaredFailAddr)
            currentPos += WORD_SIZE * 2
            debugPrefixStr = "lit"
            debugPrefixBinary = [(ILIT | declaredFailAddr << 8)].pack("l<")
          end
        end
      else
      # ----------------------------------------------------------------------
      # Token is neither an instruction nor a numeric or char literal
      # ----------------------------------------------------------------------
        if (/#{'(\A:{1,2}' + WDPTN + '+\z)'}/ =~ token) then
          # Token is a label reference (starts with :).
          if (!expectingAddr and !inData) then
            # We were not expecting a label reference here so we shall treat
            #   it as a command to place the address of the corresponding
            #   label declaration on the data stack (like a "lit").
            # So labelRef needs to be expanded to "lit" followed by labelRef
            numComplexInstrs += 1
            writeLit(declaredFailAddr)
            currentPos += WORD_SIZE
            debugPrefixStr = "lit"
            debugPrefixBinary = [(ILIT | declaredFailAddr << 8)].pack("l<")
          end
          # Store [name, addr, lineNum, wordDeclName] of labelRef
          numLabelRefs += 1
          namespace = wordDeclName # ::labelRef is treated as global
          if (token[2..2] == ":") then namespace = "" end
          labelRefs << [token[1..-1], currentPos, lineNum,
                          namespace, wordNspace]
          cellValue = 0 # Assign arbitrary zero value until SECOND PASS
          currentPos += WORD_SIZE
          expectingAddr = false
        # --------------------------------------------------------------------
        elsif (/#{'(\A\.{1,2}' + WDPTN + '+\z)'}/ =~ token) then
          # Token is a constant reference (starts with .)
          if (!expectingAddr and !inData) then
            numConstRefs += 1
            numComplexInstrs += 1
            writeLit(declaredFailAddr)
            currentPos += WORD_SIZE
            debugPrefixStr = "lit"
            debugPrefixBinary = [(ILIT | declaredFailAddr << 8)].pack("l<")
          end
          # Store [name, value, lineNum, wordDeclName] of constRef
          namespace = wordDeclName # ..constRef is treated as global
          if (token[2..2] == ".") then namespace = "" end
          constRefs << [token[1..-1], currentPos, lineNum,
                          namespace, wordNspace]
          cellValue = 0 # Assign arbitrary zero value until SECOND PASS
          currentPos += WORD_SIZE
          expectingAddr = false
        # --------------------------------------------------------------------
        elsif (/#{'(\A\#{1,2}' + WDPTN + '+\z)'}/ =~ token) then
          # Token is a slot reference (starts with #)
          if (!expectingAddr and !inData) then
            numSlotRefs += 1
            numComplexInstrs += 1
            writeLit(declaredFailAddr)
            currentPos += WORD_SIZE
            debugPrefixStr = "lit"
            debugPrefixBinary = [(ILIT | declaredFailAddr << 8)].pack("l<")
          end
          # Store [name, value, lineNum, wordDeclName] of slotRef
          namespace = wordDeclName # ##slotRef is treated as global
          if (token[2..2] == "#") then namespace = "" end
          slotRefs << [token[1..-1], currentPos, lineNum,
                          namespace, wordNspace]
          cellValue = 0 # Assign arbitrary zero value until SECOND PASS
          currentPos += WORD_SIZE
          expectingAddr = false
        # --------------------------------------------------------------------
        elsif (expectingAddr) then # FIXME ensure this change goes into flc too (note above and below order changes)
          # We were expecting an address, and we already know token is not a
          #   numeric literal or a label/constant/slot reference, therefore
          #   we have a syntax error
          $syntaxError = true
          printf("%d Not a valid label/constant/slot reference: %s\n",
            lineNum, token)
          break
        # --------------------------------------------------------------------
        elsif (/#{'(\A' + WDPTN + '+:\z)'}/ =~ token) then
          # Token is a label declaration (ends with :)
          if (inData) then
            $syntaxError = true
            printf("%d Cannot declare label in data section: %s\n",
                        lineNum, token)
            break
          end
          if (token.length > (NAME_MAX_LEN + 1)) then
            $syntaxError = true
            printf("%d Illegal label name (too long): %s\n", lineNum, token)
            break
          end
         if (labelDecls.has_key?([token[0..-2],wordDeclName,currNspace])) then
            $syntaxError = true
            printf("%d Label (or corresponding word) already defined: %s\n",
                     lineNum, token)
            break
          else  # Store addr of label decl
            numLabelDecls += 1
            labelDecls[[token[0..-2],wordDeclName,currNspace]] = asCell(lastPos)
          end
        # --------------------------------------------------------------------
        elsif (/#{'(\A' + WDPTN + '+\z)'}/ =~ token) then
          # Token is a word reference.
          if (inData) then
            $syntaxError = true
            printf("%d Word reference not permitted in data section: %s\n",
                          lineNum, token)
            break
          end
          # Store [name, addr, lineNum] of wordRef
          cellValue = 0 # Assign arbitrary zero value until SECOND PASS
          # Need to write assumed 'call' instruction prior to word reference
          #   unless we are in a data section
          if (!inData) then
            numWordRefs += 1
            numComplexInstrs += 1
            writeCall(declaredFailAddr)
            currentPos += WORD_SIZE # TODO refactor currentPos, lastPos as cell counts not byte counts
            wordRefs << [token, currentPos, lineNum, wordNspace]
          else
            numDataCells +=1
            wordRefs << [token, currentPos, lineNum, wordNspace]
            currentPos += WORD_SIZE
          end
          if (!inData) then
            debugPrefixStr = "call"
            debugPrefixBinary = [(ICALL | declaredFailAddr << 8)].pack("l<")
            currentPos += WORD_SIZE
          end
        # --------------------------------------------------------------------
        elsif (/#{'(\A' + WDPTN + '+\.'}\z)/ =~ token) then
          # Token is a constant declaration (ends with .)
          if (token.length > (NAME_MAX_LEN + 1)) then
            $syntaxError = true
           printf("%d Illegal constant name (too long): %s\n", lineNum, token)
            break
          end
          if (inData) then
            $syntaxError = true
            printf("%d Cannot declare constant in data section: %s\n",
                        lineNum, token)
            break
          end
         if (constDecls.has_key?([token[0..-2],wordDeclName,currNspace])) then
            $syntaxError = true
            printf("%d Constant already defined: %s\n", lineNum, token)
            break
          else  # The next token should be the value of the constant.
            # It will be read on the next iteration of this parsing loop
            # and will then be placed into the constDecls hash
            # under the key currentConstDeclKey
            currConstDeclKey = [token[0..-2],wordDeclName,currNspace]
            expectingConstantValue = true
            next
          end
        # --------------------------------------------------------------------
        elsif (/#{'(\A\${1,2}' + WDPTN + '+\z'})/ =~ token) then
          # Token is a strConst reference (starts with $)
          # Store [name, value, lineNum, wordDeclName] of strConstRef
          namespace = wordDeclName # $$strConstRef is treated as global
          if (token[2..2] == "$") then namespace = "" end
          currentPos += WORD_SIZE  # Account for "lit" or (in data) value cell
          refPos = currentPos      # For (in data) adjustment below
          if (inData) then
            refPos -= WORD_SIZE    # Adjust for no "lit" in data section
          end
          strConstRefs << [token[1..-1], refPos, lineNum,
                          namespace, wordNspace]
          # needs to be expanded to "lit" followed by addr of compiled string
          #   unless we are in data
          if (!inData) then
            numStrConstRefs += 1
            numComplexInstrs += 1
            writeLit(declaredFailAddr)
            currentPos += WORD_SIZE # This accounts for value cell after "lit"
            debugPrefixStr = "lit (for string)"
            debugPrefixBinary = [(ILIT | declaredFailAddr << 8)].pack("l<")
          end
          cellValue = 0 # Assign arbitrary zero value until SECOND PASS
          expectingAddr = false
        # --------------------------------------------------------------------
        elsif (/#{'(\A' + WDPTN + '+\$\z)'}/ =~ token) then
          # Token is a strConst declaration (ends with $)
          if (token.length > (NAME_MAX_LEN + 1)) then
            $syntaxError = true
            printf("%d Illegal string name (too long): %s\n", lineNum, token)
            break
          end
          if (inData) then
            $syntaxError = true
            printf("%d Cannot declare string constant in data section: %s\n",
                        lineNum, token)
            break
          end
          if (strConstDecls.has_key?(
            [token[0..-2],wordDeclName,currNspace,0])) then
              $syntaxError = true
              printf("%d strConst already defined: %s\n", lineNum, token)
              break
          else  # The next token should be ." followed by str literal tokens
                # and finally ending with the token "
            currStrConstDeclKey = [token[0..-2],wordDeclName,currNspace,0]
            expectingStrConstValue = true
            next
          end
        # --------------------------------------------------------------------

        elsif (/#{'(\A' + WDPTN + '+\#\z)'}/ =~ token) then
          # Token is a slot declaration (ends with .)
          if (slotFloor == nil) then
            $syntaxError = true
            printf("%d No slotFloor defined. Cannot declare slot: %s\n",
                        lineNum, token)
        printf("Define a slotFloor RAM address such as: slotFloor 16777216\n")
            break
          end
          if (token.length > (NAME_MAX_LEN + 1)) then
            $syntaxError = true
            printf("%d Illegal slot name (too long): %s\n", lineNum, token)
            break
          end
          if (inData) then
            $syntaxError = true
            printf("%d Cannot declare slot in data section: %s\n",
                        lineNum, token)
            break
          end
          if (slotDecls.has_key?([token[0..-2],wordDeclName,currNspace])) then
            $syntaxError = true
            printf("%d Slot already defined: %s\n", lineNum, token)
            break
          else  # The next token should be the size of the slot.
            # It will be read on the next iteration of this parsing loop
            # and will then be placed into the slotDecls hash
            # under the key currentSlotDeclKey
            currSlotDeclKey = [token[0..-2],wordDeclName,currNspace]
            expectingSlotSize = true
            next
          end
        # --------------------------------------------------------------------
        # Cater for anonymous string literals
        elsif ( /(\A\."\z)/ =~ token ) then
          # Token is ." opening an anonymous string literal
          numComplexInstrs += 1
          numAnonStrConstDecls += 1
          currStrConstDeclKey = numAnonStrConstDecls
          expectingStrConstValue = false
          inStrLiteral = true
          anonStr = true
          # Write "lit"
          writeLit(declaredFailAddr)
          currentPos += WORD_SIZE
          debugPrefixStr = "lit"
          debugPrefixBinary = [(ILIT | declaredFailAddr << 8)].pack("l<") # TODO DRY
          anonStrConstRefs << [currStrConstDeclKey, currentPos]
          # Placeholder cell for anonymous string reference
          cellValue = 0 # Placeholder till SECOND PASS
          currentPos += WORD_SIZE
        elsif (token != tokenWordDeclEnd)
          # Token is illegal
            $syntaxError = true
            printf("%d Syntax error: %s\n", lineNum, token)
            break
        end
      end
    else
    # ========================================================================
    # Token is an instruction and cellValue is its opcode
    # ========================================================================
      if (inData) then
        $syntaxError = true
        printf("%d Cannot use instruction names in data section: %s\n",
                      lineNum, token)
        break
      end
      currentPos += WORD_SIZE
      if (token == "lit" or token =="call") then
        # "lit" and "call" should never explicitly appear in source code
        $syntaxError = true
        printf("%d Syntax error. Illegal instruction: %s\n", lineNum, token)
        break
      elsif (token == "data" ) then
        inData = true # Following tokens are to be written into the program
                      # as static data
         numSimpleInstrs -= 1 # Otherwise initial iDATA gets counted twice
      end
      if (addrInstrs.cover?(cellValue)) then
        # Token is an instruction that should be followed by an address
        numComplexInstrs += 1
        expectingAddr = true
        $effectiveFailAddr = declaredFailAddr;
      else
        numSimpleInstrs += 1
        expectingAddr = false
        $effectiveFailAddr = declaredFailAddr;
      end
    end
    # ========================================================================
    # ========================================================================
    # Write program cell to binary output file
    if (cellValue != nil) then
      if (inData) then
         printf($debugFile, "===inData===>")
         numDataCells += 1
         $effectiveFailAddr = 0;
      end
      cellBinary = writeCell(cellValue, $effectiveFailAddr) # TODO NEXT
      # Just for debugging the compiler
      printf($debugFile,"%11s to %3d: %s %d (",
          token, asCell(lastPos), debugPrefixStr, cellValue)
        debugPrefixBinary.bytes.to_a.each do |byte|
          printf($debugFile,"%d ",byte) end
        cellBinary.bytes.to_a.each do |byte| printf($debugFile,"%d ",byte) end
        printf($debugFile,")\n")
    end
  end # tokens
# ============================================================================
end # Compiler FIRST PASS finished
# ============================================================================

if (inWordDecl) then
  if (wordDeclName == "") then
    $syntaxError = true
    printf("Word name not declared before end of source code\n")
  else
    $syntaxError = true
    printf("Word definition not ended before end of source code: %s\n",
      wordDeclName)
  end
end

if (!$syntaxError) then
  # Automatically insert a === "wall" at the end of the program
  numSimpleInstrs += 1
  writeWall() # FIXME iWALL is iFAIL in FVM 2.0
  # Just for debugging the compiler
  printf($debugFile,"%11s to %3d: === (", "===", asCell(currentPos))
  debugOpcode(IWALL, 0) 
  printf($debugFile,")\n")
  currentPos += WORD_SIZE
end

# ============================================================================
#                          COMPILER: SECOND PASS
# ============================================================================
if (!$syntaxError) then
  # --------------------------------------------------------------------------
  # Populate label refs
  # --------------------------------------------------------------------------
  labelRefs.each do |labelRef|
    labelName = labelRef[0]
    # The initial : is stripped on references to local labels
    #  but retained on references to global labels, so clear the wordNamespace
    #   of any labelRef found here to begin with : so that it will be matched
    #   against global labelDecls not word-local labelDecls
    if (labelName[0..0] == ":") then
      labelDeclAddr = labelDecls[[labelName[1..-1],"",labelRef[4]]]
    else
      labelDeclAddr = labelDecls[[labelName,labelRef[3],labelRef[4]]]
    end
    if (labelDeclAddr != nil) then
      cellBinary = [labelDeclAddr].pack("l<") # to little-endian 32-bit int
      $outputFile.seek(labelRef[1],IO::SEEK_SET)
      $outputFile.write(cellBinary)
    else
      $syntaxError = true
      printf("%d No such label: %s in word(%s) namespace(%s)\n",
        labelRef[2], labelRef[0], labelRef[3], labelRef[4])
      break
    end
  end
  # --------------------------------------------------------------------------
  # Populate word refs
  # --------------------------------------------------------------------------
  wordRefs.each do |wordRef|
    if (wordDecls.has_key?([wordRef[0],wordRef[3]])) then
      declAddr = wordDecls[[wordRef[0],wordRef[3]]]
      cellBinary = [declAddr].pack("l<") # Convert to little-endian 32-bit int
      $outputFile.seek(wordRef[1],IO::SEEK_SET)
      $outputFile.write(cellBinary)
    else
      $syntaxError = true
      printf("%d No such word: %s in namespace(%s)\n",
                wordRef[2], wordRef[0], wordRef[3])
      break
    end
  end
  # --------------------------------------------------------------------------
  # Populate const refs
  # --------------------------------------------------------------------------
  constRefs.each do |constRef|
    constName = constRef[0]
    # The initial . is stripped on references to local consts
    #  but retained on references to global consts, so clear the wordNamespace
    #   of any constRef found here to begin with . so that it will be matched
    #   against global constDecls not word-local constDecls
    if (constName[0..0] == ".") then
      constValue = constDecls[[constName[1..-1],"",constRef[4]]]
    else
      constValue = constDecls[[constName,constRef[3],constRef[4]]]
    end
    if (constValue != nil) then
      cellBinary = [constValue].pack("l<") # to little-endian 32-bit int
      $outputFile.seek(constRef[1],IO::SEEK_SET)
      $outputFile.write(cellBinary)
    else
      $syntaxError = true
      printf("%d No such const: %s in word(%s) namespace(%s)\n",
        constRef[2], constRef[0], constRef[3], constRef[4])
      break
    end
  end
  # --------------------------------------------------------------------------
  # Populate string refs and write referenced strs to end of program as data
  # --------------------------------------------------------------------------
  strConstRefs.each do |strConstRef|
    strName = strConstRef[0]
    # The initial $ is stripped on references to local strings
    # but retained on references to global strings, so clear the wordNamespace
    # of any strConstRef found here to begin with $ so that it will be matched
    #   against global strConstDecls not word-local strConstDecls
    currStrConstDecl = nil
    if (strName[0..0] == "$") then
      currStrConstDecl = strConstDecls[[strName[1..-1],"",strConstRef[4],0]]
      if (currStrConstDecl != nil) then
        strValue = currStrConstDecl[0]
        referencedStrs << currStrConstDecl
      end
    else
   currStrConstDecl = strConstDecls[[strName,strConstRef[3],strConstRef[4],0]]
      if (currStrConstDecl != nil) then
        strValue = currStrConstDecl[0]
        referencedStrs << currStrConstDecl
      end
    end
    if (strValue != nil ) then
      if (currStrConstDecl[1] == 0 ) then # Declared str not already written?
        # Convert to UTF-32 for compilation into program
        utf32StrValue = strValue.encode('UTF-32LE')
        # Write the string into the end of the compiled program
       compiledStrAddr = currentPos + WORD_SIZE # Add WORD_SIZE to skip "data"
        $outputFile.seek(currentPos,IO::SEEK_SET)
        # Blocks are [[DATA][SIZE]{size bytes of data}[WALL]]
        $outputFile.write(opcodeData) # write DATA opcode to start block
        numStrCells += 1
      $outputFile.write([utf32StrValue.length].pack("l<")) # write size of blk
        numStrCells += 1
        utf32StrValue.each_char do |char|
          numStrCells += 1
          word = char[0] # write char one-word wide
          $outputFile.write(word)
        end
        $outputFile.write(opcodeWall) # write WALL opcode to end block
        numStrCells += 1
        currStrConstDecl[1] = compiledStrAddr # Remember compiled addr in decl
        currentPos = $outputFile.pos # Next str will get compiled to here
      end
        # We may have written the string into the program now or earlier,
        #   hence we now get its compiled address directly from itself:
        compiledStrAddr = currStrConstDecl[1]
       cellBinary = [compiledStrAddr].pack("l<") # to little-endian 32-bit int
        $outputFile.seek(strConstRef[1],IO::SEEK_SET)
        $outputFile.write(cellBinary)
    else
      $syntaxError = true
      printf("%d No such string: %s in word(%s) namespace(%s)\n",
        strConstRef[2], strConstRef[0], strConstRef[3], strConstRef[4])
      break
    end
  end
  # --------------------------------------------------------------------------
  # Populate anon string refs and write to end of program as data
  # --------------------------------------------------------------------------
  anonStrConstRefs.each do |strConstRef|
    currStrConstDecl = anonStrConstDecls[strConstRef[0]]
    strValue = currStrConstDecl[0]
    if (strValue != nil) then
      # Convert to UTF-32 for compilation into program
      utf32StrValue = strValue.encode('UTF-32LE')
      # Write the string into the end of the compiled program
      compiledStrAddr = currentPos + WORD_SIZE # Add WORD_SIZE to skip "data"
      $outputFile.seek(currentPos,IO::SEEK_SET)
      # Blocks are [[DATA][SIZE]{size bytes of data}[WALL]]
      $outputFile.write(opcodeData) # write DATA opcode to start block
      numStrCells += 1
      $outputFile.write([utf32StrValue.length].pack("l<")) # write size of blk
      numStrCells += 1
      utf32StrValue.each_char do |char|
        numStrCells += 1
        word = char[0] # write char one-word wide
        $outputFile.write(word)
      end
      $outputFile.write(opcodeWall) # write WALL opcode to end block
      numStrCells += 1
      currentPos = $outputFile.pos
      cellBinary = [compiledStrAddr].pack("l<") # to little-endian 32-bit int
      $outputFile.seek(strConstRef[1],IO::SEEK_SET)
      $outputFile.write(cellBinary)
      currStrConstDecl[1] = compiledStrAddr # For map.info
    else
      $syntaxError = true
     printf("%s Compiler malfunction, anonymous string unknown: )\n", strName)
      break
    end
  end
  # --------------------------------------------------------------------------
  # Populate slot refs
  # --------------------------------------------------------------------------
  slotRefs.each do |slotRef|
    slotName = slotRef[0]
    # The initial # is stripped on references to local slots
    #   but retained on references to global slots, so clear the wordNamespace
    #   of any slotRef found here to begin with # so that it will be matched
    #   against global slotDecls not word-local slotDecls
    if (slotName[0..0] == "#") then
      slotValue = slotDecls[[slotName[1..-1],"",slotRef[4]]]
    else
      slotValue = slotDecls[[slotName,slotRef[3],slotRef[4]]]
    end
    if (slotValue != nil) then
      cellBinary = [slotValue].pack("l<") # to little-endian 32-bit int
      $outputFile.seek(slotRef[1],IO::SEEK_SET)
      $outputFile.write(cellBinary)
    else
      $syntaxError = true
      printf("%d No such slot: %s in word(%s) namespace(%s)\n",
        slotRef[2], slotRef[0], slotRef[3], slotRef[4])
      break
    end
  end
# ============================================================================
end # Compiler SECOND PASS finished                         if (!$syntaxError)
# ============================================================================

# ============================================================================
#                  COMPILER: MAP AND DEBUG OUTPUT
# ============================================================================
sourceFile.close
if ($syntaxError) then
  printf("\nFAILED TO COMPILE\n\n\n")
  # Truncate the half-written output file so it can never execute
  $outputFile.truncate(0)
else
    printf($debugFile,"\n%s\n",VERSION)
    printf($debugFile,"Program Compiled\n")
    numInstrs = numSimpleInstrs + numComplexInstrs
    printf($mapFile,"%s\n",VERSION)
    printf($mapFile, "%s\nProgram Compiled\n", $sourceFilename)
    printf($mapFile, "\n  ---------------------\n")
    printf($mapFile, "INSTRUCTIONS          = %d\n", numInstrs)
    printf($mapFile, "  Simple              = %d\n", numSimpleInstrs)
    printf($mapFile, "  Complex             = %d\n", numComplexInstrs)
    printf($mapFile, "LABELS                = %d\n", numLabelDecls)
    printf($mapFile, "  Refs                = %d\n", numLabelRefs)
    printf($mapFile, "WORDS                 = %d\n", numWordDecls)
    printf($mapFile, "  Refs                = %d\n", numWordRefs)
    printf($mapFile, "CONSTANTS             = %d\n", numConstDecls)
    printf($mapFile, "  Refs                = %d\n", numConstRefs)
    printf($mapFile, "SLOTS                 = %d\n", numSlotDecls)
    printf($mapFile, "  Refs                = %d\n", numSlotRefs)
    printf($mapFile, "Unnamed STRINGS       = %d\n", numAnonStrConstDecls)
    printf($mapFile, "Named   STRINGS       = %d\n", numStrConstDecls)
    printf($mapFile, "  Refs                = %d\n", numStrConstRefs)
    printf($mapFile, "  ---------------------\n")
    instrCells = numSimpleInstrs + (numComplexInstrs * 2)
    printf($mapFile, "    Instruction cells = %d\n", instrCells)
    printf($mapFile, "         String cells = %d\n", numStrCells)
    printf($mapFile, "           Data cells = %d\n", numDataCells)
    progSize = instrCells + numDataCells + numStrCells
    progSizeBytes = progSize * WORD_SIZE
    printf($mapFile, "         Program size = %d words\n", progSize)
    printf($mapFile, "       Program extent = %d bytes\n", progSizeBytes)
    progFrom = 0
    progTo = progSizeBytes > 0 ? progFrom + progSizeBytes - 1 : progFrom
    printf($mapFile, "               From   = %d\n", progFrom)
    printf($mapFile, "                 To   = %d\n", progTo)
    ramUsage = slotPointer
    printf($mapFile, "            RAM usage = %d bytes\n", ramUsage)
    if (slotFloor != nil && ramUsage > 0) then
        ramTo = slotPointer > 0 ? slotFloor + slotPointer - 1 : slotFloor + 0
        printf($mapFile, "               From   = %s\n", slotFloor)
        printf($mapFile, "                 To   = %s\n", ramTo)
    end
    printf($mapFile, "  ---------------------\n")
    printf($mapFile, "\nWORDS\n")
    wordDecls.each { |key, value|
      printf($mapFile, "%09d %08x %s {%s} \n",value,value,key[0],key[1]) }
    printf($mapFile, "\nLABELS\n")
    labelDecls.each { |key, value|
      printf($mapFile, "%09d %08x %s (%s) {%s} \n",
        value,value,key[0],key[1],key[2]) }
    printf($mapFile, "\nSLOTS\n")
    slotDecls.each { |key, value|
      printf($mapFile, "%09d %08x %s (%s) {%s} \n",
        value,value,key[0],key[1],key[2]) }
    printf($mapFile, "\nSTRINGS\n")
    strConstDecls.each { |key, value|
      if (referencedStrs.include?(strConstDecls[key])) then
        str = value[0]; if (str.length > 30) then
          str = str[0..29] << "\"..."
        else str << "\"" end
        printf($mapFile, "%09d %08x %s (%s) {%s}:\"%s\n",
          value[1],value[1],key[0],key[1],key[2],str)
      end
    }
    printf($mapFile, "\nANONYMOUS STRINGS\n")
    anonStrConstDecls.each { |key, value|
        str = value[0]; if (str.length > 30) then
          str = str[0..29] << "\"..."
        else str << "\"" end
        printf($mapFile, "%09d %08x %s: \"%s\n",value[1],value[1],key,str)
    }
    printf($mapFile, "  ---------------------\n")

end
$outputFile.close
$mapFile.close
# ============================================================================

