/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvma.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170611
 * Updated:    20180515+
 * Version:    pre-alpha-0.0.1.64+ for FVM 2.0
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

  // FIXME rationalize opcode order (these initial allocations are arbitrary)
  const NOP   = 0x00000000|0
  const DO    = 0x01000000|0
  const DONE  = 0x02000000|0
  const RPT   = 0x03000000|0
  const CPUSH = 0x04000000|0
  const CPOP  = 0x05000000|0
  const CPEEK = 0x06000000|0
  const CDROP = 0x07000000|0
  const TPUSH = 0x08000000|0
  const TPOP  = 0x09000000|0
  const TPEEK = 0x0a000000|0
  const TPOKE = 0x0b000000|0
  const TDROP = 0x0c000000|0
  const IM    = 0x80000000|0  // note: 0x0000000d available
  const DROP  = 0x0e000000|0
  const SWAP  = 0x0f000000|0
  const OVER  = 0x10000000|0
  const ROT   = 0x11000000|0
  const DUP   = 0x12000000|0
  const GET   = 0x13000000|0
  const PUT   = 0x14000000|0
  const GETI  = 0x15000000|0
  const PUTI  = 0x16000000|0
  const ROM   = 0x17000000|0
  const ADD   = 0x18000000|0
  const SUB   = 0x19000000|0
  const MUL   = 0x1a000000|0
  const DIV   = 0x1b000000|0
  const MOD   = 0x1c000000|0
  const INC   = 0x1d000000|0
  const DEC   = 0x1e000000|0
  const OR    = 0x1f000000|0
  const AND   = 0x20000000|0
  const XOR   = 0x21000000|0
  const FLIP  = 0x22000000|0
  const NEG   = 0x23000000|0
  const SHL   = 0x24000000|0
  const SHR   = 0x25000000|0
  const HOLD  = 0x26000000|0
  const GIVE  = 0x27000000|0
  const IN    = 0x28000000|0
  const OUT   = 0x29000000|0
  const INW   = 0x2a000000|0
  const OUTW  = 0x2b000000|0
  const JUMP  = 0x2c000000|0
  const JNAN  = 0x2d000000|0
  const JORN  = 0x2e000000|0
  const JANN  = 0x2f000000|0
  const JNNE  = 0x30000000|0
  const JMPE  = 0x31000000|0
  const JNNZ  = 0x32000000|0
  const JNNP  = 0x33000000|0
  const JNNG  = 0x34000000|0
  const JNNL  = 0x35000000|0
  const HALT  = 0x36000000|0
  const FAIL  = 0x37000000|0
  const SAFE =  0x38000000|0
  const DSA   = 0x39000000|0
  const DSE   = 0x3a000000|0
  const TSA   = 0x3b000000|0
  const TSE   = 0x3c000000|0
  const CSA   = 0x3d000000|0
  const CSE   = 0x3e000000|0
  const RSA   = 0x3f000000|0
  const RSE   = 0x40000000|0
  const PMI   = 0x41000000|0
  const DMW   = 0x42000000|0
  const RMW   = 0x43000000|0
  const HW    = 0x44000000|0
  const TRON  = 0x45000000|0
  const TROFF = 0x46000000|0


  const SYMBOLS = { // Note: simple only here, complex in code below
    nop:    NOP,
    do:     DO,
    done:   DONE,
    // RPT,
    cpush:  CPUSH,
    cpop:   CPOP,
    cpeek:  CPEEK,
    cdrop:  CDROP,
    tpush:  TPUSH,
    tpop:   TPOP,
    tpeek:  TPEEK,
    tpoke:  TPOKE,
    trop:   TDROP,
    // IM,
    drop:   DROP,
    swap:   SWAP,
    over:   OVER,
    rot:    ROT,
    dup:    DUP,
    get:    GET,
    put:    PUT,
    geti:   GETI,
    puti:   PUTI,
    rom:    ROM,
    add:    ADD,
    sub:    SUB,
    mul:    MUL,
    div:    DIV,
    mod:    MOD,
    inc:    INC,
    dec:    DEC,
    or:     OR,
    and:    AND,
    xor:    XOR,
    flip:   FLIP,
    neg:    NEG,
    shl:    SHL,
    shr:    SHR,
    hold:   HOLD,
    give:   GIVE,
    // IN,
    // OUT,
    // INW,
    // OUTW,
    // JUMP,
    // JNAN,
    // JORN,
    // JANN,
    // JNNE,
    // JMPE,
    // JNNZ,
    // JNNP,
    // JNNG,
    // JNNL,
    halt:   HALT,
    fail:   FAIL,
    safe:   SAFE,
    dsa:    DSA,
    dse:    DSE,
    tsa:    TSA,
    tse:    TSE,
    csa:    CSA,
    cse:    CSE,
    rsa:    RSA,
    rse:    RSE,
    pmi:    PMI,
    dmw:    DMW,
    rmw:    RMW,
    hw:     HW,
    tron:   TRON,
    troff:  TROFF

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
      } else if (this.parseModuleNum(token, lineNum)) {
      } else if (this.parseModuleStart(token,lineNum)) {
      } else if (this.parseModuleEnd(token)) {
      } else if (this.disallowGlobals(token, lineNum)) {
      } else if (this.parseUnit(token)) {
      } else if (this.parseLabelDecl(token, lineNum)) {
      } else if (this.parseDef(token)) {
      } else if (this.parseRef(token)) {
      } else if (this.parseHere(token, lineNum)) {
      } else if (this.parseDo(token)) {
      } else if (this.parseJump(token)) {
      } else if (this.parseJnan(token)) {
      } else if (this.parseJorn(token)) {
      } else if (this.parseJann(token)) {
      } else if (this.parseJnne(token)) {
      } else if (this.parseJmpe(token)) {
      } else if (this.parseJnnz(token)) {
      } else if (this.parseJnnp(token)) {
      } else if (this.parseJnng(token)) {
      } else if (this.parseJnnl(token)) {
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

    parseModuleNum(token, lineNum) { // TODO stricter location logic
      var intValue
      if (token.match(/^as\(m[0-9a-f]{1,4}\)$/)) {
        var symbolToken = token.substring(3,token.length-1);
        intValue = this.symbolToInt(symbolToken);
        this.currentModuleNum = intValue;
        return true;
      } else {
        return false;
      }
    }

    disallowGlobals(token, lineNum) {
      if (this.currentModuleNum === null) {
        // Global code shall not be allowed. All code must be modular.
        throw lineNum + ":Global code is not allowed. You must move this to a module: " + token;
      } else {
        return false;
      }
    }

     parseModuleStart(token, lineNum) {
       if (token === 'module'){
/*
         if (this.currentModuleNum !== null) {
           throw lineNum + ":Cannot nest modules: " + token;
         }
*/
         this.clearLocalsu();
         return true;
       } else {
         return false;
       }      
     }

     parseModuleEnd(token) {
       if (token === 'endm'){
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
        if (token == 'unit' || token == 'endu'){ // FIXME weak logic
          this.clearLocals();
          return true;
        }
        return false;
     }

     parseLabelDecl(token, lineNum) { // TODO refactor this whole assembler later, add u
        var intValue;
        if (token === 'PUB(x0):') {
           this.x0label = this.prgElems.cursor;
           this.decl = ""; // FIXME redundant?
           this.expectDef = false; // FIXME redundant?
           return true;
        }
        if (token.match(/pri\([s][0-9a-f]{1,3}\):/)){ // FIXME sff limit make more robust
            intValue = this.symbolToInt(token.substring(4,token.length-2));
        } else if (token.match(/loc\([u][0-9a-f]{1,3}\):/)){ // FIXME uff limit make more robust
            intValue = this.symbolToInt(token.substring(4,token.length-2));
        } else if (token.match(/PUB\([x][0-9a-f]{1,4}\):/)){
            intValue = this.symbolToInt(token.substring(4,token.length-2));
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

    parseDo(token) {
      if (this.refMatch('do', token)) {
        var symbolToken = token.substring(3,token.length-1);
        return this.parseRef(symbolToken, DO);
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

    parseJnan(token) {
      if (this.refMatch('jnan', token)) {
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JNAN);
      } else {
        return false;
      }
    }

    parseJorn(token) {
      if (this.refMatch('jorn', token)) {
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JORN);
      } else {
        return false;
      }
    }

    parseJann(token) {
      if (this.refMatch('jann', token)) {
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JANN);
      } else {
        return false;
      }
    }

    parseJnne(token) {
      if (this.refMatch('jnne', token)) {
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JNNE);
      } else {
        return false;
      }
    }

    parseJmpe(token) {
      if (this.refMatch('jmpe', token)) {
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JMPE);
      } else {
        return false;
      }
    }

    parseJnnz(token) {
      if (this.refMatch('jnnz', token)) {
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JNNZ);
      } else {
        return false;
      }
    }

    parseJnnp(token) {
      if (this.refMatch('jnnp', token)) {
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JNNP);
      } else {
        return false;
      }
    }

    parseJnng(token) {
      if (this.refMatch('jnng', token)) {
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JNNG);
      } else {
        return false;
      }
    }

    parseJnnl(token) {
      if (this.refMatch('jnnl', token)) {
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JNNL);
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
        this.use(n|IM);
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
        this.use(n|IM);
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

