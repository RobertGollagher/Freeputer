/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvma.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170611
 * Updated:    20180430+
 * Version:    pre-alpha-0.0.1.60+ for FVM 2.0
 *
 *                     This Edition of the Assembler:
 *                                JavaScript
 *                           for HTML 5 browsers
 * 
 *                                ( ) [ ] { }
 *
 *
 * Trying a m{ ... {unit ...} ...} scheme for namespaces,
 * where module is the two most significant bytes of the symbol.
 *
 * Note: Need to limit ranges of m, u, s...
 * FIXME Enforce no unreasonable use of m0.x0 (reserved the sole forward).
 * FIXME Refactor and greatly simplify this whole implementation.
 * TODO  Consider constants, variables and scope for them.
 *
 * ===========================================================================
 * 
 * WARNING: This is pre-alpha software and as such may well be incomplete,
 * unstable and unreliable. It is considered to be suitable only for
 * experimentation and nothing more.
 * 
 * ===========================================================================
 */

// Module modFVMA will provide a Freeputer Assembler implementation.
var modFVMA = (function () { 'use strict';

  const WD_BYTES = 4;
  const ADDR_MASK = 0x00ffffff;

  const START = 'm0.x0';
  const START_INDEX = 0x10000;
  const DEF = '#define';
  const HERE = '.';
  const COMSTART = '/*';
  const COMEND = '*/';
  const COMLINE= '//';
  const COMWORD= '/';

  const TPOKE = 0x51000000|0
  const TPEEK = 0x52000000|0
  const CPEEK = 0x53000000|0

  const TPUSH = 0x54000000|0
  const TPOP  = 0x55000000|0
  const TDROP = 0x59000000|0
  const CPUSH = 0x56000000|0
  const CPOP  = 0x57000000|0
  const CDROP = 0x58000000|0

  const FROMB = 0x77000000|0
  const TOB   = 0x78000000|0
  const TOT   = 0x79000000|0
  const TOR   = 0x7a000000|0

  const NOP   = 0x00000000|0 // Simple

  const MUL   = 0x30000000|0
  const DIV   = 0x31000000|0
  const MOD   = 0x32000000|0

  const TRON  = 0x33000000|0
  const TROFF = 0x34000000|0

  const HOLD  = 0x35000000|0
  const GIVE  = 0x36000000|0

  const DECM  = 0x37000000|0
  const INCM  = 0x38000000|0

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
  const ROM   = 0x0c000000|0

  const COPY  = 0x0f000000|0
  const INC   = 0x10000000|0
  const DEC   = 0x11000000|0
  const FLIP  = 0x12000000|0
  const NEG   = 0x13000000|0

  const HALT  = 0x1c000000|0
  const JMPZ  = 0x1d000000|0 // Complex
  const JMPE  = 0x1f000000|0
  const JMPG  = 0x21000000|0
  const JMPL  = 0x22000000|0
  const JUMP  = 0x23000000|0
  const RPT   = 0x24000000|0
  const IN    = 0x26000000|0
  const OUT   = 0x27000000|0

  const PMI   = 0x28000000|0
  const DMW   = 0x29000000|0
  const RMW   = 0x41000000|0
  const HW    = 0x42000000|0

  const FAIL  = 0x40000000|0

  const CALL  = 0x60000000|0
  const RET   = 0x61000000|0

  const DSA   = 0x62000000|0
  const DSE   = 0x63000000|0
  const TSA   = 0x64000000|0
  const TSE   = 0x65000000|0
  const CSA   = 0x66000000|0
  const CSE   = 0x67000000|0
  const RSA   = 0x68000000|0
  const RSE   = 0x69000000|0

  const DROP  = 0x70000000|0
  const SWAP  = 0x71000000|0
  const OVER  = 0x72000000|0
  const ROT   = 0x73000000|0
  const DUP   = 0x74000000|0

  const CATCH = 0x76000000|0

  const LIT   = 0x80000000|0

  const SYMBOLS = { // Note: simple only here, complex in code below
    nop:    NOP,

    mul:    MUL,
    div:    DIV,
    mod:    MOD,

    tron:   TRON,
    troff:  TROFF,

    hold:   HOLD,
    give:   GIVE,

    decm:   DECM,
    incm:   INCM,

    add:    ADD,
    sub:    SUB,
    or:     OR,
    and:    AND,
    xor:    XOR,
    shl:    SHL,
    shr:    SHR,
    get:    GET,
    put:    PUT,
    geti:   GETI,
    puti:   PUTI,
    rom:    ROM,

    copy:   COPY,
    inc:    INC,
    dec:    DEC,
    flip:   FLIP,
    neg:    NEG,

    halt:   HALT,

    tpush:  TPUSH,
    tpop:   TPOP,
    tpoke:  TPOKE,
    tpeek:  TPEEK,
    cpush:  CPUSH,
    cpop:   CPOP,
    cpeek:  CPEEK,
    cpush:  CPUSH,
    tdrop:  TDROP,
    cdrop:  CDROP,

    fromb:  FROMB,
    tob:    TOB,
    tot:    TOT,
    tor:    TOR,

    pmi:    PMI,
    dmw:    DMW,
    rmw:    RMW,
    hw:     HW,
    fail:   FAIL,

    call:   CALL,
    ret:    RET,
    lit:    LIT,

    dsa:    DSA,
    dse:    DSE,
    tsa:    TSA,
    tse:    TSE,
    csa:    CSA,
    cse:    CSE,
    rsa:    RSA,
    rse:    RSE,

    drop:   DROP,
    swap:   SWAP,
    over:   OVER,
    rot:    ROT,
    dup:    DUP,

    catch:  CATCH

  };

  const COND = {
    all:0xf|0
  };

  class FVMA {
    constructor(fnMsg) {
      this.fnMsg = fnMsg;
      this.reset();
    };

    reset() {
      this.prgElems = new PrgElems();
      this.inBlockComment = false;
      this.inLineComment = false;
      this.dict = {};
      this.expectDecl = false;
      this.expectDef = false;
      this.expectModuleNum = false;
      this.currentModuleNum = null;
      this.Decl = "";
      // x0 symbol is reserved for /*start*/ label...
      this.x0label = null;
      // ...and it is the only forward reference supported.
      this.x0refCell = null;
    }

    asm(str) { // FIXME no enforcement yet
      this.reset();
      this.fnMsg('Parsing...');
      var lines = str.split(/\n/);
      try {
        for (var i = 0; i < lines.length; i++) {
          this.parseLine(lines[i], i+1);
        }
        if (this.x0refCell != null) {
          if (this.x0label == null) {
            throw "Missing entry point x0: referenced at cell:"
            + this.x0refCell
            + "\nDeclare the entry point for your program with a x0: label.";
          } else {
            // Populate the x0: /*start*/ label referent into its referer
            var x0Addr = this.x0label&ADDR_MASK;
            var opcode = this.prgElems.getElem(this.x0refCell);
            this.prgElems.putElem(opcode|x0Addr,this.x0refCell);
          }
        }
        // Uncomment next line to see hex dump
        //this.fnMsg(this.prgElems);
        if (this.x0label != null) {
          this.fnMsg('Label x0: ' + this.x0label);
        }
        var dictStr = "";
        this.fnMsg('Dictionary...');
        //this.fnMsg(JSON.stringify(this.dict));
        for (var key in this.dict) {
          dictStr += modFmt.hex8(key|0) + ':' + this.dict[key].toString(16) + " ";
        }
        this.fnMsg(dictStr);
        var sz = this.prgElems.size();
        this.fnMsg('Program size: ' + sz + " words = " + sz*WD_BYTES + " bytes");
        this.fnMsg('Done');
        return this.prgElems.toBuf();
      } catch (e) {
        this.fnMsg(e);
      }
    };

    parseLine(line, lineNum) {
      // Tokens are separated by any whitespace. Tokens contain no whitespace.
      var tokens = line.split(/\s+/).filter(x => x.match(/\S+/));
      tokens.forEach(token => this.parseToken(token,lineNum), this);
      this.inLineComment = false;
    }

    use(x) {
      if (this.expectDef) {
       if (x === HERE) {
         this.dict[this.decl] = this.prgElems.cursor;
       } else {
         this.dict[this.decl] = x;
       }
       this.decl = "";
       this.expectDef = false;
      } else if (this.expectCond) {
        // FIXME
      } else {
       this.prgElems.addElem(x);
      }
    }

    parseToken(token, lineNum) {
      if (false) {
      } else if (this.inCmt(token)) {
      } else if (this.parseComment(token, lineNum)) {
      //} else if (this.parseComword(token, lineNum)) {
      } else if (this.expectingDecl(token, lineNum)) {
      } else if (this.expectingCond(token, lineNum)) {
      } else if (this.expectingModuleNum(token, lineNum)) {
      } else if (this.parseModuleStart(token,lineNum)) {
      } else if (this.parseModuleEnd(token)) {
      } else if (this.disallowGlobals(token, lineNum)) {
      } else if (this.parseUnit(token)) {
      } else if (this.parseLabelDecl(token, lineNum)) {
      } else if (this.parseDef(token)) {
      } else if (this.parseRef(token)) {
      } else if (this.parseHere(token, lineNum)) {
      } else if (this.parseCall(token)) {
      } else if (this.parseJump(token)) {
      } else if (this.parseJmpZ(token)) {
      } else if (this.parseJmpE(token)) {
      } else if (this.parseJmpG(token)) {
      } else if (this.parseJmpL(token)) {
      } else if (this.parseIn(token)) {
      } else if (this.parseOut(token)) {
      } else if (this.parseRpt(token)) {
      } else if (this.parseBr(token)) {
      // } else if (this.parseDecimalLiteral(token)) { // Disallowed for now
      } else if (this.parseHexLiteral(token, lineNum)) {
      } else if (this.parseCatch(token)) {
      } else {
        throw lineNum + ":Unknown symbol:" + token;
      }
    };

    symbolToInt(str) {
      if (str.match(/[m][0-9a-f]{1,4}\.[x][0-9a-f]{1,4}$/)){
          var modNumStr = str.match(/[m][0-9a-f]{1,4}/)[0];
          var symNumStr = str.match(/[x][0-9a-f]{1,4}/)[0];
          var symAsHex = symNumStr.replace('x','0x');
          var symIntValue = parseInt(symAsHex,16);
          var modAsHex = modNumStr.replace('m','0x');
          var modIntValue = parseInt(modAsHex,16);
          modIntValue = modIntValue << 16;
          var intValue = modIntValue | symIntValue;
          return intValue;
      } else if (str.match(/[suxm][0-9a-f]{1,4}$/)){
        if (str.match(/s[0-9a-f]{1,4}$/)) { //FIXME
          var asHex = str.replace('s','0x');
          var intValue = parseInt(asHex,16);
        } else if (str.match(/u[0-9a-f]{1,4}$/)) { //FIXME
          var asHex = str.replace('u','0x');
          var intValue = parseInt(asHex,16) << 8;
        } else if (str.match(/m[0-9a-f]{1,4}$/)) { //FIXME
          var asHex = str.replace('m','0x');
          var intValue = parseInt(asHex,16);
        } else {
          var asHex = str.replace('x','0x');
          var intValue = parseInt(asHex,16);
          intValue |= (this.currentModuleNum << 16);
        }
        return intValue;
      } else {
        throw "Assembler bug in symbolToInt(str) for:" + str;
      }
    }

    intToSymbol(i) {
      if (i >= 0x0000 && i <= 0xffff){
        var str = 's' + ('0000' + i.toString(16)).substr(-4);
        return str;
      } else {
        throw "Assembler bug in intToSymbol(int) for:" + i;
      }
    }

    expectingDecl(token, lineNum) {
      if (this.expectDecl) {
        var intValue;
        if (token.length == 5 && token.match(/[sx][0-9a-f]{1,4}/)){ // FIXME
          intValue = this.symbolToInt(token);
        } else {
          throw lineNum + ":Illegal symbol format (must be like s1 or s0001):" + token;
        }
        if (this.dict[intValue]) {
          throw lineNum + ":Already defined:" + token;
        } else {
          this.decl = intValue;
          this.expectDecl = false;
          this.expectDef = true;
        }
        return true;
      }
      return false;
    }

    expectingCond(token, lineNum) {
      return false; //FIXME
    }

     parseDef(token) {
       if (token === DEF){
         this.expectDecl = true;
         return true;
       } else {
         return false;
       }      
     }

    expectingModuleNum(token, lineNum) {
      if (this.expectModuleNum) {
        var expectModuleNum;
        var intValue
//      if (token.match(/[m][0-9a-f]{1,4}/)){
        if (token.match(/^mod\(m[0-9a-f]{1,4}\)$/)){
            var symbolToken = token.substring(4,token.length-1);
            intValue = this.symbolToInt(symbolToken);
        } /*else if (token.match(/^mod\(m[0-9a-f]{1,4},m[0-9a-f]{1,4}\)$/)){
            var symbolToken = token.substring(5,token.length-1); // FIXME
            intValue = this.symbolToInt(symbolToken);
        } */else {
          throw lineNum + ":Illegal module name (must be like m1):" + token;
        }
/* FIXME
        if (this.dict[intValue]) {
          throw lineNum + ":Already defined:" + token;
        } else {
*/
          this.currentModuleNum = intValue;
          this.expectModuleNum = false;
//        }
        return true;
      }
      return false;
    }

    disallowGlobals(token, lineNum) {
      if (this.currentModuleNum === null) {
        // Global code shall not be allowed. All code must be modular.
        throw lineNum + ":Global code is not allowed. You must move this to a module: " + token;
      } else {
        return false;
      }
    }

     // FIXME Unclear if this can work with the C preprocessor
     parseModuleStart(token, lineNum) {
       if (token === 'm{'){
         if (this.currentModuleNum !== null) {
           throw lineNum + ":Cannot nest modules: " + token;
         }
         this.expectModuleNum = true;
         this.clearLocalsu();
         return true;
       } else {
         return false;
       }      
     }

     parseModuleEnd(token) {
       if (token === '}m'){
         this.currentModuleNum = null;
         this.clearLocalsu();
         return true;
       } else {
         return false;
       }      
     }

    clearLocals() {
      // Clear symbols from 0x0000 to 0xff, the local symbols range,
      // so that this compilation unit can reuse them and cannot accidentally
      // refer to their declarations in any previous compilation unit.
      for (var key in this.dict) {
        if (key < 0xff) {
          delete this.dict[key];
        }            
      }
    }

    clearLocalsu() {
      // Clear symbols from 0x0000 to 0xffff, the local symbols range,
      // so that this compilation unit can reuse them and cannot accidentally
      // refer to their declarations in any previous compilation unit.
      for (var key in this.dict) {
        if (key < 0xffff) {
          delete this.dict[key];
        }            
      }
    }

     // The C preprocessor would replace {unit with { __label__ s0, s1 ...
     parseUnit(token) { // FIXME this is inefficient
        var intValue;
        if (token == 'u{' || token == '}u'){ // FIXME weak logic
          this.clearLocals();
          return true;
        }
        return false;
     }

     parseLabelDecl(token, lineNum) { // TODO refactor this whole assembler later, add u
        var intValue;
        if (token === 'x0:') {
           this.x0label = this.prgElems.cursor;
           this.decl = ""; // FIXME redundant?
           this.expectDef = false; // FIXME redundant?
           return true;
        }
        if (token.match(/[s][0-9a-f]{1,3}:/)){ // FIXME sff limit make more robust
            intValue = this.symbolToInt(token.substring(0,token.length-1));
        } else if (token.match(/[u][0-9a-f]{1,3}:/)){ // FIXME uff limit make more robust
            intValue = this.symbolToInt(token.substring(0,token.length-1));
        } else if (token.match(/[x][0-9a-f]{1,4}:/)){
            intValue = this.symbolToInt(token.substring(0,token.length-1));
            intValue |= (this.currentModuleNum << 16);
        } else {
          return false;
        }
        if (this.dict[intValue]) {
          throw lineNum + ":Already defined:" + token;
        } else {
          this.decl = intValue;
          this.expectDecl = false;
          this.expectDef = true;
          this.use(HERE);
        }
        return true; 
     }
 
     parseRef(token,opcode) {
       if (SYMBOLS[token] >= 0){
           var n = SYMBOLS[token];
           n = n | opcode;
           this.use(n);
           return true;
       } else if (token.match(/[su][0-9a-f]{1,4}$/) && this.dict[this.symbolToInt(token)] >= 0){
           var n = this.dict[this.symbolToInt(token)];
           n = n | opcode;
           this.use(n);
           return true;
       } else if (token.match(/[x][0-9a-f]{1,4}$/) && this.dict[this.symbolToInt(token)] >= 0){
           var n = this.dict[this.symbolToInt(token)];
           n = n | opcode;
           this.use(n);
           return true;
      } else if (token.match(/[m][0-9a-f]{1,4}\.[x][0-9a-f]{1,4}$/) && this.dict[this.symbolToInt(token)] >= 0){
           var n = this.dict[this.symbolToInt(token)];
           n = n | opcode;
           this.use(n);
           return true;
       } else if (token === START) { // Special case
           this.x0refCell = this.prgElems.cursor;
           var n = opcode; // Assembler will later overwrite for /*start*/
           this.use(n);
           return true;
       } else {
           return false;
       }
     }
 
     parseHere(token, lineNum) {
       if (token === HERE){
         if (this.expectDef) {
           this.use(token);
         } else {
           throw lineNum + ":Not permitted here:" + token;
         }
         return true;
       } else {
         return false;
       }      
     }

    inCmt(token, lineNum) {
        if (this.inLineComment) {
          return true;
        } else if (this.inBlockComment) {
          if (token == COMEND){
            this.inBlockComment = false;
          }
          return true;
        } else {
          if (token == COMEND){
            throw lineNum + ":Not permitted here:" + token;
          }
          return false;
        }     
    }

    parseComment(token, lineNum) {
      if (token === COMSTART){
        this.inBlockComment = true;
        return true;
      } else if (token.startsWith(COMSTART)){
          if (token.endsWith(COMEND)) {
            return true;
          } else {
            this.inBlockComment = true;
            return true;
          }
      } else if (token === COMLINE){
        this.inLineComment = true;
        return true;
      } else {
        return false;
      }      
    }
/*
    parseComword(token, lineNum) {
      if (token.startsWith(COMWORD)) {
        return true;
      } else {
        return false;
      }
    }
*/

    refMatch(mnem,token) {
      var re1 = new RegExp(mnem + '\\([m][0-9a-f]{1,4}\\.[x][0-9a-f]{1,4}\\)');
      var re2 = new RegExp(mnem + '\\([sux][^\\s]+\\)');
      if (token.match(re1) || token.match(re2)) { 
        return true;
      } else {
        return false;
      }
    }

    parseCall(token) {
      if (this.refMatch('call', token)) {
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, CALL);
      } else {
        return false;
      }
    }

    parseJump(token) {
      if (this.refMatch('jump', token)) {
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JUMP);
      } else {
        return false;
      }
    }

    parseJmpZ(token) {
      if (this.refMatch('jmpz', token)) {
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JMPZ);
      } else {
        return false;
      }
    }

    parseJmpE(token) {
      if (this.refMatch('jmpe', token)) {
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JMPE);
      } else {
        return false;
      }
    }

    parseJmpG(token) {
      if (this.refMatch('jmpg', token)) {
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JMPG);
      } else {
        return false;
      }
    }

    parseJmpL(token) {
      if (this.refMatch('jmpl', token)) {
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JMPL);
      } else {
        return false;
      }
    }

    parseIn(token) {
      if (this.refMatch('in', token)) {
        var symbolToken = token.substring(3,token.length-1);
        return this.parseRef(symbolToken, IN);
      } else {
        return false;
      }
    }

    parseOut(token) {
      if (this.refMatch('out', token)) {
        var symbolToken = token.substring(4,token.length-1);
        return this.parseRef(symbolToken, OUT);
      } else {
        return false;
      }
    }

    parseRpt(token) { // Only allows label symbols not raw numbers here
      if (this.refMatch('rpt', token)) {
        var symbolToken = token.substring(4,token.length-1);
        return this.parseRef(symbolToken, RPT);
      } else {
        return false;
      }
    }

    parseBr(token) { // Only allows label symbols not raw numbers here
      if (this.refMatch('br', token)) {
        var symbolToken = token.substring(3,token.length-1);
        return this.parseRef(symbolToken, BR);
      } else {
        return false;
      }
    }
