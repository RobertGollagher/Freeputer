/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvm2.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170303
 * Updated:    20170401-21:16
 * Version:    pre-alpha-0.0.0.6 for FVM 2.0
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
  const WSHL = 2;
  const SUCCESS = 0;
  const FAILURE = 1;

  const iWALL = 0|0;
  const iHALT = 255|0;

  class FVM {
    constructor(config) {
      this.pc = PRGb;
      this.running = false;
      this.status = null;
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
      this.prg = new DataView(new ArrayBuffer(this.PRGe<<WSHL));
      for (var i=0; i < this.PRGe; i++) {
        this.prg.setInt32(i<<WSHL, this.program[i], true);
      }
    };

    run() {
      this.fnTrc('FVM run...');
      this.running = true;
      var instr, failAddr, opcode;
      while (this.running) {
        instr = this.prgWord(this.pc);
        failAddr = instr & 0xffffff00;
        opcode = instr & 0x000000ff;
        if (this.tracing) {
          this.trace(this.pc,failAddr,opcode); 
        }
        this.pc++;
        switch (opcode) {
          case iWALL:
            this.status = FAILURE;
            this.running = false;
            break;
          case iHALT:
            this.status = SUCCESS;
            this.running = false;
            break;
          default:
            this.status = FAILURE;
            this.running = false;
            break;
        }
      }
      this.fnTrc('FVM ran. Exit code: ' + this.status);
    };

    prgWord(addr) {
      if (addr >= 0 && addr < this.PRGe) {
        return this.prg.getUint32(addr<<WSHL, true);
      }
      return 0;
    }

    trace(pc,failAddr,opcode) {
      this.fnTrc(modFmt.hex8(this.pc) + " " + 
                 modFmt.hex6(failAddr>>8) + ":" +
                 modFmt.hex2(opcode));
    }
  }

  class Config {
    constructor() {
      this.program = [
        iHALT
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

  var hex8 = function(i) {
    if (typeof i === 'number') {
      return ('00000000' + i.toString(16)).substr(-8);
    } else {
      return '        ';
    }
  };

  var hex6 = function(i) {
    if (typeof i === 'number') {
      return ('000000' + i.toString(16)).substr(-6);
    } else {
      return '      ';
    }
  };

  return {
    hex2: hex2,
    hex8: hex8,
    hex6: hex6
  };

})(); // modFmt





