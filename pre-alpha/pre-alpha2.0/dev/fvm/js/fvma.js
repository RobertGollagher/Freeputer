/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvma.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170611
 * Updated:    20170701:2336+
 * Version:    pre-alpha-0.0.0.46+ for FVM 2.0
 *
 *                     This Edition of the Assembler:
 *                                JavaScript
 *                           for HTML 5 browsers
 * 
 *                                ( ) [ ] { }
 *
 *              Note: This implementation is only for Plan C, GOLD.
 *              FIXME not ported to Plan C, GOLD yet.
 *              Currently largely unimplemented!
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

  const DEF = '#def';
  const HERE = '.';
  const COMSTART = '(';
  const COMEND = ')';
  const COMWORD= '/';

  const WORD_SIZE_BYTES = 4;

  const iFAL = 0x00|0; // FIXME
  const iJMP = 0x01|0;
  const iHAL = 0x1f|0; // FIXME

  const SYMBOLS = {
    '--': 0x00|0,
    hal: iHAL,
    jmp: iJMP,
    fal: iFAL,
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
      //this.fnMsg('Parsing...');
      var lines = str.split(/\n/);
      try {
        for (var i = 0; i < lines.length; i++) {
          this.parseLine(lines[i], i+1);
        }
        this.fnMsg(this.prgElems);
        this.fnMsg('Melding...');
        this.prgElems.meld();
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
         this.dict[this.decl] = (this.prgElems.cursor / 2) * WORD_SIZE_BYTES;
       } else {
         this.dict[this.decl] = x;
       }
      if (this.decl == 0) { // TODO check corner cases
        if (this.dict[this.decl] > 1) {
          this.prgElems.putElem(SYMBOLS['jmp'],2);
          this.prgElems.putElem(this.dict[this.decl],3);
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
      //} else if (this.parseHex5(token)) {
      //} else if (this.parseHex6(token)) {
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

    parseHex8(token) {
      if (token.length == 10 && token.match(/0x[0-9a-f]{8}/)){
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

    parseForw(token) { // TODO check overflow or out of bounds and endless loop
      if (token.length == 4 && token.match(/0f[0-9a-f]{2}/)){
        var asHex = token.replace('0f','0x');
        var n = parseInt(asHex,16);
        var m = this.prgElems.cursor/2 + n;
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
        var m = this.prgElems.cursor/2 - n;
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

    meld() {
      var melded = [];
      for (var i = 0; i < this.elems.length-1; i=i+2) {
        // FIXME for Plan C GOLD this is just a hack and not working fully
        melded.push((this.elems[i] << 26) | (this.elems[i+1]));
      }
      this.elems = melded;
    }

    toBuf() {
      return new Uint32Array(this.elems).buffer;
    }

    toString() {
      var str = ":";
      this.elems.forEach(x => str = str + modFmt.hex8(x) + ":");
      return str;
    }

  };

  return {
    makeFVMA: function(fnMsg) {
      return new FVMA(fnMsg);
    }
  };

})(); // modFVMA