/*
    parseDecimalLiteral(token) {
      if (token.match(/^[0-9]+/)){
        var n = parseInt(token,10);
        if (n > 0x7fffffff) {
          throw lineNum + ":Literal value out of bounds:" + token;
        }
        this.use(n|LIT);
        return true;
      } else {
        return false;
      }
    }
*/
    parseHexLiteral(token, lineNum) {
      if (token.match(/^i\(0x[0-9a-f]{1,8}\)/)){
        var symbolToken = token.substring(2,token.length-1);
        var n = parseInt(symbolToken,16);
        if (n > 0x7fffffff) {
          throw lineNum + ":Literal value out of bounds:" + token;
        }
        this.use(n|LIT);
        return true;
      } else {
        return false;
      }
    }

    parseCatch(token) {
      if (this.refMatch('catch', token)) {
        var symbolToken = token.substring(6,token.length-1);
        return this.parseRef(symbolToken, CATCH);
      } else {
        return false;
      }
    }

  };

  class PrgElems {
    constructor() {
      this.cursor = 0;
      this.elems = [];
    };

    putElem(n, index) { //FIXME not robust
      this.elems[index] = n;
    }

    addElem(n) {
      this.cursor = this.elems.push(n);
    }

    getElem(index) {
      return this.elems[index];
    }

    topElem() {
      return this.elems[-1];
    }

    size() {
      return this.elems.length;
    }

    toBuf() {
      return new Uint32Array(this.elems).buffer;
    }

    toString() {
      var str = ":";
      var dv = new DataView(this.toBuf());
      for (var i = 0; i < dv.byteLength; i+=WD_BYTES) {
        str += modFmt.hex8(dv.getUint32(i, true)) + ":";
      }
      return str;
    }

  };

  return {
    makeFVMA: function(fnMsg) {
      return new FVMA(fnMsg);
    }
  };

})(); // modFVMA

