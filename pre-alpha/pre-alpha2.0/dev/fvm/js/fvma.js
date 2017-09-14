/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvma.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170611
 * Updated:    20170915+
 * Version:    pre-alpha-0.0.1.3+ for FVM 2.0
 *
 *                     This Edition of the Assembler:
 *                                JavaScript
 *                           for HTML 5 browsers
 * 
 *                                ( ) [ ] { }
 *
 *   FIXME Only partially implemented, just enough for the test program.
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

  const DEF = '#def';
  const HERE = '.';
  const COMSTART = '/*';
  const COMEND = '*/';
  const COMWORD= '/';

  const IM    = 0x80000000|0
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

  const SYMBOLS = { // Note: simple only here, complex in code below
    nop:    NOP,
    add:    ADD,
    tor:    TOR,
    fromb:  FROMB,
    halt:   HALT
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
      this.inComment = false;
      this.dict = {};
      this.expectDecl = false;
      this.expectDef = false;
      this.Decl = "";
    }

    asm(str) { // FIXME no enforcement yet
      this.reset();
      this.fnMsg('Parsing...');
      var lines = str.split(/\n/);
      try {
        for (var i = 0; i < lines.length; i++) {
          this.parseLine(lines[i], i+1);
        }
        this.fnMsg(this.prgElems);
        this.fnMsg('Dictionary...');
        this.fnMsg(JSON.stringify(this.dict));
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
    }

    use(x) {
      if (this.expectDef) {
       if (x === HERE) { // FIXME store symbol only as a 32-bit word to save space
          // note: nowadays using byte addressing
         this.dict[this.decl] = (this.prgElems.cursor / 3);
       } else {
         this.dict[this.decl] = x;
       }
      if (this.decl == 0) { // TODO check corner cases
        if (this.dict[this.decl] > 1) {
          this.prgElems.putElem(SYMBOLS['jmp'],3);
          this.prgElems.putElem(0|0,4); // FIXME JADE
          this.prgElems.putElem(this.dict[this.decl],5);
        } 
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
      } else if (this.parseComword(token, lineNum)) {
      } else if (this.expectingDecl(token, lineNum)) {
      } else if (this.expectingCond(token, lineNum)) {
      } else if (this.parseForw(token)) {
      } else if (this.parseBackw(token)) {
      } else if (this.parseDef(token)) {
      } else if (this.parseRef(token)) {
      } else if (this.parseHere(token, lineNum)) {
      } else if (this.parseImm(token)) {
      } else if (this.parseRpt(token)) {
      } else if (this.parseHex2(token)) {
      } else if (this.parseHex4(token)) {
      } else if (this.parseHex6(token)) {
      } else if (this.parseHex8(token)) {
      } else {
        throw lineNum + ":Unknown symbol:" + token;
      }
    };

    symbolToInt(str) {
      if (str.length == 6 && str.match(/0s[0-9a-f]{4}/)){
        var asHex = str.replace('0s','0x');
        var intValue = parseInt(asHex,16);
        return intValue;
      } else {
        throw "Assembler bug in symbolToInt(str) for:" + str;
      }
    }

    intToSymbol(i) {
      if (i >= 0x0000 && i <= 0xffff){
        var str = '0s' + ('0000' + i.toString(16)).substr(-4);
        return str;
      } else {
        throw "Assembler bug in intToSymbol(int) for:" + i;
      }
    }

    expectingDecl(token, lineNum) {
      if (this.expectDecl) {
        var intValue;
        if (token.length == 6 && token.match(/0s[0-9a-f]{4}/)){
          intValue = this.symbolToInt(token);
        } else {
          throw lineNum + ":Illegal symbol format (must be like 0s0001):" + token;
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
 
     parseRef(token) {
       if (SYMBOLS[token] >= 0){
           this.use(SYMBOLS[token]);
           return true;
       } else if (token.length == 6 && token.match(/0s[0-9a-f]{4}/) && this.dict[this.symbolToInt(token)] >= 0){
         this.use(this.dict[this.symbolToInt(token)]);
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
        if (this.inComment) {
          if (token == COMEND){
            this.inComment = false;
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
        this.inComment = true;
        return true;
      } else {
        return false;
      }      
    }

    parseComword(token, lineNum) {
      if (token.startsWith(COMWORD)) {
        return true;
      } else {
        return false;
      }
    }

    parseImm(token) {
      if (token.match(/i\([^\s]+\)/)){
        var n = parseInt(token.substring(2,token.length-1)); //FIXME
        n = n | IM;
        this.use(n);
        return true;
      } else {
        return false;
      }
    }

    parseRpt(token) {
      if (token.match(/rpt\([^\s]+\)/)){
        var n = parseInt(token.substring(4,token.length-1)); //FIXME
        n = n | RPT;
        this.use(n);
        return true;
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

    topElem() {
      return this.elems[-1];
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

