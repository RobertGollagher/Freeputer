/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvma.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170611
 * Updated:    20170616-0739+
 * Version:    pre-alpha-0.0.0.8 for FVM 2.0
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

/*
Design notes:

  - this is a bold experimental design based on absolute minimalism:
      - extremely simple one-pass assembler requiring only a few bytes of memory (easily run anywhere)
      - everything is done by hand using **human intelligence** (possibly helped by a preprocessor or validator)
      - no definitions, no declarations, no symbol tables, no labels (only relative and block-absolute and absolute jumps)
      - this is a total commitment to the principal of YAGNI (which delivers extreme portability)
      - purpose is just enough to bootstrap higher languages later (and not one iota more)
      - to facilitate this the compromises are:
          - coding will be done in SMALL blocks (called phrases) of constant size padded with nops
          - therefore, factoring and the use of composition will be EXTREME
          - clever assembler design will allow block relocation by hand
          - what is coded later will build on what is coded earlier
          - changing what was coded earlier will be rare
          - strategies for reuse will be employed
          - 1 line = 1 instruction

*/

// Module modFVMA will provide a Freeputer Assembler implementation.
var modFVMA = (function () { 'use strict';

  const PHRSIZE = 16;
  const COMSTART = '(';
  const COMEND = ')';
  const COMWORD= '/';
  const SYMBOLS = {
    fal: 0x00,
    lit: 0x01,
    cal: 0x02,
    ret: 0x91,
    frt: 0x84,
    jmp: 0x03,
    nop: 0xfd,
    hal: 0xff,
    '---': 0x000000
  };

  class FVMA {
    constructor(fnMsg) {
      this.fnMsg = fnMsg;
      this.prgElems = new PrgElems();
      this.inComment = false;
    };

    asm(str) { // FIXME no enforcement yet
      //this.fnMsg('Parsing...');
      var lines = str.split(/\n/);
      try {
        for (var i = 0; i < lines.length; i++) {
          this.parseLine(lines[i], i+1);
        }
        //this.fnMsg(this.prgElems);
        //this.fnMsg('Melding...');
        this.prgElems.meld();
        //this.fnMsg(this.prgElems);
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
      this.prgElems.addElem(x);
    }

    parseToken(token, lineNum) {
      if (false) {
      } else if (this.inCmt(token)) {
      } else if (this.parseComment(token, lineNum)) {
      } else if (this.parseComword(token, lineNum)) {
      } else if (this.parseSymbol(token)) {
      } else if (this.parsePhrAbs(token)) {
      } else if (this.parsePhrnum(token)) {
      } else if (this.parseForw(token)) {
      } else if (this.parseBackw(token)) {
      } else if (this.parseHex2(token)) {
      } else if (this.parseHex6(token)) {
      } else {
        throw lineNum + ":Unknown symbol:" + token;
      }
    };

    parseSymbol(token) {
      if (SYMBOLS[token] >= 0){
        this.use(SYMBOLS[token]);
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

    parseHex2(token) {
      if (token.length == 4 && token.match(/0x[0-9a-f]{2}/)){
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

    parsePhrAbs(token) { // TODO check overflow or out of bounds and endless loop
      if (token.length == 8 && token.match(/0n[0-9a-f]{6}/)){
        var asHex = token.replace('0n','0x');
        var n = parseInt(asHex,16);
        var m = Math.floor((this.prgElems.cursor/2)/PHRSIZE) + n;
        this.use(m);
        return true;
      } else {
        return false;
      }      
    }

    parseForw(token) { // TODO check overflow or out of bounds and endless loop
      if (token.length == 8 && token.match(/0f[0-9a-f]{6}/)){
        var asHex = token.replace('0f','0x');
        var n = parseInt(asHex,16);
        var m = this.prgElems.cursor/2 + n;
        this.use(m);
        return true;
      } else {
        return false;
      }      
    }

    parsePhrnum(token) { // TODO check overflow or out of bounds and endless loop
      if (token.length == 8 && token.match(/0p[0-9a-f]{6}/)){
        var asHex = token.replace('0p','0x');
        var n = parseInt(asHex,16);
        var m = n*PHRSIZE;
        this.use(m);
        return true;
      } else {
        return false;
      }      
    }

    parseBackw(token) { // TODO check overflow or out of bounds and endless loop
      if (token.length == 8 && token.match(/0r[0-9a-f]{6}/)){
        var asHex = token.replace('0r','0x');
        var n = parseInt(asHex,16);
        var m = this.prgElems.cursor/2 - n -1;
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

    addElem(n) {
      this.cursor = this.elems.push(n);
    }

    topElem() {
      return this.elems[-1];
    }

    meld() {
      var melded = [];
      for (var i = 0; i < this.elems.length-1; i=i+2) {
        melded.push(this.elems[i] + (this.elems[i+1] << 8));
      }
      this.elems = melded;
    }

    toBuf() {
      return new Uint32Array(this.elems).buffer;
    }

    toString() {
      var str = " ";
      this.elems.forEach(x => str = str + modFmt.hex8(x) + "\n");
      return str;
    }

  };

  return {
    makeFVMA: function(fnMsg) {
      return new FVMA(fnMsg);
    }
  };

})(); // modFVMA

