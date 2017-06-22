/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvm2.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170303
 * Updated:    20170622-0651+
 * Version:    pre-alpha-0.0.0.44+ for FVM 2.0
 *
 *                   This Edition of the Virtual Machine:
 *                                JavaScript
 *                           for HTML 5 browsers
 * 
 *                                ( ) [ ] { }
 *
 *              Note: This implementation is only for Plan C.
 *          It is extremely minimal without any stack support.
 *
 * ===========================================================================
 * 
 * WARNING: This is pre-alpha software and as such may well be incomplete,
 * unstable and unreliable. It is considered to be suitable only for
 * experimentation and nothing more.
 * 
 * ===========================================================================
 *
 */

/*

The Bottom Line:

1. VM needs to be robust and correct: SOLVED
2. Assembler needs to be easy to implement and need little RAM: SOLVED
3. VM needs to be easier to implement : MOST IMPORTANT THING
    - easiest approach is 1 flat address space: OK
    - might just use lit/call from addr to solve length issues: OK
    - just use dedicated I/O instructions: OK
4. VM needs great hardware freedom
5. Need more standardization
6. Make all the complexity just go away!
                ------------------------

TRIAL:

  - only unsigned
  - no premature optimisation: just factor it small
  - very few registers if any  
  - could go smaller on instr width? (using literal table)

*/

