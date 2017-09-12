/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvm2.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170303
 * Updated:    20170912+
 * Version:    pre-alpha-0.0.1.1+ for FVM 2.0
 *
 *                               This Edition:
 *                                JavaScript
 *                           for HTML 5 browsers
 * 
 *                                ( ) [ ] { }
 *
 *  Status: In progress of porting 'qmisc.c' to JavaScript.
 *  Untested. Incomplete. Non-functional. Do not run this yet.
*   For notes see 'qmisc.c'.
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
  const MAX_MEM_WORDS = 0x1000
  const MEM_WORDS = MAX_MEM_WORDS
  const MEM_MASK  = MEM_WORDS-1

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
  const COPY  = 0x0f000000|0
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
  const JMPS  = 0x21000000|0
  const JMPU  = 0x22000000|0
  const JUMP  = 0x23000000|0
  const RPT   = 0x24000000|0
  const BR    = 0x25000000|0
  const IN    = 0x26000000|0
  const OUT   = 0x27000000|0

  class FVM {
    constructor(config) {
      this.tracing = true; // comment this line out unless debugging
      this.vA = 0|0; // accumulator
      this.vB = 0|0; // operand register
      this.vT = 0|0; // temporary register
      this.vR = 0|0; // repeat register
      this.vL = 0|0; // link register (not accessible)
      this.vZ = 0|0; // program counter (not accesible) (maybe it should be)
      this.mem = new DataView(new ArrayBuffer(MEM_WORDS)); // Von Neumann
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

    safe(addr) { return addr & MEM_MASK; }
    enbyte(x)  { return x & BYTE_MASK; }
    enrange(x) { return x & METADATA_MASK; }
    enshift(x) { return x & SHIFT_MASK; }

    runVM() {
      initVM();
      loadProgram();
      while(true) {
        WORD instr = this.load(vZ);
        if (this.tracing) {
          traceVM();
        }

        vZ = safe(++vZ);
        // Handle immediates
        if (instr&MSb) {
          vB = enrange(x);
          continue;
        }

        // Handle all other instructions
        WORD opcode = instr & OPCODE_MASK;
        switch(opcode) { // TODO Fix order
          case NOP:    break;
          case ADD:    vA+=vB; break;
          case SUB:    vA-=vB; break;
          case OR:     vA|=vB; break;
          case AND:    vA&=vB; break;
          case XOR:    vA^=vB; break;
          case FLIP:   vB = vB^MSb; break;
          case SHL:    vA<<=enshift(vB); break;
          case SHR:    vA>>=enshift(vB); break;
          case GET:    vA = this.load(safe(vB)); break;
          case PUT:    this.store(safe(vB), vA); break;
          case GETI:   sB = safe(vB); vA = this.load(safe(this.load(sB))); break;
          case PUTI:   sB = safe(vB); this.store(safe(this.load(sB]), vA); break;
          case INCM:   sB = safe(vB); val = this.load(sB); this.store(sB, ++val); break;
          case DECM:   sB = safe(vB); val = this.load(sB); this.store(sB, --val); break;
          case AT:     vB = this.load(safe(vB)); break;
          case COPY:   this.store(safe(vB+vA), this.load(safe(vB))); break;
          case INC:    ++vB; break;
          case DEC:    --vB; break;
          case SWAP:   vB = vB^vA; vA = vA^vB; vB = vB^vA; break;
          case TOB:    vB = vA; break;
          case TOR:    vR = vA; break;
          case TOT:    vT = vA; break;
          case FROMB:  vA = vB; break;
          case FROMR:  vA = vR; break;
          case FROMT:  vA = vT; break;
          case JMPA:   if (vA == 0) vZ = instr&MEM_MASK; break;
          case JMPB:   if (vB == 0) vZ = instr&MEM_MASK; break;
          case JMPE:   if (vA == vB) vZ = instr&MEM_MASK; break;
          case JMPN:   if (MSb == (vA&MSb)) vZ = instr&MEM_MASK; break;
          case JMPS:   if (vB == (vA&vB)) vZ = instr&MEM_MASK; break;
          case JMPU:   if (vB == (vA|vB)) vZ = instr&MEM_MASK; break;
          case JUMP:   vZ = instr&MEM_MASK; break;
          case RPT:    if ( vR != 0) { --vR; vZ = instr&MEM_MASK; } break;
          case BR:     vL = vZ; vZ = instr&MEM_MASK; break;
          case LINK:   vZ = vL; break;
          case MEM:    vA = MEM_WORDS; break;
          case IN:     vA = fnStdin(); break; // FIXME
          case OUT:    this.fnStdout(vA); break; // FIXME
          case HALT:   vA = enbyte(vA); return vA; break;
          default: return ILLEGAL; break;
        }
      }
    }

    store(addr,val) {
      this.mem.setInt32(addr, val, true);
    }

    load(addr) {
      try {
        return this.mem.getUint32(addr, true);
      } catch (e) {
        return 0; // outside bounds of DataView
      }
      return 0; // outside bounds of mem
    }

    traceVM() {} // TODO
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





