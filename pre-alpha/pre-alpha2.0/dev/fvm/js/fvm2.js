/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 *
 * Program:    fvm2.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170303
 * Updated:    20180515+
 * Version:    pre-alpha-0.0.1.64+ for FVM 2.0
 *
 *                               This Edition:
 *                                JavaScript
 *                           for HTML 5 browsers
 *
 *                                ( ) [ ] { }
 *
 * See 'pre-alpha/pre-alpha2.0/README.md' for the proposed design.
 * This FVM 2.0 implementation is incomplete but has proven the concept.
 *
 * When run as a Web Worker, if using Chromium, you must start the browser by:
 *    chromium --allow-file-access-from-files
 *
 * Uses SharedArrayBuffer (ES2017), so on Firefox 46-54 about:config needs:
 *    javascript.options.shared_memory = true
 *
 * WARNING: The new instruction set has just been ported from 'fvm2.c'
 * to this 'fvm2.js' implementation and is largely untested.
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
  const METADATA_MASK = 0x7fffffff|0 // 31 bits
  const OPCODE_MASK   = 0xff000000|0
  const BYTE_MASK     = 0x000000ff|0
  const SHIFT_MASK    = 0x0000001f|0
  const SUCCESS = 0
  const FAILURE = 1
  const ILLEGAL = 2
  const BEYOND = FAILURE

  const INT_MAX =  2147483647;
  const INT_MIN = -2147483648;
  const NAN = 0x80000000

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

  const WORD_BYTES = 4;
  const WORD_PWR = 2;
  const STACK_ELEMS = 256;
  const STACK_BYTES = STACK_ELEMS << WORD_PWR;
  const STACK_1 = STACK_BYTES - WORD_BYTES;

  // FIXME rationalize opcode order (these initial allocations are arbitrary)
  const NOP   = 0x00000000|0
  const DO    = 0x01000000|0
  const DONE  = 0x02000000|0
  const RPT   = 0x03000000|0
  const CPUSH = 0x04000000|0
  const CPOP  = 0x05000000|0
  const CPEEK = 0x06000000|0
  const CDROP = 0x07000000|0
  const TPUSH = 0x08000000|0
  const TPOP  = 0x09000000|0
  const TPEEK = 0x0a000000|0
  const TPOKE = 0x0b000000|0
  const TDROP = 0x0c000000|0
  const IM    = 0x80000000|0  // note: 0x0000000d available
  const DROP  = 0x0e000000|0
  const SWAP  = 0x0f000000|0
  const OVER  = 0x10000000|0
  const ROT   = 0x11000000|0
  const DUP   = 0x12000000|0
  const GET   = 0x13000000|0
  const PUT   = 0x14000000|0
  const GETI  = 0x15000000|0
  const PUTI  = 0x16000000|0
  const ROM   = 0x17000000|0
  const ADD   = 0x18000000|0
  const SUB   = 0x19000000|0
  const MUL   = 0x1a000000|0
  const DIV   = 0x1b000000|0
  const MOD   = 0x1c000000|0
  const INC   = 0x1d000000|0
  const DEC   = 0x1e000000|0
  const OR    = 0x1f000000|0
  const AND   = 0x20000000|0
  const XOR   = 0x21000000|0
  const FLIP  = 0x22000000|0
  const NEG   = 0x23000000|0
  const SHL   = 0x24000000|0
  const SHR   = 0x25000000|0
  const HOLD  = 0x26000000|0
  const GIVE  = 0x27000000|0
  const IN    = 0x28000000|0
  const OUT   = 0x29000000|0
  const INW   = 0x2a000000|0
  const OUTW  = 0x2b000000|0
  const JUMP  = 0x2c000000|0
  const JNAN  = 0x2d000000|0
  const JORN  = 0x2e000000|0
  const JANN  = 0x2f000000|0
  const JNNE  = 0x30000000|0
  const JMPE  = 0x31000000|0
  const JNNZ  = 0x32000000|0
  const JNNP  = 0x33000000|0
  const JNNG  = 0x34000000|0
  const JNNL  = 0x35000000|0
  const HALT  = 0x36000000|0
  const FAIL  = 0x37000000|0
  const SAFE =  0x38000000|0
  const DSA   = 0x39000000|0
  const DSE   = 0x3a000000|0
  const TSA   = 0x3b000000|0
  const TSE   = 0x3c000000|0
  const CSA   = 0x3d000000|0
  const CSE   = 0x3e000000|0
  const RSA   = 0x3f000000|0
  const RSE   = 0x40000000|0
  const PMI   = 0x41000000|0
  const DMW   = 0x42000000|0
  const RMW   = 0x43000000|0
  const HW    = 0x44000000|0
  const TRON  = 0x45000000|0
  const TROFF = 0x46000000|0


  const SYMBOLS = {

    0x00000000: "noop ",
    0x01000000: "do   ",
    0x02000000: "done ",
    0x03000000: "rpt  ",
    0x04000000: "cpush",
    0x05000000: "cpop ",
    0x06000000: "cpeek",
    0x07000000: "cdrop",
    0x08000000: "tpush",
    0x09000000: "tpop ",
    0x0a000000: "tpeek",
    0x0b000000: "tpoke",
    0x0c000000: "tdrop",
    0x80000000: "lit  ", // FIXME
    0x0e000000: "drop ",
    0x0f000000: "swap ",
    0x10000000: "over ",
    0x11000000: "rot  ",
    0x12000000: "dup  ",
    0x13000000: "get  ",
    0x14000000: "put  ",
    0x15000000: "geti ",
    0x16000000: "puti ",
    0x17000000: "rom  ",
    0x18000000: "add  ",
    0x19000000: "sub  ",
    0x1a000000: "mul  ",
    0x1b000000: "div  ",
    0x1c000000: "mod  ",
    0x1d000000: "inc  ",
    0x1e000000: "dec  ",
    0x1f000000: "or   ",
    0x20000000: "and  ",
    0x21000000: "xor  ",
    0x22000000: "flip ",
    0x23000000: "neg  ",
    0x24000000: "shl  ",
    0x25000000: "shr  ",
    0x26000000: "hold ",
    0x27000000: "give ",
    0x28000000: "in   ",
    0x29000000: "out  ",
    0x2a000000: "inw  ",
    0x2b000000: "outw ",
    0x2c000000: "jump ",
    0x2d000000: "jnan ",
    0x2e000000: "jorn ",
    0x2f000000: "jann ",
    0x30000000: "jnne ",
    0x31000000: "jmpe ",
    0x32000000: "jnnz ",
    0x33000000: "jnnp ",
    0x34000000: "jnng ",
    0x35000000: "jnnl ",
    0x36000000: "halt ",
    0x37000000: "fail ",
    0x38000000: "safe ",
    0x39000000: "dsa  ",
    0x3a000000: "dse  ",
    0x3b000000: "tsa  ",
    0x3c000000: "tse  ",
    0x3d000000: "csa  ",
    0x3e000000: "cse  ",
    0x3f000000: "rsa  ",
    0x40000000: "rse  ",
    0x41000000: "pmi  ",
    0x42000000: "dmw  ",
    0x43000000: "rmw  ",
    0x44000000: "hw   ",
    0x45000000: "tron ",
    0x46000000: "troff"
  };

  class FVM {
    constructor(config) {
      this.fnTrc = config.fnTrc;
      this.fnStdin = config.fnStdin;
      this.fnStdout = config.fnStdout;
      this.tracing = false;
      this.vZ = 0|0; // program counter (not accessible) (maybe it should be)
      this.tmp = 0|0; //tmp var only
      this.pm = new DataView(new ArrayBuffer(PM_WORDS*WD_BYTES)); // Harvard
      this.dm = new DataView(new ArrayBuffer(DM_WORDS*WD_BYTES)); // Harvard
      this.rm = new DataView(new ArrayBuffer(RM_WORDS*WD_BYTES));

      // FIXME should be a loadRm function here but this will do for testing
      this.rmstore(0,0x41); // 'A'
      this.rmstore(1,0x42); // 'B'
      this.rmstore(2,0x43); // 'C'
      this.rmstore(0xfff,0x5a); // 'Z'

      this.loadProgram(config.program, this.pm);

      this.hd = config.hold; // FIXME exceptions

      this.rs = new Stack(this,FAILURE,FAILURE); // return stack
      this.ds = new Stack(this,FAILURE,FAILURE); // data stack
      this.ts = new Stack(this,FAILURE,FAILURE); // temporary stack
      this.cs = new Stack(this,FAILURE,FAILURE); // counter stack or repeat stack
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

        var addr, val, n1, n2;

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
          this.ds.doPush(instr&METADATA_MASK);
          continue;
        }


        // Handle all other instructions
        var opcode = instr & OPCODE_MASK;

try {

        switch(opcode) { // TODO Fix order. FIXME negative opcodes not thrown
          case DSA:     this.ds.doPush(this.ds.free()); break;
          case DSE:     this.ds.doPush(this.ds.used()); break;
          case TSA:     this.ds.doPush(this.ts.free()); break;
          case TSE:     this.ds.doPush(this.ts.used()); break; 
          case CSA:     this.ds.doPush(this.cs.free()); break;
          case CSE:     this.ds.doPush(this.cs.used()); break; 
          case RSA:     this.ds.doPush(this.rs.free()); break;
          case RSE:     this.ds.doPush(this.rs.used()); break;
          case PMI:     this.ds.doPush(PM_WORDS); break;
          case DMW:     this.ds.doPush(DM_WORDS); break;
          case RMW:     this.ds.doPush(RM_WORDS); break;
          case HW:      this.ds.doPush(HD_WORDS); break;
          case DROP:   this.ds.drop(); break;
          case SWAP:   this.ds.swap(); break;
          case OVER:   this.ds.over(); break;
          case ROT:    this.ds.rot(); break;
          case DUP:    this.ds.dup(); break;
          case TPUSH:  this.ts.doPush(this.ds.doPop()); break;
          case TPOP:   this.ds.doPush(this.ts.doPop()); break;
          case TPEEK:  this.ds.doPush(this.ts.doPeekAt(this.ds.doPop())); break;
          case TPOKE:  this.ts.doPokeAt(this.ds.doPop(),this.ds.doPop()); break;
          case TDROP:  this.ts.drop(); break;
          case CPUSH:  this.cs.doPush(this.ds.doPop()); break;
          case CPOP:   this.ds.doPush(this.cs.doPop()); break;
          case CPEEK:  this.ds.doPush(this.cs.doPeek()); break;
          case CDROP:  this.cs.drop(); break;
          case RPT:    if (this.cs.gtOne()) {
                          this.cs.dec();
                          this.vZ = instr&PM_MASK;
                       } else {
                          this.cs.doPop();
                       }
                       break;
          case DO:     this.rs.doPush(this.vZ); this.vZ = instr&PM_MASK; break;
          case DONE:   this.vZ = this.rs.doPop(); break;
          case NOP:    break;
          case SAFE:   if(this.ds.used() < 2) this.vZ = instr&PM_MASK; break;
          case ADD:    this.ds.apply2((a,b) => a+b); break;
          case SUB:    this.ds.apply2((a,b) => a-b); break;
          case MUL:    this.ds.apply2((a,b) => a*b); break;
          case DIV:    var b = this.doPop();
                       var a = this.doPop();
                       if (a == NAN || b == NAN || b == 0) {
                         this.doPush(NAN);
                       } else {
                         this.doPush(this.verify((a/b)|0));
                       }
                       break;
          // FIXME this might be implementation-defined behaviour?
          // FIXME also, in general, endianness of JavaScript on platforms.
          case MOD:    var b = this.doPop();
                       var a = this.doPop();
                       if (a == NAN || b == NAN || b == 0) {
                         this.doPush(NAN);
                       } else {
                         this.doPush(this.verify((a%b)|0));
                       }
                       break;
          case INC:    this.ds.apply1((a) => ++a); break;
          case DEC:    this.ds.apply1((a) => --a); break;
          // TODO Remove tracing opcodes, replace with a debugging solution
          case TRON:   this.tracing = true; break;
          case TROFF:  this.tracing = false; break;
          case OR:     this.ds.apply2((a,b) => a|b); break;
          case AND:    this.ds.apply2((a,b) => a&b); break;
          case XOR:    this.ds.apply2((a,b) => a^b); break;
          case FLIP:   this.ds.apply1((a) => a^MSb); break;
          case NEG:    this.ds.apply1((a) => (~a)+1); break;
          case SHL:    this.ds.apply2((a,b) => a*Math.pow(2,this.enshift(b))); break;
          case SHR:    this.ds.apply2((a,b) => a>>>this.enshift(b)); break;
          // FIXME in theory all I/O could branch on failure, should implement this
          case OUT:    this.fnStdout(this.enbyte(this.ds.doPop()&0xff)); break;
          case IN:
              var inputChar = this.fnStdin();
              // Note: unfortunately 0 is used here to indicate
              // that no input is available. Have to live with this for now
              // until further refactoring is done.
              if (inputChar == 0) {
                  this.vZ = instr&PM_MASK;
              } else {
                  this.ds.doPush(inputChar&0xff);
              }
              break;
          // FIXME endianness of outw, inw
          case OUTW:    this.fnStdout(this.ds.doPop()); break;
          case INW:
              var inputChar = this.fnStdin(); // FIXME this is not a word yet!
              // Note: unfortunately 0 is used here to indicate
              // that no input is available. Have to live with this for now
              // until further refactoring is done.
              if (inputChar == 0) {
                  this.vZ = instr&PM_MASK;
              } else {
                  this.ds.doPush(inputChar);
              }
              break;
          case GIVE:   this.ds.doPush(this.give(this.ds.doPop())); break;
          case HOLD:   this.hold(this.ds.doPop(),this.ds.doPop()); break;
          case ROM:    this.ds.doPush(this.rmload(this.ds.doPop())); break;
          case GET:    this.ds.doPush(this.load(this.ds.doPop())); break;
          case PUT:    this.store(this.ds.doPop(),this.ds.doPop()); break;
          case GETI:   this.ds.doPush(this.load(this.load(this.ds.doPop()))); break;
          case PUTI:   this.store(this.load(this.ds.doPop()),this.ds.doPop()); break;
          case JUMP:   this.vZ = instr&PM_MASK; break;
          // Jump if n is NaN. Preserve n.
          case JNAN:    if (this.ds.doPeek() == NAN) {
                          this.vZ = instr&PM_MASK;
                        }
                        break;
          // Jump if either n1 or n2 is NaN. Preserve both.
          case JORN:    n1 = this.ds.doPeek();
                        n2 = this.ds.doPeekAt(2);
                        if ((n1 == NAN) || (n2 == NAN )) {
                          this.vZ = instr&PM_MASK;
                        }
                        break;
          // Jump if n1 and n2 are both NaN. Preserve both.
          case JANN:    n1 = this.ds.doPeek();
                        n2 = this.ds.doPeekAt(2);
                        if ((n1 == NAN) && (n2 == NAN)) {
                          this.vZ = instr&PM_MASK;
                        }
                        break;
          // Jump if n2 == n1 and neither are NaN. Preserve n2.
          case JNNE:    n1 = this.ds.doPeek();
                        n2 = this.ds.doPeekAt(2);
                        if ((n1 != NAN) && (n2 != NAN) && (n1 == n2)) {
                          this.vZ = instr&PM_MASK;
                        }
                        break;
          // Jump if n2 == n1 (even if they are NaN). Preserve n2.
          case JMPE:    if (this.ds.doPop() == this.ds.doPeek()) {
                          this.vZ = instr&PM_MASK;
                        }
                        break;
          // Jump if n is 0. Preserve n.
          case JNNZ:    if (this.ds.doPeek() == 0) {
                          this.vZ = instr&PM_MASK;
                        }
                        break;
          // Jump if n > 0 and not NaN. Preserve n.
          case JNNP:    n1 = this.ds.doPeek();
                        if ((n1 != NAN) && (n1 > 0)) {
                          this.vZ = instr&PM_MASK;
                        }
                        break;
          
          // Jump if n2 > n1 and neither are NaN. Preserve n2.
          case JNNG:    n1 = this.ds.doPop();
                        n2 = this.ds.doPeek();
                        if ((n1 != NAN) && (n2 != NAN) && (n1 < n2)) {
                          this.vZ = instr&PM_MASK;
                        }
                        break;
          // Jump if n2 < n1 and neither are NaN. Preserve n2.
          case JNNL:    n1 = this.ds.doPop();
                        n2 = this.ds.doPeek();
                        if ((n1 != NaN) && (n2 != NaN) && (n1 > n2)) {
                          this.vZ = instr&PM_MASK;
                        }
                        break;
          case HALT:   return SUCCESS; break;
          case FAIL:   return FAILURE; break;
          default: return ILLEGAL; break;
        }
      } catch(e) {
          return e;
        }
      }
    }

    store(addr,val) {
      if(addr == NAN) throw BEYOND;
      try {
        this.dm.setInt32(addr*WD_BYTES, val, true);
      } catch (e) {
        throw BEYOND;
      }
    }

    load(addr) {
      if(addr == NAN) throw BEYOND;
      try {
        return this.dm.getUint32(addr*WD_BYTES, true);
      } catch (e) {
        throw BEYOND;
      }
    }

    rmload(addr) {
      if(addr == NAN) throw BEYOND;
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

    hold(addr,val) {
      if(addr == NAN) throw BEYOND;
      try {
        this.hd.setInt32(addr*WD_BYTES, val, true);
      } catch (e) {
        throw BEYOND; // FIXME branch don't trap
      }
    }

    give(addr) {
      if(addr == NAN) throw BEYOND;
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
        mnem + " / " +
        this.cs + "/ ( " +
        this.ds + ") [ " +
        this.ts + "] { " +
        this.rs + "}";
      this.fnTrc(traceStr);
    }
  }

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
      wd = this.doPeek();
      if (wd != 0 && wd !=NAN) {
        --wd;
      }
      this.doReplace(wd);
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

    doPeekAt(elemNum) {
      if (elemNum < 1 || elemNum > STACK_ELEMS) {
        throw this.uerr; // underflow
      }
      var elemAddr = (this.sp+((elemNum-1)*WORD_BYTES));
      if (elemAddr <= STACK_1) {
        var elem = this.elems.getInt32(elemAddr, true);
        return elem;
      } else {
        throw this.uerr; // underflow
      }
    }

    doPokeAt(elemNum, val) {
      if (elemNum == NAN || elemNum < 1 || elemNum > STACK_ELEMS) {
        throw this.uerr; // underflow
      }
      var elemAddr = (this.sp+((elemNum-1)*WORD_BYTES));
      if (elemAddr <= STACK_1) {
        this.elems.setInt32(elemAddr, val, true);
      } else {
        throw this.uerr; // underflow
      }
    }

    apply1(f) {
      var a = this.doPop();
      if (a == NAN) {
        this.doPush(NAN);
      } else {
        this.doPush(this.verify(f(a), a));
      }
    }

    apply2(f) {
      var b = this.doPop();
      var a = this.doPop();
      if (a == NAN || b == NAN) {
        this.doPush(NAN);
      } else {
        this.doPush(this.verify(f(a,b), a, b));
      }
    }

    verify(i, a , b) {
      if (i < INT_MIN || i > INT_MAX) {
        return NAN;
      }
      return i;
    }

/* // Classic: topmost element to the right
    toString() {
      var str = '';
      for (var i = STACK_1; i >= this.sp; i-=WORD_BYTES) {
        str = str + modFmt.hex8(this.elems.getUint32(i, true)) + ' ';
      }
      return str;
    }
*/

    // Modern: topmost element to the left
    toString() {
      var str = '';
      for (var i = this.sp; i <= STACK_1; i+=WORD_BYTES) {
        str = str + modFmt.hex8(this.elems.getUint32(i, true)) + ' ';
      }
      return str;
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
try {
  // For use by Node.js only
  if(exports) {
    exports.makeFVM = modFVM.makeFVM
    exports.makeConfig = modFVM.makeConfig
  }
} catch(e) {}

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
