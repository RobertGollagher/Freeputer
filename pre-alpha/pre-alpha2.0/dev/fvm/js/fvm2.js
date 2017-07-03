/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvm2.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170303
 * Updated:    20170701:2336+
 * Version:    pre-alpha-0.0.0.46+ for FVM 2.0
 *
 *                   This Edition of the Virtual Machine:
 *                                JavaScript
 *                           for HTML 5 browsers
 * 
 *                                ( ) [ ] { }
 *
 *               Note: This implementation is only for Plan C, JADE.
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

  const MM_SIZE = 0xefff; // in bytes .        .       .       .       .
  const WORD_SIZE_BYTES = 4;
  const SUCCESS = 0;
  const FAILURE = 1;

  const iFAL = 0x00|0; // FIXME
  const iJMP = 0x01|0;
  const iADDI = 0x10|0;
  const iHAL = 0x1f|0; // FIXME
  const MNEMS = [
    'fal ','jmp ','','','','','','',
    '','','','','','','','',
    'addi','','','','','','','',
    '','','','','','','','hal ',
  ]

  const COND = [
    '=0','=1','=2','=3',
    '=4','=5','=6','=7',
    '=8','=9','=a','=b',
    '=c','=d','=e','all'
  ]

  // Plan C instruction format JADE
  const OPCODE_MASK = 0xff000000; //   11111111000000000000000000000000
  const DST_MASK =    0x00ff0000; //   00000000111111110000000000000000
  const DSTM_MASK =   0x00c00000; //   00000000110000000000000000000000
  const SRC_MASK =    0x0000ffff; //   00000000000000001111111111111111
  const SRCM_MASK =   0x0000c000; //   00000000000000001100000000000000

  class FVM {
    constructor(config) {
      this.pc = WORD_SIZE_BYTES; // non-zero, also nowadays is byte-addressed
      this.mm = new DataView(new ArrayBuffer(MM_SIZE));
      this.fnTrc = config.fnTrc;
      this.loadProg(config.program, this.mm);
      this.tracing = true;
      this.regs = [
        0|0, 0|0, 0|0, 0|0, 0|0, 0|0, 0|0, 0|0, 
        0|0, 0|0, 0|0, 0|0, 0|0, 0|0, 0|0, 0|0
      ]
    };

    loadProg(pgm, mm) {
      if (pgm === undefined) {
        this.loadedProg = false;
        this.fnTrc('No program to load');
      } else {
        var program = new DataView(pgm);
        for (var i = 0; i < program.byteLength; i++) {
          var w = program.getUint8(i, true);
          mm.setUint8(i, w, true);
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

        this.pc+=WORD_SIZE_BYTES; // nowadays is byte-addressed
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
                throw 'UNIMPLEMENTED';
                break;
              case 0x2:
                throw 'UNIMPLEMENTED';
                break;
              case 0x3:
                throw 'UNIMPLEMENTED';
                break;
              default:
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

        if (this.pc > this.mmSize) { //FIXME
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
      this.mm.setInt32(addr, val, true);
    }

    load(addr) {
      try {
        return this.mm.getUint32(addr, true);
      } catch (e) {
        return 0; // outside bounds of DataView
      }
      return 0; // outside bounds of mm
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

    trace(pc,instr,opcode,dst,dstm,src,srcm) {
      this.fnTrc(modFmt.hex6(this.pc) + ' ' + 
                 modFmt.hex8(instr) + ' ' +
                 MNEMS[opcode] + ' ' +
                 //modFmt.hex2(opcode) + '--' +
                 modFmt.hex2(dst) + ':' +
                 modFmt.hex1(dstm) + ' | ' +
                 modFmt.hex4(src) + ':' +
                 modFmt.hex1(srcm) + 
                ' r1 ' + modFmt.hex8(this.regs[0x1]) +
                ' r2 ' + modFmt.hex8(this.regs[0x2])
                 );
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

  var hex1 = function(i) {
    if (typeof i === 'number') {
      return ('0' + i.toString(16)).substr(-1);
    } else {
      return '      ';
    }
  };

  var hex2 = function(i) {
    if (typeof i === 'number') {
      return ('00' + i.toString(16)).substr(-2);
    } else {
      return '      ';
    }
  };

  var hex3 = function(i) {
    if (typeof i === 'number') {
      return ('000' + i.toString(16)).substr(-3);
    } else {
      return '      ';
    }
  };

  var hex4 = function(i) {
    if (typeof i === 'number') {
      return ('0000' + i.toString(16)).substr(-4);
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
      return '      ';
    }
  };

  return {
    hex1: hex1,
    hex2: hex2,
    hex3: hex3,
    hex4: hex4,
    hex6: hex6,
    hex8: hex8,
  };

})(); // modFmt





