/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 *
 * Program:    fvm2.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170303
 * Updated:    20180430+
 * Version:    pre-alpha-0.0.1.61+ for FVM 2.0
 *
 *                               This Edition:
 *                                JavaScript
 *                           for HTML 5 browsers
 *
 *                                ( ) [ ] { }
 *
 * 20180430: See 'fvm2-mainline.js' for the proper, parked version of this.
 * This 'fvm2.js' is now being hacked into a QMISC approach for comparison
 * prior to deciding which of the two competing strategies to pursue..
 *
 * When run as a Web Worker, if using Chromium, you must start the browser by:
 *    chromium --allow-file-access-from-files
 *
 * Uses SharedArrayBuffer (ES2017), so on Firefox 46-54 about:config needs:
 *    javascript.options.shared_memory = true 
 *
 * TODO
 * - think hard about assembly language format and forward references
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

  const MATH_OVERFLOW = 21

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
  const HD_WORDS = S; // Hold size (change this line as desired)
  const RM_WORDS = MAX_MEM_WORDS
  const RM_MASK  = RM_WORDS-1
  const LS_LNKTS = 0x100
  const vL_MAX   = LS_LNKTS-1

  const WORD_BYTES = 4;

  const FROMB = 0x77000000|0
  const TOB   = 0x78000000|0
  const TOT   = 0x79000000|0
  const TOR   = 0x7a000000|0

  const PUSH  = 0x7b000000|0
  const POP   = 0x7c000000|0

  const NOP   = 0x00000000|0
  const MUL   = 0x30000000|0
  const DIV   = 0x31000000|0
  const MOD   = 0x32000000|0
  const TRON  = 0x33000000|0
  const TROFF = 0x34000000|0
  const HOLD  = 0x35000000|0
  const GIVE  = 0x36000000|0

  const DECM  = 0x37000000|0
  const INCM  = 0x38000000|0

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
  const ROM   = 0x0c000000|0
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
  const PMI   = 0x28000000|0
  const DMW   = 0x29000000|0
  const RMW   = 0x41000000|0
  const HW    = 0x42000000|0
  const FAIL  = 0x40000000|0
  const CALL  = 0x60000000|0
  const RET   = 0x61000000|0

  const CATCH = 0x76000000|0
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

    0x35000000: "hold ",
    0x36000000: "give ",

    0x37000000: "decm ",
    0x38000000: "incm ",

    0x03000000: "or   ",
    0x04000000: "and  ",
    0x05000000: "xor  ",
    0x06000000: "shl  ",
    0x07000000: "shr  ",
    0x08000000: "get  ",
    0x09000000: "put  ",
    0x0a000000: "geti ",
    0x0b000000: "puti ",
    0x0c000000: "rom  ",

    0x10000000: "inc  ",
    0x11000000: "dec  ",
    0x12000000: "flip ",

    0x13000000: "neg  ",

    0x1c000000: "halt ",
    0x1d000000: "jmpz ",
    0x1f000000: "jmpe ",
    0x21000000: "jmpg ",
    0x22000000: "jmpl ",
    0x23000000: "jump ",
    0x24000000: "rpt  ",
    0x26000000: "in   ",
    0x27000000: "out  ",

    0x28000000: "pmi  ",
    0x29000000: "dmw  ",
    0x41000000: "rmw  ",
    0x42000000: "hw   ",
    0x40000000: "fail ",

    0x77000000: "fromb",
    0x78000000: "tob",
    0x79000000: "tot",
    0x7a000000: "tor",

    0x7b000000: "push",
    0x7c000000: "pop",

    0x60000000: "call ",
    0x61000000: "ret  ",

    0x76000000: "catch",

    0x80000000: "lit  "

  };

  class FVM {
    constructor(config) {
      this.fnTrc = config.fnTrc;
      this.fnStdin = config.fnStdin;
      this.fnStdout = config.fnStdout;
      this.tracing = false;
      this.vZ = 0|0; // program counter (not accessible)

      this.pm = new DataView(new ArrayBuffer(PM_WORDS*WD_BYTES)); // Harvard
      this.loadProgram(config.program, this.pm);

      this.dm = new DataView(new ArrayBuffer(DM_WORDS*WD_BYTES)); // Harvard

      this.rm = new DataView(new ArrayBuffer(RM_WORDS*WD_BYTES));
      // FIXME should be a loadRm function here but this will do for testing
      this.rmstore(0,0x41); // 'A'
      this.rmstore(1,0x42); // 'B'
      this.rmstore(2,0x43); // 'C'
      this.rmstore(0xfff,0x5a); // 'Z'

      this.hd = config.hold; // FIXME exceptions

      this.vA = 0|0;
      this.vB = 0|0;
      this.vT = 0|0;
      this.vR = 0|0;
      this.vL = 0|0; // link counter (not accessible)
      // link stack (not accessible)
      this.ls = new DataView(new ArrayBuffer(LS_LNKTS*WD_BYTES));

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

        var sB;

        var instr = this.pmload(this.vZ);
        if (this.tracing) {
          this.traceVM(instr);
        }

        if (this.vZ >= PM_WORDS || this.vZ < 0 ) {
          return BEYOND;
        }

        ++this.vZ;

        // Handle immediates
        if (instr&MSb) {
          this.vB = instr&METADATA_MASK;
          continue;
        }


        // Handle all other instructions
        var opcode = instr & OPCODE_MASK;

try {

        switch(opcode) { // TODO Fix order. FIXME negative opcodes not thrown
          // TODO Remove tracing opcodes, replace with a debugging solution
          case TRON:   this.tracing = true; break;
          case TROFF:  this.tracing = false; break;

          case FROMB:   this.vA = this.vB; break;
          case TOB:     this.vB = this.vA; break;
          case TOT:     this.vT = this.vA; break;
          case TOR:     this.vR = this.vA; break;

          case PMI:     this.vA = PM_WORDS; break;
          case DMW:     this.vA = DM_WORDS; break;
          case RMW:     this.vA = RM_WORDS; break;
          case HW:      this.vA = HD_WORDS; break;

          case RPT:    if (this.vR > 1) {
                          this.vR--;
                          this.vZ = instr&PM_MASK;
                       }
                       break;

          //case CALL:   this.rs.doPush(this.vZ); this.vZ = instr&PM_MASK; break;
          case CALL:   
            if (this.vL<vL_MAX) {
              this.lsstore(++(this.vL),this.vZ);
              this.vZ = instr&PM_MASK;
              break;
            } else {
              vA = FAILURE;
              return this.enbyte(this.vA);
            }
          // case RET:    this.vZ = this.rs.doPop(); break;
          case RET:
            if (this.vL>0) {
              this.vZ = this.lsload((this.vL)--)
              break;
            } else {
              vA = FAILURE;
              return this.enbyte(this.vA);
            }

          case NOP:    return this.enbyte(this.vA); break; // FIXME this is halt
          case CATCH:  break;
          case ADD:    break;
          case SUB:    break;
          case MUL:    break;
          case DIV:    break;
          case MOD:    break;
          case INC:    break;
          case DEC:    break;

          case OR:     /*this.ds.apply2((a,b) => a|b);*/ break;
          case AND:    /*this.ds.apply2((a,b) => a&b);*/ break;
          case XOR:    /*this.ds.apply2((a,b) => a^b);*/ break;
          case FLIP:   /*this.ds.apply1((a) => a^MSb);*/ break;
          case NEG:    /*this.ds.apply1((a) => (~a)+1);*/ break;
          case SHL:    /*this.ds.apply2((a,b) => a*Math.pow(2,this.enshift(b)));*/ break;
          case SHR:    /*this.ds.apply2((a,b) => a>>>this.enshift(b));*/ break;
          // FIXME in theory all I/O could branch on failure, should implement this
          case OUT:    this.fnStdout(this.enbyte(this.vA)); break;
          case IN:
              var inputChar = this.fnStdin();
              // Note: unfortunately 0 is used here to indicate
              // that no input is available. Have to live with this for now
              // until further refactoring is done.
              if (inputChar == 0) {
                  this.vZ = instr&PM_MASK;
              } else {
                  this.vA = inputChar&0xff;
              }
              break;
          case GIVE:   this.vA = this.give(this.vB); break; // FIXME these
          case HOLD:   this.hold(this.vB,this.vA); break;
          case ROM:    this.vA = (this.rmload(this.vB)); break;
          case GET:    this.vA=this.load(this.safe(this.vB)); break;
          case PUT:    this.store(this.safe(this.vB),this.vA); break;
          case GETI:   sB = this.safe(this.vB); this.vA = this.load(this.safe(this.load(sB))); break;
          case PUTI:   sB = this.safe(this.vB); this.store(this.safe(this.load(sB)),this.vA); break;
          // QMISC Note: no over/underflow checks here yet!
          // Which is the reason these were removed from the regular instruction set.
          case POP:    sB = this.safe(this.vB); this.vA = this.load(this.safe(this.load(sB)));
                       this.store(sB,this.load(sB)+1); break;
          case PUSH:   this.store(sB,this.load(sB)-1);
                       sB = this.safe(this.vB); this.store(this.safe(this.load(sB)),this.vA); break;
          case DECM:   sB = this.safe(this.vB); this.store(sB,this.load(sB)-1); break;
          case INCM:   sB = this.safe(this.vB); this.store(sB,this.load(sB)+1); break;
          case JUMP:   this.vZ = instr&PM_MASK; break;
          case HALT:   return this.enbyte(this.vA); break;
          default: return ILLEGAL; break;
        }
      } catch(e) {
          var nextInstr = this.pmload(this.vZ);
          var nextOpcode = nextInstr & OPCODE_MASK;
          if (nextOpcode == CATCH) {
            if (this.tracing) {
              this.traceVM(nextInstr, true);
            }
            this.vZ = nextInstr&PM_MASK;
          } else {
            // FIXME need to go here if e is not a trap!
            return e;
          }
        }
      }
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

    rmload(addr) {
      try {
        return this.rm.getUint32(addr*WD_BYTES, true);
      } catch (e) {
        throw BEYOND;
      }
    }

    // FIXME Add a better means of ROM (rm) initialization
    rmstore(addr,val) {
      try {
        this.rm.setInt32(addr*WD_BYTES, val, true);
      } catch (e) {
        throw 'ROM initialization failed' // FIXME
      }
    }

    lsload(addr) { // FIXME these
      try {
        return this.ls.getUint32(addr*WD_BYTES, true);
      } catch (e) {
        throw BEYOND;
      }
    }

    lsstore(addr,val) {
      try {
        this.ls.setInt32(addr*WD_BYTES, val, true);
      } catch (e) {
        throw BEYOND;
      }
    }

    hold(addr,val) {
      try {
        this.hd.setInt32(addr*WD_BYTES, val, true);
      } catch (e) {
        throw BEYOND; // FIXME branch don't trap
      }
    }

    give(addr) {
      try {
        return this.hd.getUint32(addr*WD_BYTES, true);
      } catch (e) {
        throw BEYOND; // FIXME branch don't trap
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
        modFmt.hex8(this.vZ) + " " +
        modFmt.hex8(instr) + " " +
        mnem +
        " vA:" + modFmt.hex8(this.vA) +
        " vB:" + modFmt.hex8(this.vB) +
        " vT:" + modFmt.hex8(this.vT) +
        " vR:" + modFmt.hex8(this.vR) +
        " vL:" + modFmt.hex8(this.vL) +
        " ls[0]:" + modFmt.hex8(this.lsload(0)) +
        " ls[1]:" + modFmt.hex8(this.lsload(1)) +
//        " ls[2]:" + modFmt.hex8(this.lsload(2)) +
        " dm[0x200]:" + modFmt.hex8(this.load(0x200)) +
        " dm[0x1ff]:" + modFmt.hex8(this.load(0x1ff));
//        " dm[0x1fe]:" + modFmt.hex8(this.load(0x1fe)) ;
      this.fnTrc(traceStr);
    }
  }

  class Config {
    constructor(prg, hold) {
      this.program = prg; // FIXME ? unused
      this.hold = hold;
      this.fnStdin = null;
      this.fnStdout = null;
    };
  }

  return {
    makeFVM: function(config) {
      return new FVM(config);
    },
    makeConfig: function(prg,hold) {
      return new Config(prg,hold);
    }
  };

})(); // modFVM

var stdinBuf;   // TODO make per FVM instance rather than global
var stdinArray; // TODO make per FVM instance rather than global
// FIXME NEXT more work on this function for when input unavailable
function awaitReadStdin() {
  postMessage([1]);
  Atomics.wait(stdinArray,0,0); // TODO maybe add timeout here
  var inWord = Atomics.load(stdinArray,1); // FIXME should be byte-based
  Atomics.store(stdinArray,0,0);  // Flips ready flag back to 0 = not ready
  return inWord;
}

function invokeRun(e) {
  console.log('Message received by FVM to invoke its run');
  var prg = e.data[1];
  var hold = e.data[3];
  var cf = modFVM.makeConfig(prg, hold);
  cf.fnStdout = x => postMessage([0,x]);

  stdinBuf = e.data[2];
  stdinArray = new Int32Array(stdinBuf);
  cf.fnStdin = awaitReadStdin;

  cf.fnTrc = x => postMessage([2,x]);
  var exitCode = modFVM.makeFVM(cf).run();

  // FIXME hold: consider situation if run stopped with exception
  console.log('Posting message back from FVM at end of run');
  postMessage([3,exitCode,hold]);
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





