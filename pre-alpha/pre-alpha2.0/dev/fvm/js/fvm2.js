/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvm2.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170303
 * Updated:    20170619-1927+
 * Version:    pre-alpha-0.0.0.40+ for FVM 2.0
 *
 *                   This Edition of the Virtual Machine:
 *                                JavaScript
 *                           for HTML 5 browsers
 * 
 *                                ( ) [ ] { }
 *
 *    Note: This implementation is now experimenting with Plans B and C;
 *          thus it is a hybrid and internally inconsistent for now.
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

  const PRGb = 0x00000000;
  const PRGt = 0x000fffff; // modified for 20-bit PRG trial
  const MEMb = 0x00100000;
  const MEMt = 0x3fffffff;
  const BLKb = 0x40000000;
  const BLKt = 0x7fffffff;
  const VOLb = 0x80000000;
  const VOLt = 0xfeffffff;
  const SYSb = 0xff000000;
  const SYSt = 0xffffffff;
  const WORD_BYTES = 4;
  const WORD_PWR = 2; // FIXME necessary any more?
  const STACK_ELEMS = 256; //256; FIXME
  const STACK_BYTES = STACK_ELEMS << WORD_PWR;
  const STACK_1 = STACK_BYTES - WORD_BYTES;
  const INT_MAX =  2147483647;
  const INT_MIN = -2147483648;
  const MSP = 0xffffff00;
  const LSB = 0x000000ff;
  const SUCCESS = 0|0;
  const FAILURE = 1|0;
  const STDLOG = -2;
  const STDOUT = -4;
  const GRDOUT = -6;
  const USROUT = -8;
  const MNEMS = [ // Note: temporarily using FVM 1.x opcodes
    'fal ','lit ','cal ','jmp ','    ','    ','    ','bnz ', // 0
    '    ','    ','    ','    ','    ','    ','    ','    ', // 8
    '    ','    ','    ','jnz ','    ','    ','    ','    ', // 16
    'jeq ','    ','    ','    ','    ','    ','    ','    ', // 24
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
    '    ','    ','    ','    ','frt ','    ','    ','    ', // 128
    '    ','    ','    ','    ','    ','    ','    ','    ', // 136
    '    ','ret ','    ','    ','    ','    ','    ','    ', // 144
    '    ','    ','    ','    ','    ','    ','    ','drp ', // 152
    '    ','    ','    ','    ','    ','    ','    ','    ', // 160
    '    ','    ','    ','    ','    ','    ','    ','    ', // 168
    '    ','    ','    ','    ','    ','    ','    ','    ', // 176
    '    ','    ','    ','    ','    ','@   ','!   ','    ', // 184
    '    ','    ','    ','    ','    ','    ','    ','    ', // 192
    '    ','+   ','-   ','    ','    ','    ','    ','    ', // 200
    'xA  ','xB  ','xC  ','    ','    ','    ','    ','    ', // 208 // Hybrid Plan C instrs
    '    ','    ','    ','    ','    ','    ','    ','    ', // 216
    '    ','    ','    ','    ','    ','    ','    ','    ', // 224
    '    ','    ','    ','    ','    ','    ','    ','    ', // 232
    '    ','    ','    ','    ','    ','    ','    ','    ', // 240
    '    ','    ','    ','    ','    ','nop ','    ','hal ', // 248
  ];

  const iFAIL = 0|0;
  const iLIT = 1|0;
  const iCALL = 2|0;
  const iJMP = 3|0;
  const iBRNZ = 7|0;
  const iJNZ = 19|0;
  const iJEQ = 24|0;
  const iEXIT = 145|0;
  const iDROP = 159|0;
  const iADD = 201|0;
  const iLOAD = 189|0;
  const iSTORE = 190|0;
  const iSUB = 202|0;
  const iFRET = 132|0;
  const iNOOP = 253|0;
  const iHALT = 255|0;

  // Plan C
  const xA = 208|0;
  const xB = 209|0;
  const xC = 210|0;

  class FVM {
    constructor(config) {
      this.pc = PRGb + 1;
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
      this.PRGe = config.program.byteLength;
      this.prg = new DataView(config.program);
      this.RAMa = config.RAMa;
      this.RAMz = config.RAMz;
      this.ram = new DataView(new ArrayBuffer(this.RAMz-this.RAMa));
      this.ds = new Stack(this);
      this.ss = new Stack(this);
      this.rs = new Stack(this);

      //Plan C
      this.xA = 0;
      this.xB = 0;
      this.xC = 0;
    };

    run() {
      this.fnTrc('FVM run...');
      this.running = true;
      var metadata, instr, failAddr, opcode, lit;

      while (this.running) {
        instr = this.wordAtPc();
        metadata = (instr & MSP) >> 8;
        failAddr = metadata; // TODO refactor naming
        lit = metadata; // TODO refactor naming
        opcode = instr & LSB;
        if (this.tracing) {
          // lit = this.prgWord(this.pc+1);
          this.trace(this.pc,failAddr,opcode,lit); 
        }
        this.pc++;
        if (this.pc > PRGt) { // FIXME disallow split
          this.pc = this.pc & PRGt;
        }
        // lit = this.wordAtPc(); // For fixed-width
        try {
          switch (opcode) {
            case iFAIL:
              this.fail();
              break;
            // Plan C
            case xA:
              this.xA = metadata;
              break;
            case xB:
              this.xB = metadata;
              break;
            case xC:
              this.xC = metadata;
              break;
            // -------------------------
            case iLIT:
              try {
                this.ds.doPush(metadata);
              } catch (e) {
                if (e == FAILURE) {
                  break;
                } else {
                  throw e;
                }
              }
              this.pc++;
              break;
            case iCALL:
              try {
                this.rs.doPush(this.pc+1);
                this.pc = metadata;
              } catch (e) {
                if (e == FAILURE) {
                  break;
                } else {
                  throw e;
                }
              }
              break;
            case iJMP:
              this.pc = metadata;
              break;
            case iBRNZ:
              var n1 = this.ds.doPeek();
              if (n1 != 0) {  
                this.pc = metadata;
              }
              break;
            case iJNZ:
              var n1 = this.ds.doPop();
              if (n1 != 0) {  
                this.pc = metadata;
              }
              break;
            case iJEQ:
              var n2 = this.ds.doPop();
              var n1 = this.ds.doPop();
              if (n1 == n2) {  
                this.pc = metadata;
              }
              break;
            case iFRET:
              var rsTos = this.rs.doPop() & PRGt;
              if (rsTos < 2) {
                throw FAILURE;
              }
              var callPc = rsTos - 2;
              var callInstr = this.prgWord(callPc);
              var callOpcode = callInstr & LSB;
              if (callOpcode != iCALL) {
                throw FAILURE;
              }
              this.pc = callPc;
              failAddr = callPc + 1; // (callInstr & MSP) >> 8;
              throw FAILURE;
              break;
            case iEXIT:
              // FIXME handle failure
              this.pc = this.rs.doPop() & PRGt;
              break;
            case iLOAD:
              // FIXME incomplete implementation
              var addr = this.ds.doPop();
              switch (addr) {
                default:
                  if ((addr >= this.RAMa) && (addr <= this.RAMz)) {
                    var val = this.ram.getInt32(addr-this.RAMa, true);
                    this.ds.doPush(val);
                  } else {
                    throw FAILURE;
                  }
                break;
              }
              break;
            case iSTORE:
              // FIXME incomplete implementation
              var addr = this.ds.doPop();
              var val = this.ds.doPop();
              switch (addr) {
                case STDLOG:
                  this.lsb(this.fnLog,val);
                  break;
                case STDOUT:
                  this.lsb(this.fnStdout,val);
                  break;
                case GRDOUT:
                  this.lsb(this.fnGrdout,val);
                  break;
                case USROUT:
                  this.lsb(this.fnUsrout,val);
                  break;
                default:
                  if ((addr >= this.RAMa) && (addr <= this.RAMz)) {
                    this.ram.setInt32(addr-this.RAMa, val, true);
                  } else {
                    throw FAILURE;
                  }
                break;
              }
              break;
            case iDROP:
              this.ds.doPop();
              break;
            case iADD:
              if (lit == 0) {
                this.ds.apply2((a,b) => a+b);
              } else {
                this.ds.apply1((a) => a+lit);
              }
              break;
            case iSUB:
              this.ds.apply2((a,b) => a-b);
              break;
            case iNOOP:
              break;
            case iHALT:
              this.succeed();
              break;
            default:
              this.fnTrc('Illegal opcode: 0x' + modFmt.hex2(opcode));
              throw FAILURE;
              break;
          }
        } catch(e) {
          if (e != FAILURE) {
            throw e;
          }    
          if (opcode != iFRET) {
            this.fnTrc('FAILURE **********************');
          } else {
            //this.fnTrc(modFmt.hex6(this.pc) + ' ' + modFmt.hex6(failAddr) + ' ****************');
          }
          this.pc = failAddr;
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
      if (addr >= 0 && addr < this.PRGe) {
        try {
          return this.prg.getUint32(addr<<WORD_PWR, true);
        } catch (e) {
          return 0; // outside bounds of DataView
        }
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
                 MNEMS[opcode] + ' ( ' +
                 this.ds + ')[ ' +
                 this.ss + ']{ ' +
                 this.rs + '}' +
                 // Plan C
                 ' xA: ' + modFmt.hex6(this.xA) +
                 ' xB: ' + modFmt.hex6(this.xB) +
                 ' xC: ' + modFmt.hex6(this.xC)
                 );
    }
  }

  class Config {
    constructor(prg) {
      var F_ADDR = 0x00001e00;
      var S_ADDR = 0x00001e00;
      var C_ADDR = 0x00002000;
      this.program = prg;
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
        throw FAILURE; // overflow
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
        throw FAILURE; // underflow
      }
    }

    doPeek() {
      if (this.sp <= STACK_1) {
        var elem = this.elems.getInt32(this.sp, true);
        return elem;
      } else {
        throw FAILURE; // underflow
      }
    }

    apply1(f) {
      var a = this.doPop();
      this.doPush(this.verify(f(a), a));
    }

    apply2(f) {
      var b = this.doPop();
      var a = this.doPop();
      this.doPush(this.verify(f(a,b), a, b));
    }

    verify(i, a , b) {
      if (i < INT_MIN || i > INT_MAX) {
        if (a) {
          this.doPush(a);
        }
        if (b) {
          this.doPush(b);
        }
        throw FAILURE;
      }
      return i;
    }  

    toString() {
      var str = '';
      for (var i = STACK_1; i >= this.sp; i-=WORD_BYTES) {
        str = str + modFmt.hex8(this.elems.getUint32(i, true)) + ' ';        
      }
      return str;
    }
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





