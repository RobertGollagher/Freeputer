/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvma.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170611
 * Updated:    20171104+
 * Version:    pre-alpha-0.0.1.13+ for FVM 2.0
 *
 *                     This Edition of the Assembler:
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

// Module modFVMA will provide a Freeputer Assembler implementation.
var modFVMA = (function () { 'use strict';

  const WD_BYTES = 4;
  const ADDR_MASK = 0x00ffffff;

  const START = 's0000';
  const DEF = '#define';
  const HERE = '.';
  const COMSTART = '/*';
  const COMEND = '*/';
  const COMLINE= '//';
  //const COMWORD= '/';

  // const IM    = 0x80000000|0  // NO LONGER USED

  const IMMA  = 0x30000000|0
  const IMMB  = 0x31000000|0
  const IMMR  = 0x32000000|0
  const IMMT  = 0x33000000|0

  const PUSH  = 0x50000000|0
  const POP   = 0x51000000|0
  const DPUSH = 0x52000000|0
  const DPOP  = 0x53000000|0


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
  const JMPG  = 0x21000000|0
  const JMPL  = 0x22000000|0
  const JUMP  = 0x23000000|0
  const RPT   = 0x24000000|0
  const BR    = 0x25000000|0
  const IN    = 0x26000000|0 // FIXME make complex
  const OUT   = 0x27000000|0

  const CALL  = 0x60000000|0
  const RET   = 0x61000000|0

  const SYMBOLS = { // Note: simple only here, complex in code below
    nop:    NOP,
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
    incm:   INCM,
    decm:   DECM,
    at:     AT,
    copy:   COPY,
    inc:    INC,
    dec:    DEC,
    flip:   FLIP,
    swap:   SWAP,
    tob:    TOB,
    tor:    TOR,
    tot:    TOT,
    fromb:  FROMB,
    fromr:  FROMR,
    fromt:  FROMT,
    mem:    MEM,
    link:   LINK,
    halt:   HALT,
    br:     BR,
    in:     IN, // FIXME make complex
    out:    OUT,
    push:   PUSH,
    pop:    POP,
    dpush:  DPUSH,
    dpop:   DPOP,
    call:   CALL,
    ret:    RET
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
      this.Decl = "";
      // s0000 symbol is reserved for /*start*/ label...
      this.s0000label = null;
      // ...and it is the only forward reference supported.
      this.s0000refCell = null;
    }

    asm(str) { // FIXME no enforcement yet
      this.reset();
      this.fnMsg('Parsing...');
      var lines = str.split(/\n/);
      try {
        for (var i = 0; i < lines.length; i++) {
          this.parseLine(lines[i], i+1);
        }
        if (this.s0000refCell != null) {
          this.s0000label = this.dict[0];
          if (this.s0000label == null) {
            throw "Missing entry point s0000: /*start*/ referenced at cell:"
            + this.s0000refCell
            + "\nDeclare the entry point for your program with a s0000: /*start*/ label.";
          } else {
            // Populate the s0000: /*start*/ label referent into its referer
            var s0000Addr = this.s0000label&ADDR_MASK;
            var opcode = this.prgElems.getElem(this.s0000refCell);
            this.prgElems.putElem(opcode|s0000Addr,this.s0000refCell);
          }
        }
        // Uncomment next line to see hex dump
        //this.fnMsg(this.prgElems);
        this.fnMsg('Dictionary...');
        this.fnMsg(JSON.stringify(this.dict));
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
      //} else if (this.parseForw(token)) {
      //} else if (this.parseBackw(token)) {
      } else if (this.parseLabelDecl(token, lineNum)) {
      } else if (this.parseDef(token)) {
      } else if (this.parseRef(token)) {
      } else if (this.parseHere(token, lineNum)) {
      } else if (this.parseImmA(token)) {
      } else if (this.parseImmB(token)) {
      } else if (this.parseImmR(token)) {
      } else if (this.parseImmT(token)) {
      } else if (this.parseCall(token)) {
      } else if (this.parseJump(token)) {
      } else if (this.parseJmpA(token)) {
      } else if (this.parseJmpB(token)) {
      } else if (this.parseJmpE(token)) {
      } else if (this.parseJmpN(token)) {
      } else if (this.parseJmpG(token)) {
      } else if (this.parseJmpL(token)) {
      } else if (this.parseRpt(token)) {
      } else if (this.parseBr(token)) {
      } else if (this.parseHex2(token)) {
      } else if (this.parseHex4(token)) {
      } else if (this.parseHex6(token)) {
      } else if (this.parseHex8(token)) {
      } else {
        throw lineNum + ":Unknown symbol:" + token;
      }
    };

    symbolToInt(str) {
      if (str.length == 5 && str.match(/s[0-9a-f]{4}/)){
        var asHex = str.replace('s','0x');
        var intValue = parseInt(asHex,16);
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
        if (token.length == 5 && token.match(/s[0-9a-f]{4}/)){
          intValue = this.symbolToInt(token);
        } else {
          throw lineNum + ":Illegal symbol format (must be like s0001):" + token;
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

     parseLabelDecl(token, lineNum) { // TODO refactor this whole assembler later
        var intValue;
        if (token.length == 6 && token.match(/s[0-9a-f]{4}:/)){
          intValue = this.symbolToInt(token.substring(0,token.length-1));
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
       } else if (token.length == 5 && token.match(/s[0-9a-f]{4}/) && this.dict[this.symbolToInt(token)] >= 0){
           var n = this.dict[this.symbolToInt(token)];
           n = n | opcode;
           this.use(n);
           return true;
       } else if (token === START) { // Special case
           this.s0000refCell = this.prgElems.cursor;
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

    parseImmA(token) {
      if (token.match(/a\(s[^\s]+\)/)){ // FIXME broken now we are VW32/64; also make more strict
        var symbolToken = token.substring(2,token.length-1);
        return this.parseRef(symbolToken, IMMA);
      } else if (token.match(/a\([^\s]+\)/)){
        var n = parseInt(token.substring(2,token.length-1)); //FIXME
        this.use(IMMA); // FIXME here moving to VW32/64 instr encoding
        this.use(n);
        return true;
      } else {
        return false;
      }
    }

    parseImmB(token) {
      if (token.match(/b\(s[^\s]+\)/)){ // FIXME broken now we are VW32/64; also make more strict
        var symbolToken = token.substring(2,token.length-1);
        return this.parseRef(symbolToken, IMMB);
      } else if (token.match(/b\([^\s]+\)/)){
        var n = parseInt(token.substring(2,token.length-1)); //FIXME
        this.use(IMMB); // FIXME here moving to VW32/64 instr encoding
        this.use(n);
        return true;
      } else {
        return false;
      }
    }

    parseImmR(token) {
      if (token.match(/r\(s[^\s]+\)/)){ // FIXME broken now we are VW32/64; also make more strict
        var symbolToken = token.substring(2,token.length-1);
        return this.parseRef(symbolToken, IMMR);
      } else if (token.match(/r\([^\s]+\)/)){
        var n = parseInt(token.substring(2,token.length-1)); //FIXME
        this.use(IMMR); // FIXME here moving to VW32/64 instr encoding
        this.use(n);
        return true;
      } else {
        return false;
      }
    }

    parseImmT(token) {
      if (token.match(/t\(s[^\s]+\)/)){ // FIXME broken now we are VW32/64; also make more strict
        var symbolToken = token.substring(2,token.length-1);
        return this.parseRef(symbolToken, IMMT);
      } else if (token.match(/t\([^\s]+\)/)){
        var n = parseInt(token.substring(2,token.length-1)); //FIXME
        this.use(IMMT); // FIXME here moving to VW32/64 instr encoding
        this.use(n);
        return true;
      } else {
        return false;
      }
    }

    parseCall(token) {
      if (token.match(/call\(s[^\s]+\)/)){ // FIXME make more strict
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, CALL);
      } else if (token.match(/call\([^\s]+\)/)){
        var n = parseInt(token.substring(5,token.length-1)); //FIXME
        n = n | CALL;
        this.use(n);
        return true;
      } else {
        return false;
      }
    }

    parseJump(token) {
      if (token.match(/jump\(s[^\s]+\)/)){ // FIXME make more strict
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JUMP);
      } else if (token.match(/jump\([^\s]+\)/)){
        var n = parseInt(token.substring(5,token.length-1)); //FIXME
        n = n | JUMP;
        this.use(n);
        return true;
      } else {
        return false;
      }
    }

    parseJmpA(token) {
      if (token.match(/jmpa\(s[^\s]+\)/)){ // FIXME make more strict
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JMPA);
      } else if (token.match(/jmpa\([^\s]+\)/)){
        var n = parseInt(token.substring(5,token.length-1)); //FIXME
        n = n | JMPA;
        this.use(n);
        return true;
      } else {
        return false;
      }
    }

    parseJmpB(token) {
      if (token.match(/jmpb\(s[^\s]+\)/)){ // FIXME make more strict
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JMPB);
      } else if (token.match(/jmpb\([^\s]+\)/)){
        var n = parseInt(token.substring(5,token.length-1)); //FIXME
        n = n | JMPB;
        this.use(n);
        return true;
      } else {
        return false;
      }
    }

    parseJmpE(token) {
      if (token.match(/jmpe\(s[^\s]+\)/)){ // FIXME make more strict
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JMPE);
      } else if (token.match(/jmpe\([^\s]+\)/)){
        var n = parseInt(token.substring(5,token.length-1)); //FIXME
        n = n | JMPE;
        this.use(n);
        return true;
      } else {
        return false;
      }
    }

    parseJmpN(token) {
      if (token.match(/jmpn\(s[^\s]+\)/)){ // FIXME make more strict
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JMPN);
      } else if (token.match(/jmpn\([^\s]+\)/)){
        var n = parseInt(token.substring(5,token.length-1)); //FIXME
        n = n | JMPN;
        this.use(n);
        return true;
      } else {
        return false;
      }
    }

    parseJmpG(token) {
      if (token.match(/jmpg\(s[^\s]+\)/)){ // FIXME make more strict
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JMPG);
      } else if (token.match(/jmpg\([^\s]+\)/)){
        var n = parseInt(token.substring(5,token.length-1)); //FIXME
        n = n | JMPG;
        this.use(n);
        return true;
      } else {
        return false;
      }
    }

    parseJmpL(token) {
      if (token.match(/jmpl\(s[^\s]+\)/)){ // FIXME make more strict
        var symbolToken = token.substring(5,token.length-1);
        return this.parseRef(symbolToken, JMPL);
      } else if (token.match(/jmpl\([^\s]+\)/)){
        var n = parseInt(token.substring(5,token.length-1)); //FIXME
        n = n | JMPL;
        this.use(n);
        return true;
      } else {
        return false;
      }
    }

    parseRpt(token) { // Only allows label symbols not raw numbers here
      if (token.match(/rpt\(s[^\s]+\)/)){
        var symbolToken = token.substring(4,token.length-1);
        return this.parseRef(symbolToken, RPT);
      } else {
        return false;
      }
    }

    parseBr(token) { // Only allows label symbols not raw numbers here
      if (token.match(/br\(s[^\s]+\)/)){
        var symbolToken = token.substring(3,token.length-1);
        return this.parseRef(symbolToken, BR);
      } else {
        return false;
      }
    }

    parseHex8(token) {
      if (token.length == 10 && token.match(/0x[0-9a-f]{8}/)){
        var n = parseInt(token,16);
        this.use(n);
        return true;
      } else {
        return false;
      }
    }

    parseHex6(token) {
      if (token.length == 8 && token.match(/0x[0-9a-f]{6}/)){
        var n = parseInt(token,16);
        this.use(n);
        return true;
      } else {
        return false;
      }      
    }

    parseHex4(token) {
      if (token.length == 6 && token.match(/0x[0-9a-f]{4}/)){
        var n = parseInt(token,16);
        this.use(n);
        return true;
      } else {
        return false;
      }      
    }

    parseHex2(token) {
      if (token.length == 4 && token.match(/0x[0-9a-f]{2}/)){
        var n = parseInt(token,16);
        this.use(n);
        return true;
      } else {
        return false;
      }      
    }
/*
    parseForw(token) { // TODO check overflow or out of bounds and endless loop
      if (token.length == 4 && token.match(/0f[0-9a-f]{2}/)){
        var asHex = token.replace('0f','0x');
        var n = parseInt(asHex,16);
        var m = (Math.floor(this.prgElems.cursor/3)) + n;
        this.use(m);
        return true;
      } else {
        return false;
      }      
    }

    parseBackw(token) { // TODO check overflow or out of bounds and endless loop
      if (token.length == 4 && token.match(/0r[0-9a-f]{2}/)){
        var asHex = token.replace('0r','0x');
        var n = parseInt(asHex,16);
        var m = (Math.floor(this.prgElems.cursor/3)) - n;
        this.use(m);
        return true;
      } else {
        return false;
      }      
    }*/
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

