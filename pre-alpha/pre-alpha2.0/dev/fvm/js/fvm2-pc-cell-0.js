/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 *
 * Program:    fvm2.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170303
 * Updated:    20180411+
 * Version:    pre-alpha-0.0.1.46+ for FVM 2.0
 *
 *                               This Edition:
 *                                JavaScript
 *                           for HTML 5 browsers
 *
 *                                ( ) [ ] { }
 *
 * 
 * This implementation is now being experimentally cut down to a much
 * simpler (and possibly generic, configurable) stack machine in an effort
 * to find a way of reducing VM complexity by an order of magnitude.
 *
 * When run as a Web Worker, if using Chromium, you must start the browser by:
 *    chromium --allow-file-access-from-files
 *
 * Uses SharedArrayBuffer (ES2017), so on Firefox 46-54 about:config needs:
 *    javascript.options.shared_memory = true 
 *
 * TODO
 * - think hard about assembly language format and forward references
 * - consider adding non-consuming instrs for convenience aka []
 * - third space needed (pm, dm, rom) such as for strings or von Neumann
 *
 * ===========================================================================
 * DESIGN NOTES:
 * - An environ can supply blocking I/O and/or non-blocking I/O;
 *   use an environ appropriate to the program at hand.
 *
 * ===========================================================================
 *
 * WARNING: This is pre-alpha software and as such may well be incomplete,
 * unstable and unreliable. It is considered to be suitable only for
 * experimentation and nothing more.
 *
 * ===========================================================================
 */

