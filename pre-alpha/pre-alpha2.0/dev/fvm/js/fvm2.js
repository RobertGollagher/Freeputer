/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvm2.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170303
 * Updated:    20170402-1231
 * Version:    pre-alpha-0.0.0.7 for FVM 2.0
 *
 *                               This Edition:
 *                                JavaScript 
 *                           for HTML 5 browsers
 * 
 *                                ( ) [ ] { }
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

  const PRGb = 0;
  const PRGt = 0xffffff;
  const MEMb = 0x1000000;
  const MEMt = 0x3fffffff;
  const BLKb = 0x40000000;
  const BLKt = 0x7fffffff;
  const VOLb = 0x80000000;
  const VOLt = 0xfeffffff;
  const SYSb = 0xff000000;
  const SYSt = 0xffffffff;
  const WORD_BYTES = 4;
  const WORD_PWR = 2;
  const STACK_ELEMS = 256;
  const STACK_BYTES = STACK_ELEMS << WORD_PWR;
  const STACK_1 = STACK_BYTES - WORD_BYTES;
  const SUCCESS = 0;
  const FAILURE = 1;
  const SIMPLE = 64;
  const MNEMS = [
    'wall','','call','','','','','', // 0 COMPLEX
    '','','','','','','','', // 8
    '','','','','','','','', // 16
    '','','','','','','','', // 24
    '','','','','','','','', // 32
    '','','','','','','','', // 40
    '','','','','','','','', // 48
    '','','','','','','','', // 56
    '','','','','','','','', // 64 SIMPLE
    '','','','','','','','', // 72
    '','','','','','','','', // 80
    '','','','','','','','', // 88
    '','','','','','','','', // 96
    '','','','','','','','', // 104
    '','','','','','','','', // 112
    '','','','','','','','', // 120
    '','','','','','','','', // 128
    '','','','','','','','', // 136
    '','ret ','','','','','','', // 144
    '','','','','','','','', // 152
    '','','','','','','','', // 160
    '','','','','','','','', // 168
    '','','','','','','','', // 176
    '','','','','','','','', // 184
    '','','','','','','','', // 192
    '','','','','','','','', // 200
    '','','','','','','','', // 208
    '','','','','','','','', // 216
    '','','','','','','','', // 224
    '','','','','','','','', // 232
    '','','','','','','','', // 240
    '','','','','','','','halt', // 248
  ];

  const iWALL = 0|0;
  const iLIT = 1|0;
  const iCALL = 2|0;
  const iEXIT = 145|0;
  const iHALT = 255|0;

  class FVM {
    constructor(config) {
      this.pc = PRGb;
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
      this.program = config.program;
      this.PRGe = this.program.length;
      this.prg = new DataView(new ArrayBuffer(this.PRGe<<WORD_PWR));
      for (var i=0; i < this.PRGe; i++) {
        this.prg.setInt32(i<<WORD_PWR, this.program[i], true);
      }
      this.ds = new Stack(this);
      this.ss = new Stack(this);
      this.rs = new Stack(this);
    };

    run() {
      this.fnTrc('FVM run...');
      this.running = true;
      var instr, failAddr, opcode, lit;
      while (this.running) {
        instr = this.wordAtPc();
        failAddr = instr & 0xffffff00;
        opcode = instr & 0x000000ff;
        if (this.tracing) {
          if (opcode < SIMPLE) {
            lit = this.prgWord(this.pc+1);
          } else {
            lit = '';
          }
          this.trace(this.pc,failAddr,opcode,lit); 
        }
        this.pc++;
        switch (opcode) {
          case iWALL:
            this.fail();
            break;
          case iCALL:
            this.rs.doPush(this.pc+1);
            this.pc = this.wordAtPc();
            break;
          case iEXIT:
            this.pc = this.rs.doPop();
            break;
          case iHALT:
            this.succeed();
            break;
          default:
            this.fnTrc('Illegal instruction: 0x' + modFmt.hex2(opcode));
            this.fail();
            break;
        }
      }
      this.fnTrc('FVM ran. Exit code: ' + this.exitCode);
    };

    wordAtPc() {
      if (this.pc >= 0 && this.pc < this.PRGe) {
        return this.prg.getUint32(this.pc<<WORD_PWR, true);
      }
      return 0;
    }

    prgWord(addr) {
      if (addr >= 0 && addr < this.PRGe) {
        return this.prg.getUint32(addr<<WORD_PWR, true);
      }
      return 0;
    }

    fail() {
      if (this.rs.used() == 0) {
        this.exitCode = FAILURE;
        this.running = false;
        this.fnTrc('VM failure');
        return;
      } else {
        this.fnTrc('Subroutine failure');
        this.pc = this.rs.doPop();
        // FIXME NEXT: (1) add return metadata; (2) add branch on failed call
        return;
      }
    }

    succeed() {
      this.exitCode = SUCCESS;
      this.running = false;
    }

    trace(pc,failAddr,opcode,lit) {
      this.fnTrc(modFmt.hex8(this.pc) + ' ' + 
                 modFmt.hex6(failAddr>>8) + ':' +
                 modFmt.hex2(opcode) + ' ' +
                 MNEMS[opcode] + ' ' +
                 modFmt.hex8(lit) + ' ( ' +
                 this.ds + ')[ ' +
                 this.ss + ']{ ' +
                 this.rs + '}')
    }
  }

  class Config {
    constructor() {
      // FIMXE program hardcoded for intial development
      this.program = [
        iCALL, // 00
        0x00000008, // 01
        iCALL, // 02
        0x00000009, // 03
        iHALT, // 04
        iHALT, // 05
        iHALT, // 06
        iHALT, // 07
        iEXIT, // 08
        iEXIT, // 09
        iHALT, // 0a
        iHALT, // 0b
        iHALT, // 0c
        iHALT, // 0d
        iHALT, // 0e
        iHALT // 0f
      ]
      this.RAMa = -1;
      this.RAMz = -1;
      this.BLKz = -1;
      this.VOLz = -1;
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

  class Stack {
    constructor(fvm) {
      this.fvm = fvm;
      this.elems = new DataView(new ArrayBuffer(STACK_BYTES));
      this.sp = STACK_BYTES;
    }

    used() {
      return (STACK_BYTES-this.sp)>>WORD_PWR;
    }

    avail() {
      return STACK_ELEMS-this.used();
    }

    doPush(i) {
      if (this.sp >= WORD_BYTES) {
        this.sp = (this.sp-WORD_BYTES);
      } else {
        this.fvm.fail(); // overflow
        return;
      }
      this.elems.setInt32(this.sp, i, true);
    }

    doPop() {
      if (this.sp <= STACK_1) {
        var elem = this.elems.getInt32(this.sp, true);
        this.sp += WORD_BYTES;
        return elem;
      } else {
        this.fvm.fail(); // underflow
      }
    }

    doPeek() {
      if (this.sp <= this.SP_ONE) {
        return elem = this.elems.getInt32(this.sp, true);
      } else {
        this.fvm.fail(); // underflow
      }
    }

    toString() {
      var str = '';
      for (var i = STACK_1; i >= this.sp; i-=WORD_BYTES) {
        str = str + modFmt.hex8(this.elems.getInt32(i, true)) + ' ';        
      }
      return str;
    }
  }

  return {
    makeFVM: function(config) {
      return new FVM(config);
    },
    makeConfig: function() {
      return new Config();
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
    hex6: hex6,
    hex8: hex8
  };

})(); // modFmt





