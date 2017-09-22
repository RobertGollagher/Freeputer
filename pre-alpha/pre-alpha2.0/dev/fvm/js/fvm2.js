/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvm2.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170303
 * Updated:    20170913+
 * Version:    pre-alpha-0.0.1.4+ for FVM 2.0
 *
 *                               This Edition:
 *                                JavaScript
 *                           for HTML 5 browsers
 * 
 *                                ( ) [ ] { }
 *
 * THIS IMPLEMENTATION IS EXPERIMENTALLY BEING CUT DOWN TO TRUE MISC:
 *   - Removed all jumps except JMPE, JUMP (5 less)
 *   - Removed NOP (1 less)
 *   - Removed FLIP (1 less)
 *   - Removed INC, DEC (2 less)
 *   - Removed INCM, DECM (2 less)
 *   - Removed COPY (1 less)
 *   - Removed GETI, PUTI (2 less)
 *   - Removed BR, LINK (2 less and eliminated vL)
 *   - Removed TOT, FROMT (2 less and eliminated vT)
 *   - 
 *   - 
 *   - 
 *   - 
 * TODO: Later consider increasing space
 *
 * TODO:
 * - Adopt standard sizes: XS, S, M, L, XL (1 kB, 16 kB, 256 kB, 4 MB, 64 MB)
 * - Trap on read/write/jump out of bounds rather than masking
 * - Bring 'fvm2.js', 'qmisc.c' and 'qmisc.s' into line
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

  const ADD   = 0x01000000|0 // Simple
  const SUB   = 0x02000000|0
  const OR    = 0x03000000|0
  const AND   = 0x04000000|0
  const XOR   = 0x05000000|0
  const SHL   = 0x06000000|0
  const SHR   = 0x07000000|0
  const GET   = 0x08000000|0
  const PUT   = 0x09000000|0
  const AT    = 0x0e000000|0
  const SWAP  = 0x13000000|0
  const TOB   = 0x14000000|0
  const TOR   = 0x15000000|0
  const FROMB = 0x17000000|0
  const FROMR = 0x18000000|0
  const MEM   = 0x1a000000|0
  const HALT  = 0x1c000000|0
  const JMPE  = 0x1f000000|0 // Complex
  const JUMP  = 0x23000000|0
  const RPT   = 0x24000000|0
  const IN    = 0x26000000|0
  const OUT   = 0x27000000|0

  class FVM {
    constructor(config) {
      this.fnTrc = config.fnTrc;
      this.fnStdout = config.fnStdout;
      this.tracing = true; // comment this line out unless debugging
      this.vA = 0|0; // accumulator
      this.vB = 0|0; // operand register
      //this.vT = 0|0; // temporary register
      this.vR = 0|0; // repeat register
      //this.vL = 0|0; // link register (not accessible)
      this.vZ = 0|0; // program counter (not accesible) (maybe it should be)
      this.mem = new DataView(new ArrayBuffer(MEM_WORDS)); // Von Neumann
      this.loadProgram(config.program, this.mem);
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

    run() {
      this.initVM();
      while(true) {
        var instr = this.load(this.vZ);
        if (this.tracing) {
          this.traceVM(instr);
        }

        this.vZ = this.safe(++this.vZ);
        // Handle immediates
        if (instr&MSb) {
          this.vB = this.enrange(instr);
          continue;
        }

        // Handle all other instructions
        var opcode = instr & OPCODE_MASK;
        switch(opcode) { // TODO Fix order
          case ADD:    this.vA+=this.vB; break; /*KEEP*/
          case SUB:    this.vA-=this.vB; break; /*KEEP*/
          case OR:     this.vA|=this.vB; break; /*KEEP*/
          case AND:    this.vA&=this.vB; break; /*KEEP*/
          case XOR:    this.vA^=this.vB; break; /*KEEP*/
          case SHL:    this.vA<<=enshift(this.vB); break;
          case SHR:    this.vA>>=enshift(this.vB); break;
          case GET:    this.vA = this.load(this.safe(this.vB)); break; /*KEEP*/
          case PUT:    this.store(this.safe(this.vB), this.vA); break; /*KEEP*/
          case AT:     this.vB = this.load(this.safe(this.vB)); break; /*CONVENIENT*/

          case SWAP:   this.vB = this.vB^this.vA; this.vA = this.vA^this.vB; this.vB = this.vB^this.vA; break;
          case TOB:    this.vB = this.vA; break;
          case TOR:    this.vR = this.vA; break;
          case FROMB:  this.vA = this.vB; break;
          case FROMR:  this.vA = this.vR; break;





          case RPT:    if ( this.vR != 0) { --this.vR; this.vZ = instr&MEM_MASK; } break;
          case MEM:    this.vA = MEM_WORDS; break;


          case JMPE:   if (this.vA == this.vB) this.vZ = instr&MEM_MASK; break; /*KEEP*/
          case JUMP:   this.vZ = instr&MEM_MASK; break; /*KEEP*/



          case IN:     this.vA = fnStdin(); break; // FIXME /*KEEP*/
          case OUT:    fnStdout(this.vA); break; // FIXME /*KEEP*/
          case HALT:   this.vA = this.enbyte(this.vA); return this.vA; break; /*KEEP*/


          default: return ILLEGAL; break;
        }
      }
    }

    store(addr,val) {
      this.mem.setInt32(addr*WD_BYTES, val, true);
    }

    load(addr) {
      return this.mem.getUint32(addr*WD_BYTES, true);
    }

    traceVM(instr, vR) {
      var traceStr =
        modFmt.hex8(this.vZ) + " " +
        modFmt.hex8(instr) + " " +
        "vA:" + modFmt.hex8(this.vA) + " " +
        "vB:" + modFmt.hex8(this.vB) + " " +
        //"vT:" + modFmt.hex8(this.vT) + " " +
        "vR:" + modFmt.hex8(this.vR); // + " " +
        //"vL:" + modFmt.hex8(this.vL);
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