// Module modFVM will provide an FVM implementation.
var modFVM = (function () { 'use strict';

  const WD_BYTES = 4
  const WD_BITS = WD_BYTES*8
  const MSb = 0x80000000|0 // Bit mask for most significant bit
  const IM = MSb
  const METADATA_MASK = 0x7fffffff|0 // 31 bits
  const OPCODE_MASK   = 0xff000000|0
  const BYTE_MASK     = 0x000000ff|0
  const SHIFT_MASK    = 0x0000001f|0
  const SUCCESS = 0
  const FAILURE = 1
  const ILLEGAL = 2
  const BEYOND = 3


  const INT_MAX =  2147483647;
  const INT_MIN = -2147483648;

  // Radical experiments (including making stacks generic)
  const PC_CELL = 0; // Cell to hold the PC!
  const SP_CELL = 1; // Cell to hold address of generic stack pointer!


  // Experimental robustness features:
  const SAFE_MARGIN = 2; // SAFE branches unless data stack has this free


  const MATH_OVERFLOW = 21
  const DS_UNDERFLOW = 31
  const DS_OVERFLOW = 32
  const RS_UNDERFLOW = 33
  const RS_OVERFLOW = 34
  const TS_UNDERFLOW = 35
  const TS_OVERFLOW = 36
  const CS_UNDERFLOW = 37
  const CS_OVERFLOW = 38


  const XS = 0x100 // 1 kB
  const  S = 0x1000 // 16 kB
  const  M = 0x10000 // 256 kB
  const  L = 0x100000 // 4 MB
  const XL = 0x1000000 // 64 MB
  const MAX_MEM_WORDS = S
  const PM_WORDS = MAX_MEM_WORDS
  const PM_MASK  = PM_WORDS-1
  const DM_WORDS = MAX_MEM_WORDS
  const DM_MASK  = DM_WORDS-1 // size: p3d3 (see '../../../README.md')

  const WORD_BYTES = 4;
  const WORD_PWR = 2; // FIXME necessary any more?
  const STACK_ELEMS = 256; //256; FIXME
  const STACK_BYTES = STACK_ELEMS << WORD_PWR;
  const STACK_1 = STACK_BYTES - WORD_BYTES;

  const PUSH  = 0x50000000|0
  const POP   = 0x51000000|0

  const TPUSH = 0x54000000|0
  const TPOP  = 0x55000000|0
  const TPEEK = 0x52000000|0
  const CPEEK = 0x53000000|0
  const TDROP = 0x59000000|0
  const CPUSH = 0x56000000|0
  const CPOP  = 0x57000000|0
  const CDROP = 0x58000000|0

  const NOP   = 0x00000000|0 // Simple


  const MUL   = 0x30000000|0
  const DIV   = 0x31000000|0
  const MOD   = 0x32000000|0

  const TRON  = 0x33000000|0 // FIXME find a better way than tron, troff
  const TROFF = 0x34000000|0

  const ADD   = 0x01000000|0
  const SUB   = 0x02000000|0
  const OR    = 0x03000000|0
  const AND   = 0x04000000|0
  const XOR   = 0x05000000|0
  const SHL   = 0x06000000|0
  const SHR   = 0x07000000|0
  const GET   = 0x08000000|0
  const PUT   = 0x09000000|0
  const GETI  = 0x0a000000|0
  const PUTI  = 0x0b000000|0
  const INCM  = 0x0c000000|0
  const DECM  = 0x0d000000|0

  const INC   = 0x10000000|0
  const DEC   = 0x11000000|0
  const FLIP  = 0x12000000|0
  const NEG   = 0x13000000|0

  const HALT  = 0x1c000000|0
  const JMPZ  = 0x1d000000|0 // Complex
  const JMPB  = 0x1e000000|0
  const JMPE  = 0x1f000000|0
  const JMPN  = 0x20000000|0
  const JMPG  = 0x21000000|0
  const JMPL  = 0x22000000|0
  const JUMP  = 0x23000000|0
  const RPT   = 0x24000000|0
  const IN    = 0x26000000|0
  const OUT   = 0x27000000|0

  const PMW   = 0x28000000|0
  const DMW   = 0x29000000|0

  const FAIL  = 0x40000000|0

  const CALL  = 0x60000000|0
  const RET   = 0x61000000|0

  const DROP  = 0x70000000|0
  const SWAP  = 0x71000000|0
  const OVER  = 0x72000000|0
  const ROT   = 0x73000000|0
  const DUP   = 0x74000000|0


  const SAFE  = 0x75000000|0
  const CATCH = 0x76000000|0

  const SP    = 0x79000000|0
  const LIT   = 0x80000000|0

  const SYMBOLS = {

    0x00000000: "nop  ",
    0x01000000: "add  ",
    0x02000000: "sub  ",


    0x30000000: "mul  ",
    0x31000000: "div  ",
    0x32000000: "mod  ",
    0x33000000: "tron ",
    0x34000000: "troff",

    0x03000000: "or   ",
    0x04000000: "and  ",
    0x05000000: "xor  ",
    0x06000000: "shl  ",
    0x07000000: "shr  ",
    0x08000000: "get  ",
    0x09000000: "put  ",
    0x0a000000: "geti ",
    0x0b000000: "puti ",
    0x0c000000: "incm ",
    0x0d000000: "decm ",

    0x10000000: "inc  ",
    0x11000000: "dec  ",
    0x12000000: "flip ",

    0x13000000: "neg  ",

    0x1c000000: "halt ",
    0x1d000000: "jmpz ",
    0x1e000000: "jmpb ",
    0x1f000000: "jmpe ",
    0x20000000: "jmpn ",
    0x21000000: "jmpg ",
    0x22000000: "jmpl ",
    0x23000000: "jump ",
    0x24000000: "rpt  ",
    0x26000000: "in   ",
    0x27000000: "out  ",

    0x28000000: "pmw  ",
    0x29000000: "dmw  ",

    0x40000000: "fail ",

    0x50000000: "push ",
    0x51000000: "pop  ",

    0x52000000: "tpeek",
    0x53000000: "cpeek",

    0x54000000: "tpush",
    0x55000000: "tpop ",
    0x59000000: "tdrop ",
    0x56000000: "cpush",
    0x57000000: "cpop ",
    0x58000000: "cdrop",

    0x60000000: "call ",
    0x61000000: "ret  ",

    0x70000000: "drop ",
    0x71000000: "swap ",
    0x72000000: "over ",
    0x73000000: "rot  ",
    0x74000000: "dup  ",

    0x75000000: "safe ",
    0x76000000: "catch",

    0x79000000: "sp   ",
    0x80000000: "lit  "

  };

  class FVM {
    constructor(config) {
      this.fnTrc = config.fnTrc;
      this.fnStdin = config.fnStdin;
      this.fnStdout = config.fnStdout;
      this.tracing = true; // comment this line out unless debugging

      this.tmp = 0|0; //tmp var only
      this.pm = new DataView(new ArrayBuffer(PM_WORDS*WD_BYTES)); // Harvard
      this.dm = new DataView(new ArrayBuffer(DM_WORDS*WD_BYTES)); // Harvard
      this.loadProgram(config.program, this.pm);

      // Radical experiment: move PC into cell 0 of data memory
      this.store(PC_CELL, 0|0); // Note: perhaps this is not of great worth
      // unless, like with SP_CELL, we make this a pointer to the PC
      // rather thank, as it is now, the PC itself. Why? Because
      // that would help with fast virtualisation.

/*
      this.rs = new Stack(this,RS_UNDERFLOW,RS_OVERFLOW); // return stack
      this.ds = new Stack(this,DS_UNDERFLOW,DS_OVERFLOW); // data stack
      this.ts = new Stack(this,TS_UNDERFLOW,TS_OVERFLOW); // temporary stack
      this.cs = new Stack(this,CS_UNDERFLOW,CS_OVERFLOW); // counter stack or repeat stack
*/
    };

    loadProgram(pgm, mem) {
      if (pgm === undefined) {
        this.loadedProg = false;
        this.fnTrc('No program to load');
      } else {
        var program = new DataView(pgm);
        for (var i = 0; i < program.byteLength; i++) {
          var w = program.getUint8(i, true);
          mem.setUint8(i, w, true);
        }
        this.loadedProg = true;
      }
    }

    initVM() {} // Nothing here yet

    safe(addr) { return addr & DM_MASK; }
    enbyte(x)  { return x & BYTE_MASK; }
    enrange(x) { return x & METADATA_MASK; }
    enshift(x) { return x & SHIFT_MASK; }

    run() {
      if (!this.loadedProg) {
        this.fnTrc('No program to run');
        return FAILURE;
      }

      this.initVM();
      while(true) {

        var addr, val;

        // Radical experiment (generic stack pointer for whole VM):
        var vmsp;

        // Radical experiment (PC in cell 0 of data memory)
        this.vZ = this.load(PC_CELL); // TODO refactor away this.vZ

        var instr = this.pmload(this.vZ);
        if (this.tracing) {
          this.traceVM(instr);
        }

        if (this.vZ >= PM_WORDS || this.vZ < 0 ) {
          return BEYOND; // TODO maybe make circular
        }

        ++this.vZ;
        this.store(PC_CELL, this.vZ);
        
        // Handle immediates
        if (instr&MSb) {
          // this.ds.doPush(instr&METADATA_MASK);

          // Radical experiment:
          //   - totally generic concept of stacks within program memory
          //   - note that this has no stack-overflow check yet!
          vmsp = this.load(SP_CELL);
          this.store(vmsp,instr&METADATA_MASK);
          vmsp -= 1;
          this.store(SP_CELL, vmsp);
          continue;
        }


        // Handle all other instructions
        var opcode = instr & OPCODE_MASK;

try {

        switch(opcode) { // TODO Fix order. FIXME negative opcodes not thrown

// This block is to allow the programmer to achieve robustness,
// but it is entirely the programmer's responsibility to do so.
          // Branch if ds does not have at least SAFE_MARGIN free
          case SAFE:  if (
            this.ds.free < SAFE_MARGIN
          ) {
             this.vZ = instr&PM_MASK;
             this.store(PC_CELL, this.vZ);
             break;
          }
// End of robustness block

// This block is all done except corner cases:
          case DROP:   this.ds.drop(); break;
          case SWAP:   this.ds.swap(); break;
          case OVER:   this.ds.over(); break;
          case ROT:    this.ds.rot(); break;
          case DUP:    this.ds.dup(); break;
          case TPUSH:  this.ts.doPush(this.ds.doPop()); break;
          case TPOP:   this.ds.doPush(this.ts.doPop()); break;
          case TPEEK:  this.ds.doPush(this.ts.doPeek()); break;
          case TDROP:  this.ts.drop(); break;
          case CPUSH:  this.cs.doPush(this.ds.doPop()); break;
          case CPOP:   this.ds.doPush(this.cs.doPop()); break;
          case CPEEK:  this.ds.doPush(this.cs.doPeek()); break;
          case CDROP:  this.cs.drop(); break;
          case RPT:    if (this.cs.gtOne()) {
                          this.cs.dec();
                          this.vZ = instr&PM_MASK;
                          this.store(PC_CELL, this.vZ);
                       } else {
                          this.cs.doPop();
                       }
                       break;
          /**/
          case CALL:   this.rs.doPush(this.vZ); this.vZ = instr&PM_MASK; this.store(PC_CELL, this.vZ); break;
          case RET:    this.vZ = this.rs.doPop(); this.store(PC_CELL, this.vZ); break;
          case NOP:    break;

          case CATCH:  break;

          case MUL:    this.ds.apply2((a,b) => a*b); break;
          // TODO Give 'proper' divide by zero trap for div, mod (not just math overflow)
          case DIV:    this.ds.apply2((a,b) => a/b); break;
          case MOD:    this.ds.apply2((a,b) => a%b); break;


          case TRON:   this.tracing = true; break;
          case TROFF:  this.tracing = false; break;


          case ADD:    this.ds.apply2((a,b) => a+b); break;
          case SUB:    this.ds.apply2((a,b) => a-b); break;
          case OR:     this.ds.apply2((a,b) => a|b); break;
          case AND:    this.ds.apply2((a,b) => a&b); break;
          case XOR:    this.ds.apply2((a,b) => a^b); break;
          case FLIP:   this.ds.apply1((a) => a^MSb); break;
          case NEG:    this.ds.apply1((a) => (~a)+1); break;


          case SHL:    this.ds.apply2((a,b) => a*Math.pow(2,this.enshift(b))); break;
          case SHR:    this.ds.apply2((a,b) => a>>>this.enshift(b)); break;
          case INC:    this.ds.apply1((a) => ++a); break;
          case DEC:    this.ds.apply1((a) => --a); break;
          // FIXME in theory OUT could branch on failure, should implement this
          case OUT:    this.fnStdout(this.enbyte(this.ds.doPop())); break;
          case IN:
              var inputChar = this.fnStdin();
              if (inputChar === undefined) { //FIXME use null not undefined for these
                  this.vZ = instr&PM_MASK; this.store(PC_CELL, this.vZ);
              } else {
                  this.ds.doPush(inputChar);
              }
              break;
          case GET:    this.ds.doPush(this.load(this.ds.doPop())); break;
          case PUT:    //this.store(this.ds.doPop(),this.ds.doPop()); break;
                        this.store(this.ppop(),this.ppop()); break
          case GETI:   this.ds.doPush(this.load(this.load(this.ds.doPop()))); break;
          case PUTI:   this.store(this.load(this.ds.doPop()),this.ds.doPop()); break;
          case INCM:   addr = this.ds.doPop();
                       this.store(addr,this.load(addr)+1); break;
          case DECM:   addr = this.ds.doPop();
                       this.store(addr,this.load(addr)-1); break;

          // TODO probably should add reverse-direction pop and push so as
          // to easily support bidirectional move and fill by use of rpt;
          // this is an alternative to adding CISC instructions.
          case POP:    addr = this.ds.doPop();
                       val = this.load(addr);
                       this.store(addr,val+1);
                       this.ds.doPush(this.load(val)); break;
          case PUSH:   addr = this.ds.doPop();
                       val = this.load(addr)-1;
                       this.store(addr,val);
                       this.store(val,this.ds.doPop()); break;

          case JUMP:   this.vZ = instr&PM_MASK; this.store(PC_CELL, this.vZ); break;
          case JMPE:   if (this.ds.doPop() == this.ds.doPop()) this.vZ = instr&PM_MASK;
                        this.store(PC_CELL, this.vZ); break;
          case JMPG:   if (this.ds.doPop() > this.ds.doPop()) this.vZ = instr&PM_MASK; 
                        this.store(PC_CELL, this.vZ); break;
          case JMPL:   if (this.ds.doPop() < this.ds.doPop()) this.vZ = instr&PM_MASK; 
                        this.store(PC_CELL, this.vZ); break;
          case JMPZ:   if (this.ds.doPop() == 0) this.vZ = instr&PM_MASK; 
                        this.store(PC_CELL, this.vZ); break;

          // Radical experiment; sets address of general stack-pointer cell
          case SP:     this.store(SP_CELL,instr&PM_MASK); break;

          case HALT:   return SUCCESS; break;
          case FAIL:   return FAILURE; break;
// End of done block

          default: return ILLEGAL; break;
        }
      } catch(e) {
          var nextInstr = this.pmload(this.vZ);
          var nextOpcode = nextInstr & OPCODE_MASK;
          if (nextOpcode == CATCH) {
            if (this.tracing) {
              this.traceVM(nextInstr, true);
            }
            this.vZ = nextInstr&PM_MASK; this.store(PC_CELL, this.vZ); break;
          } else {
            // FIXME need to go here if e is not a trap!
            return e;
          }
        }
      }
    }

    ppop() { // FIXME no underflow check here yet, not yet circular
      var val, sp;
      sp = this.load(SP_CELL);
      val = this.load(sp++);
      this.store(SP_CELL, sp);
      return val;
    }

    ppush(val) { // FIXME no overflow check here yet, not yet circular
      var sp = this.load(SP_CELL);
      this.store(val, sp--);
    }

    store(addr,val) {
      try {
        this.dm.setInt32(addr*WD_BYTES, val, true);
      } catch (e) {
        throw BEYOND;
      }
    }

    load(addr) {
      try {
        return this.dm.getUint32(addr*WD_BYTES, true);
      } catch (e) {
        throw BEYOND;
      }
    }

    pmload(addr) {
      return this.pm.getUint32(addr*WD_BYTES, true);
    }

    traceVM(instr, highlight) {
      var opcode = instr & OPCODE_MASK;
      var mnem = '?';
      if (opcode < 0) {
        mnem = SYMBOLS[0x80000000];
      } else {
        mnem = SYMBOLS[opcode];
      }
      if (highlight) {
        mnem = "*" + mnem;
      } else {
        mnem = " " + mnem;
      }
      var traceStr =
        "vZ:" + modFmt.hex8(this.vZ) +
        " c0:" + modFmt.hex8(this.load(PC_CELL)) + " " +
        modFmt.hex8(instr) + " " +
        mnem 
        // Radical experiment: // FIXME
        + " (sp " + modFmt.hex8(this.load(SP_CELL))
        + " ... " + modFmt.hex8(this.load(this.load(SP_CELL)+1)) + ", "
        + modFmt.hex8(this.load(this.load(SP_CELL)+2)) + " )";        


/*
        + " / " +
        this.cs + "/ ( " +
        this.ds + ") [ " +
        this.ts + "] { " +
        this.rs + "}";
*/
      this.fnTrc(traceStr);
    }
  }

/* Radical experiment above, pushing stacks within VM program memory
   rather than implementing a fixed number of stacks out here;
   hence commenting out this Stack class. */
/*
  class Stack {
    constructor(fvm, uerr, oerr) {
      this.uerr = uerr;
      this.oerr = oerr;
      this.fvm = fvm;
      this.elems = new DataView(new ArrayBuffer(STACK_BYTES));
      this.sp = STACK_BYTES;
    }

    used() {
      return (STACK_BYTES-this.sp)>>WORD_PWR;
    }

    free() {
      return STACK_ELEMS-this.used();
    }

    dec() {
      this.doReplace(this.doPeek()-1);
    }

    gtOne() {
      return this.doPeek() > 1;
    }

    doReplace(i) {
      if (this.sp <= STACK_1) {
        this.elems.setInt32(this.sp, i, true);
      } else {
        throw this.uerr; // underflow
      }
    }

    doPush(i) {
      if (this.sp >= WORD_BYTES) {
        this.sp = (this.sp-WORD_BYTES);
      } else {
        throw this.oerr; // overflow
        return;
      }
      this.elems.setInt32(this.sp, i, true);
    }

    drop() {
      if (this.sp <= STACK_1) {
        this.sp += WORD_BYTES;
      } else {
        throw this.uerr; // underflow
      }
    }

    swap() {
      if (this.sp <= STACK_1+1) {
        var a = this.elems.getInt32(this.sp, true);
        var b = this.elems.getInt32(this.sp+WORD_BYTES, true);
        this.elems.setInt32(this.sp, b, true);
        this.elems.setInt32(this.sp+WORD_BYTES, a, true);
      } else {
        throw this.uerr; // underflow
      }
    }

    over() {
      if (this.sp <= STACK_1-1) {
        var elem = this.elems.getInt32(this.sp+WORD_BYTES, true);
        this.doPush(elem);
      } else {
        throw this.uerr; // underflow
      }
    }

    rot() {
      if (this.sp <= STACK_1-2) {
        var a = this.elems.getInt32(this.sp+WORD_BYTES+WORD_BYTES, true);
        var b = this.elems.getInt32(this.sp+WORD_BYTES, true);
        var c = this.elems.getInt32(this.sp, true);
        this.elems.setInt32(this.sp, a, true);
        this.elems.setInt32(this.sp+WORD_BYTES, c, true);
        this.elems.setInt32(this.sp+WORD_BYTES+WORD_BYTES, b, true);
      } else {
        throw this.uerr; // underflow
      }
    }


    dup() {
      var elem = this.doPeek();
      this.doPush(elem);
    }

    doPop() {
      if (this.sp <= STACK_1) {
        var elem = this.elems.getInt32(this.sp, true);
        this.sp += WORD_BYTES;
        return elem;
      } else {
        throw this.uerr; // underflow
      }
    }

    doPeek() {
      if (this.sp <= STACK_1) {
        var elem = this.elems.getInt32(this.sp, true);
        return elem;
      } else {
        throw this.uerr; // underflow
      }
    }

    apply1(f) {
      var a = this.doPop();
      this.doPush(this.verify(f(a), a));
    }

    apply2(f) {
      var b = this.doPop();
      var a = this.doPop();
      this.doPush(this.verify(f(a,b), a, b));
    }

    verify(i, a , b) {
      if (i < INT_MIN || i > INT_MAX) {
        throw MATH_OVERFLOW;
      }
      return i;
    }

    toString() {
      var str = '';
      for (var i = STACK_1; i >= this.sp; i-=WORD_BYTES) {
        str = str + modFmt.hex8(this.elems.getUint32(i, true)) + ' ';
      }
      return str;
    }
  }
*/

  class Config {
    constructor(prg) {
      this.program = prg;
      this.fnStdin = null;
      this.fnStdout = null;
    };
  }

  return {
    makeFVM: function(config) {
      return new FVM(config);
    },
    makeConfig: function(prg) {
      return new Config(prg);
    }
  };

})(); // modFVM

