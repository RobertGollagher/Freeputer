/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvm2.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170303
 * Updated:    20170911+
 * Version:    pre-alpha-0.0.1.0+ for FVM 2.0
 *
 *                               This Edition:
 *                                JavaScript
 *                           for HTML 5 browsers
 * 
 *                                ( ) [ ] { }
 *
 *   Status: About to begin porting 'qmisc.c' to JavaScript.
 *   FIXME This is currently completely broken and does not work yet.
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
  const MAX_MEM_WORDS = 0x1000    // 16 kB = 4096 words = standard size
  const MEM_WORDS = MAX_MEM_WORDS //     (favours modular design) 
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

  // Plan C instruction format JADE
  const OPCODE_MASK = 0xff000000; //   11111111000000000000000000000000
  const DST_MASK =    0x003f0000; //   00000000001111110000000000000000
  const DSTM_MASK =   0x00c00000; //   00000000110000000000000000000000
  const SRC_MASK =    0x0000ffff; //   00000000000000001111111111111111
  const SRCM_MASK =   0x0000c000; //   00000000000000001100000000000000

  class FVM {
    constructor(config) {
      this.pc = WD_BYTES; // non-zero, also nowadays is byte-addressed
      this.mem = new DataView(new ArrayBuffer(MEM_WORDS));
      this.fnTrc = config.fnTrc;
      this.loadProg(config.program, this.mem);
      this.tracing = true;
      this.regs = [
        0|0, 0|0, 0|0, 0|0, 0|0, 0|0, 0|0, 0|0, 
        0|0, 0|0, 0|0, 0|0, 0|0, 0|0, 0|0, 0|0
      ]
    };

    loadProg(pgm, mem) {
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

    run() {
      if (!this.loadedProg) {
        return;
      }
      this.fnTrc('FVM run...');
      this.running = true;
      var instr, opcode, dst, dstm, src, srcm;

      while (this.running) {
        instr = this.wordAtPc();

        // TODO optimize later
        opcode = (instr & OPCODE_MASK) >> 24;
        dst = (instr & DST_MASK) >> 16;
        dstm = (instr & DSTM_MASK) >> 22;
        src = (instr & SRC_MASK);
        srcm = (instr & SRC_MASK) >> 14;

        if (this.tracing) {
          this.trace(this.pc,instr,opcode,dst,dstm,src,srcm); 
        }

        this.pc+=WD_BYTES; // nowadays is byte-addressed
        switch (opcode) {
          case iFAL:
            this.fail(); // FIXME
            break;
          case iJMP: // FIXME not properly implemented yet!
            this.pc = src;
            break;
          case iADDI: // FIXME
            switch(dstm) {
              case 0x0:
                this.regs[dst]+=src;
                break;
              case 0x1:
                this.store(this.regs[dst], this.load(this.regs[dst]) + src);
                break;
              case 0x2:
                this.store(this.regs[dst], this.load(this.regs[dst]) + src);
                this.regs[dst]+=WD_BYTES; // TODO what about overflow?
                break;
              case 0x3:
                this.regs[dst]-=WD_BYTES;
                this.store(this.regs[dst], this.load(this.regs[dst]) + src);
                break;
              default:
                throw ('VM BUG!');
                break;
            }
            break;
          case iHAL:
            this.succeed(); // FIXME
            break;
          default:
            this.fnTrc('Illegal opcode: 0x' + modFmt.hex2(opcode));
            this.fail();
            break;
        }

        if (this.pc > this.memSize) { //FIXME
          this.running = false;
        }

      }
      this.fnTrc('FVM ran. Exit code: ' + this.exitCode);
    };

    cellAtPc() {
      var addr = this.load(this.pc);
      if (addr > PRGt) {
        throw FAILURE;
      }
      return addr;
    }

    wordAtPc() {
      return this.load(this.pc);
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

    fail() {
      this.exitCode = FAILURE;
      this.running = false;
      this.fnTrc('VM failure');
    }

    succeed() {
      this.exitCode = SUCCESS;
      this.running = false;
      this.fnTrc('VM success');
    }

    trace(pc) {
      this.fnTrc(modFmt.hex8(this.pc));
    }
  }

  class Config {
    constructor(prg) {
      this.program = prg;
      this.fnStdin = null;
      this.fnStdout = null;
      this.fnTrc = null;
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





