/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvm2.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170303
 * Updated:    20170923+
 * Version:    pre-alpha-0.0.1.12+ for FVM 2.0
 *
 *                               This Edition:
 *                                JavaScript
 *                           for HTML 5 browsers
 * 
 *                                ( ) [ ] { }
 *
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
  const MSb = 0x80000000 // Bit mask for most significant bit
  const IM = MSb
  const METADATA_MASK = 0x7fffffff // 31 bits
  const OPCODE_MASK   = 0xff000000
  const BYTE_MASK     = 0x000000ff
  const SHIFT_MASK    = 0x0000001f
  const SUCCESS = 0
  const FAILURE = 1
  const ILLEGAL = 2
  const BEYOND = 3
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

  const NOP   = 0x00000000|0 // Simple
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
  const AT    = 0x0e000000|0
  const INC   = 0x10000000|0
  const DEC   = 0x11000000|0
  const FLIP  = 0x12000000|0
  const SWAP  = 0x13000000|0
  const TOB   = 0x14000000|0
  const TOR   = 0x15000000|0
  const TOT   = 0x16000000|0
  const FROMB = 0x17000000|0
  const FROMR = 0x18000000|0
  const FROMT = 0x19000000|0
  const MEM   = 0x1a000000|0
  const LINK  = 0x1b000000|0
  const HALT  = 0x1c000000|0
  const JMPA  = 0x1d000000|0 // Complex
  const JMPB  = 0x1e000000|0
  const JMPE  = 0x1f000000|0
  const JMPN  = 0x20000000|0
  const JMPG  = 0x21000000|0
  const JMPL  = 0x22000000|0
  const JUMP  = 0x23000000|0
  const RPT   = 0x24000000|0
  const BR    = 0x25000000|0
  const IN    = 0x26000000|0
  const OUT   = 0x27000000|0

  const SYMBOLS = {
  
    0x00000000: "nop  ",
    0x01000000: "add  ",
    0x02000000: "sub  ",
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
    0x0e000000: "at   ",
    0x10000000: "inc  ",
    0x11000000: "dec  ",
    0x12000000: "flip ",
    0x13000000: "swap ",
    0x14000000: "tob  ",
    0x15000000: "tor  ",
    0x16000000: "tot  ",
    0x17000000: "fromb",
    0x18000000: "fromr",
    0x19000000: "fromt",
    0x1a000000: "mem  ",
    0x1b000000: "link ",
    0x1c000000: "halt ",
    0x1d000000: "jmpa ",
    0x1e000000: "jmpb ",
    0x1f000000: "jmpe ",
    0x20000000: "jmpn ",
    0x21000000: "jmpg ",
    0x22000000: "jmpl ",
    0x23000000: "jump ",
    0x24000000: "rpt  ",
    0x25000000: "br   ",
    0x26000000: "in   ",
    0x27000000: "out  ",
    "-2147483648": "imm  "
  };

  class FVM {
    constructor(config) {
      this.fnTrc = config.fnTrc;
      this.fnStdout = config.fnStdout;
      this.tracing = true; // comment this line out unless debugging
      this.vA = 0|0; // accumulator
      this.vB = 0|0; // operand register
      this.vT = 0|0; // temporary register
      this.vR = 0|0; // repeat register
      this.vL = 0|0; // link register (not accessible)
      this.vZ = 0|0; // program counter (not accesible) (maybe it should be)
      this.pm = new DataView(new ArrayBuffer(PM_WORDS)); // Harvard
      this.dm = new DataView(new ArrayBuffer(DM_WORDS)); // Harvard
      this.loadProgram(config.program, this.pm);
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
      this.initVM();
      while(true) {
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
          this.vB = this.enrange(instr);
          continue;
        }

        // Handle all other instructions
        var opcode = instr & OPCODE_MASK;
        switch(opcode) { // TODO Fix order. FIXME negative opcodes not thrown
          case NOP:    break;
          case ADD:    this.vA+=this.vB; break;
          case SUB:    this.vA-=this.vB; break;
          case OR:     this.vA|=this.vB; break;
          case AND:    this.vA&=this.vB; break;
          case XOR:    this.vA^=this.vB; break;
          case FLIP:   this.vB = this.vB^MSb; break;
          case SHL:    this.vA<<=enshift(this.vB); break;
          case SHR:    this.vA>>=enshift(this.vB); break;
          case GET:    if (this.vB&DM_MASK!=0) return BEYOND;
                       this.vA = this.load(this.vB); break;
          case PUT:    if (this.vB&DM_MASK!=0) return BEYOND;
                       this.store(this.vB, this.vA); break;
          case GETI:   if (this.vB&DM_MASK!=0) return BEYOND;
                       sI = this.load(vB);
                       if (sI&DM_MASK!=0) return BEYOND;
                       this.vA = this.load(sI); break;
          case PUTI:   if (this.vB&DM_MASK!=0) return BEYOND;
                       sI = this.load(vB);
                       if (sI&DM_MASK!=0) return BEYOND;
                       this.store(sI, this.vA); break;
          case INCM:   if (this.vB&DM_MASK!=0) return BEYOND;
                       val = this.load(vB); this.store(vB, ++val); break;
          case DECM:   if (this.vB&DM_MASK!=0) return BEYOND;
                       val = this.load(vB); this.store(vB, --val); break;
          case AT:     if (this.vB&DM_MASK!=0) return BEYOND;
                       this.vB = this.load(this.vB); break;
          case INC:    ++this.vB; break;
          case DEC:    --this.vB; break;
          case SWAP:   this.vB = this.vB^this.vA;
                       this.vA = this.vA^this.vB;
                       this.vB = this.vB^this.vA; break;
          case TOB:    this.vB = this.vA; break;
          case TOR:    this.vR = this.vA; break;
          case TOT:    this.vT = this.vA; break;
          case FROMB:  this.vA = this.vB; break;
          case FROMR:  this.vA = this.vR; break;
          case FROMT:  this.vA = this.vT; break;
          case JMPA:   if (this.vA == 0) this.vZ = instr&PM_MASK; break;
          case JMPB:   if (this.vB == 0) this.vZ = instr&PM_MASK; break;
          case JMPE:   if (this.vA == this.vB) this.vZ = instr&PM_MASK; break;
          case JMPN:   if (MSb == (this.vA&MSb)) this.vZ = instr&PM_MASK; break;
          case JMPG:   if (this.vA > this.vB) this.vZ = instr&PM_MASK; break;
          case JMPL:   if (this.vA < this.vB) this.vZ = instr&PM_MASK; break;
          case JUMP:   this.vZ = instr&PM_MASK; break;
          case RPT:    if ( this.vR != 0) { --this.vR; this.vZ = instr&PM_MASK; } break;
          case BR:     this.vL = this.vZ; this.vZ = instr&PM_MASK; break;
          case LINK:   this.vZ = this.vL; break;
          case MEM:    this.vA = PM_WORDS; break;
          case IN:     this.vA = this.fnStdin(); break; // FIXME
          case OUT:    this.fnStdout(this.enbyte(this.vA)); break; // FIXME
          case HALT:   this.vB = this.enbyte(this.vB); return this.vB; break;
          default: return ILLEGAL; break;
        }
      }
    }

    store(addr,val) {
      this.dm.setInt32(addr*WD_BYTES, val, true);
    }

    load(addr) {
      return this.dm.getUint32(addr*WD_BYTES, true);
    }

    pmload(addr) {
      return this.pm.getUint32(addr*WD_BYTES, true);
    }

    traceVM(instr) {
      var opcode = instr & OPCODE_MASK;
      var traceStr =
        modFmt.hex8(this.vZ) + " " +
        modFmt.hex8(instr) + " " +
        SYMBOLS[opcode] + " " +
        "vA:" + modFmt.hex8(this.vA) + " " +
        "vB:" + modFmt.hex8(this.vB) + " " +
        "vT:" + modFmt.hex8(this.vT) + " " +
        "vR:" + modFmt.hex8(this.vR) + " " +
        "vL:" + modFmt.hex8(this.vL);
      this.fnTrc(traceStr);
    }
  }

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