// Module modFVM will provide an FVM implementation.
var modFVM = (function () { 'use strict';

  const PRG_SIZE_BYTES = 0x4000000; // = words 0x000000...0xffffff
  const WORD_BYTES = 4; // ? would byte-indexing be better ?
  const WORD_PWR = 2;
  const INT_MAX =  2147483647;
  const INT_MIN = -2147483648;
  const MSP = 0xffffff00;
  const LSB = 0x000000ff;
  const SUCCESS = 0|0;
  const FAILURE = 1|0;
  const MNEMS = [ // Note: temporarily using FVM 1.x opcodes
    'fal ','    ','    ','jmp ','jst ','    ','    ','    ', // 0
    'res ','lit ','src ','dst ','add ','lod ','sav ','    ', // 8
    '    ','    ','src@','dst@','    ','    ','    ','    ', // 16
    '    ','    ','    ','    ','    ','    ','    ','    ', // 24
    '    ','    ','    ','    ','    ','    ','    ','    ', // 32
    '    ','    ','    ','    ','    ','    ','    ','    ', // 40
    '    ','    ','    ','    ','    ','    ','    ','    ', // 48
    '    ','    ','    ','    ','    ','    ','    ','    ', // 56
    '    ','    ','    ','    ','    ','    ','    ','    ', // 64
    '    ','    ','    ','    ','    ','    ','    ','    ', // 72
    '    ','    ','    ','    ','    ','    ','    ','    ', // 80
    '    ','    ','    ','    ','    ','    ','    ','    ', // 88
    '    ','    ','    ','    ','    ','    ','    ','    ', // 96
    '    ','    ','    ','    ','    ','    ','    ','    ', // 104
    '    ','    ','    ','    ','    ','    ','    ','    ', // 112
    '    ','    ','    ','    ','    ','    ','    ','    ', // 120
    '    ','    ','    ','    ','    ','    ','    ','    ', // 128
    '    ','    ','    ','    ','    ','    ','    ','    ', // 136
    '    ','    ','    ','    ','    ','    ','    ','    ', // 144
    '    ','    ','    ','    ','    ','    ','    ','    ', // 152
    '    ','    ','    ','    ','    ','    ','    ','    ', // 160
    '    ','    ','    ','    ','    ','    ','    ','    ', // 168
    '    ','    ','    ','    ','    ','    ','    ','    ', // 176
    '    ','    ','    ','    ','    ','    ','    ','    ', // 184
    '    ','    ','    ','    ','    ','    ','    ','    ', // 192
    '    ','    ','    ','    ','    ','    ','    ','    ', // 200
    '    ','    ','    ','    ','    ','    ','    ','    ', // 208 // Hybrid Plan C instrs
    '    ','    ','    ','    ','    ','    ','    ','    ', // 216
    '    ','    ','    ','    ','    ','    ','    ','    ', // 224
    '    ','    ','    ','    ','    ','    ','    ','    ', // 232
    '    ','    ','    ','    ','    ','    ','    ','    ', // 240
    '    ','    ','    ','    ','    ','nop ','    ','hal ', // 248
  ];

  const iFAL = 0x00|0;
  const iJMP = 0x03|0;
  const iJST = 0x04|0;
  const iLTA = 0x08|0;
  const iLTO = 0x09|0;
  const iLTS = 0x0a|0;
  const iLTD = 0x0b|0;
  const iADD = 0x0c|0;
  const iLDA = 0x0d|0;
  const iSTA = 0x0e|0;
  const iSRCat = 0x12|0;
  const iDSTat = 0x13|0;
  const iNOP = 0xfd|0;
  const iHAL = 0xff|0;

  class FVM {
    constructor(config) {
      this.pc = 1;
      this.running = false;
      this.exitCode = null;
      this.tracing = true;
      this.fnStdin = config.fnStdin;
      this.fnStdout = config.fnStdout;
      this.fnUsrin = config.fnUsrin;
      this.fnUsrout = config.fnUsrout;
      this.fnGrdin = config.fnGrdin;
      this.fnGrdout = config.fnGrdout;
      this.fnLog = config.fnLog;
      this.fnTrc = config.fnTrc;

      // Plan C revised: just use 1 flat PRG space for everything
      this.prg = new DataView(new ArrayBuffer(PRG_SIZE_BYTES));
      var program = new DataView(config.program);
      for (var i = 0; i < config.program.byteLength; i+=WORD_BYTES) {
        var w = program.getInt32(i, true);
        this.prg.setInt32(i, w, true);
      }

      // Registers. For now just treat as ordinary Numbers:
      this.rA = 0; // Accumulator
      this.rO = 0; // Operand
      this.rS = 0; // Source pointer
      this.rD = 0; // Destination pointer
    };

    run() {
      this.fnTrc('FVM run...');
      this.running = true;
      var metadata, instr, failAddr, opcode, lit;

      while (this.running) {
        instr = this.wordAtPc();
        metadata = (instr & MSP) >> 8;
        failAddr = metadata;
        lit = metadata;
        opcode = instr & LSB;
        if (this.tracing) {
          this.trace(this.pc,failAddr,opcode,lit); 
        }
        this.pc++;
        /* FIXME
        if (this.pc > this.mmSize) {
          this.pc = 0;
        } */
        switch (opcode) {
          case iFAL:
            this.fail();
            break;
          case iLTA:
            this.rA = metadata;
            break;
          case iLTO:
            this.rO = metadata;
            break;
          case iLTS:
            this.rS = metadata;
            break;
          case iLTD:
            this.rD = metadata;
            break;
          case iSRCat:
            this.rS = this.prgWord(metadata);
            break;
          case iDSTat:
            this.rD = this.prgWord(metadata);
            break;
          case iADD:
            this.rA = this.rA + this.rO; // FIXME handle overflow and consider unsigned
            break;
          case iLDA: // FIXME
            this.rA = this.prg.getInt32(this.rS*WORD_BYTES, true);
            break;
          case iSTA: // FIXME
            this.prg.setInt32(this.rD*WORD_BYTES, this.rA, true);
            break;
          case iJMP:
            this.pc = metadata;
            break;
          case iJST:
            this.pc = this.dst;
            break;
          case iNOP:
            break;
          case iHAL:
            this.succeed();
            break;
          default:
            this.fnTrc('Illegal opcode: 0x' + modFmt.hex2(opcode));
            this.fail();
            break;
        }
      }
      this.fnTrc('FVM ran. Exit code: ' + this.exitCode);
    };

    lsb(f,x) {
      f(x&LSB);
    }

    cellAtPc() {
      var addr = this.prgWord(this.pc);
      if (addr > PRGt) {
        throw FAILURE;
      }
      return addr;
    }

    wordAtPc() {
      return this.prgWord(this.pc);
    }

    prgWord(addr) {
      try {
        // FIXME for now there is no address space but the program itself!
        return this.prg.getUint32(addr<<WORD_PWR, true);
      } catch (e) {
        return 0; // outside bounds of DataView
      }
      return 0; // outside bounds of PRG
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

    trace(pc,failAddr,opcode,lit) {
      this.fnTrc(modFmt.hex6(this.pc) + ' ' + 
                 modFmt.hex6(failAddr) + ':' +
                 modFmt.hex2(opcode) + ' ' +
                 MNEMS[opcode] + ' ' +
                 modFmt.hex8(this.rO) + ' ' +
                 modFmt.hex8(this.rA) + ' S:' +
                 modFmt.hex6(this.rS) + ' D:' +
                 modFmt.hex6(this.rD)
                 );
    }
  }

  class Config {
    constructor(prg) {
      this.program = prg;
      this.fnStdin = null;
      this.fnStdout = null;
      this.fnUsrin = null;
      this.fnUsrout = null;
      this.fnGrdin = null;
      this.fnGrdout = null;
      this.fnLog = null;
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

  var hex2 = function(i) {
    if (typeof i === 'number') {
        return ('00' + i.toString(16)).substr(-2);
    } else {
      return ' ';
    }
  }

  var hex4 = function(i) {
    if (typeof i === 'number') {
      return ('0000' + i.toString(16)).substr(-4);
    } else {
      return '      ';
    }
  };

  var hex5 = function(i) {
    if (typeof i === 'number') {
      return ('00000' + i.toString(16)).substr(-5);
    } else {
      return '      ';
    }
  };

  var hex6 = function(i) {
    if (typeof i === 'number') {
      return ('000000' + i.toString(16)).substr(-6);
    } else {
      return '      ';
    }
  };

  var hex8 = function(i) {
    if (typeof i === 'number') {
      return ('00000000' + i.toString(16)).substr(-8);
    } else {
      return '        ';
    }
  };

  return {
    hex2: hex2,
    hex4: hex4,
    hex5: hex5,
    hex6: hex6,
    hex8: hex8
  };

})(); // modFmt





