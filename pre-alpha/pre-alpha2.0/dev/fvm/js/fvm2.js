/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvm2.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170303
 * Updated:    20170622-0719+
 * Version:    pre-alpha-0.0.0.42+ for FVM 2.0
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

// Module modFVM will provide an FVM implementation.
var modFVM = (function () { 'use strict';

  const WORD_BYTES = 4;
  const WORD_PWR = 2;
  const INT_MAX =  2147483647;
  const INT_MIN = -2147483648;
  const MSP = 0xffffff00;
  const LSB = 0x000000ff;
  const SUCCESS = 0|0;
  const FAILURE = 1|0;
  const MNEMS = [ // Note: temporarily using FVM 1.x opcodes
    'fal ','    ','    ','jmp ','    ','    ','    ','    ', // 0
    'lta ','lto ','lts ','ltd ','    ','    ','    ','    ', // 8
    '    ','    ','    ','    ','    ','    ','    ','    ', // 16
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
  const iLTA = 0x08|0;
  const iLTO = 0x09|0;
  const iLTS = 0x0a|0;
  const iLTD = 0x0b|0;
  const iADD = 0x12|0;
  const iNOP = 0xfe|0;
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
      // FIXME for now there is no address space but the program itself!
      this.prg = new DataView(config.program);

      // Reegisters. For now just treat as ordinary Numbers:
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
        if (this.pc > this.mmSize) {
          this.pc = 0;
        }
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
          case iJMP:
            this.pc = metadata;
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
      this.fnTrc(modFmt.hex5(this.pc) + ' ' + 
                 modFmt.hex6(failAddr) + ':' +
                 modFmt.hex2(opcode) + ' ' +
                 MNEMS[opcode] + ' A:' +
                 modFmt.hex8(this.rA) + ' O:' +
                 modFmt.hex8(this.rO) + ' S:' +
                 modFmt.hex8(this.rS) + ' D:' +
                 modFmt.hex8(this.rD)
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
    hex5: hex5,
    hex6: hex6,
    hex8: hex8
  };

})(); // modFmt