var stdinBuf;   // TODO make per FVM instance rather than global
var stdinArray; // TODO make per FVM instance rather than global
function awaitReadStdin() {
  postMessage([1]);
  Atomics.wait(stdinArray,0,0); // TODO maybe add timeout here
  var inWord = Atomics.load(stdinArray,1);
  Atomics.store(stdinArray,0,0);  // Flips ready flag back to 0 = not ready
  return inWord;
}

function invokeRun(e) {
  console.log('Message received by FVM to invoke its run');
  var prg = e.data[1];
  var cf = modFVM.makeConfig(prg);
  cf.fnStdout = x => postMessage([0,x]);

  stdinBuf = e.data[2];
  stdinArray = new Int32Array(stdinBuf);
  cf.fnStdin = awaitReadStdin;

  cf.fnTrc = x => postMessage([2,x]);
  var exitCode = modFVM.makeFVM(cf).run();
  console.log('Posting message back from FVM at end of run');
  postMessage([3,exitCode]);
}

onmessage = function(e) {
  switch(e.data[0]) {
    // TODO Declare appropriate constants for message types
    case(0): // Message type 0 = invokeRun
      invokeRun(e);
      break;
    default:
      throw ("FIXME unhandled type of message to FVM");
      break;
  }
}

// Module modFmt provides formatting.
var modFmt = (function () { 'use strict';

  var hex8 = function(i) {
    if (typeof i === 'number') {
      return ('00000000' + i.toString(16)).substr(-8);
    } else {
      return '      ';
    }
  };

  return {
    hex8: hex8,
  };

})(); // modFmt





