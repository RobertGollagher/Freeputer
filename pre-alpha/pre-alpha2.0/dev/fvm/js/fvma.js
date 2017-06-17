/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvma.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170611
 * Updated:    20170617-1137+
 * Version:    pre-alpha-0.0.0.11 for FVM 2.0
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
      - IMPRACTICAL: everything is done by hand using **human intelligence** (possibly helped by a preprocessor or validator)
      - IMPRACTICAL: no definitions, no declarations, no symbol tables, no labels (only relative and block-absolute and absolute jumps)
      - this is a total commitment to the principal of YAGNI (which delivers extreme portability)
      - purpose is just enough to bootstrap higher languages later (and not one iota more)
      - to facilitate this the compromises are:
          - IMPRACTICAL: coding will be done in SMALL blocks (called phrases) of constant size padded with nops
          - therefore, factoring and the use of composition will be EXTREME
          - IMPRACTICAL: clever assembler design will allow block relocation by hand
          - what is coded later will build on what is coded earlier
          - IMPRACTICAL: changing what was coded earlier will be rare
          - strategies for reuse will be employed
          - BORDERLINE: 1 line = 1 instruction

Modifications:

  - Experimentation has shown:
      - declarations and definitions are necessary for reasonable productivity (especially labels but some others too)
      - failing to use them would just shift the burden to sophisticated text editors (not worthwhile overall)
      - using fixed-size phrases carries too high a runtime burden (too much RAM needed for loading programs)
      - bug-fixing requires being able to change what was coded earlier (in place)
  - Practical upshot of this is:
      - added #def back into the assembler
      - TODO remove phrase functionality

Jury is still out on:

  - Do we need to support forward references to labels or not?
      - Intuitive answer: no, except 1 label to start, because:
          - forward references encourage monolithic design and tangled dependencies, which is bad
          - can use relative forwards instead within molecules
          - in theory all we need is start for the entry point
  
*/

// Module modFVMA will provide a Freeputer Assembler implementation.
var modFVMA = (function () { 'use strict';

  const DEF = '#def';
  const HERE = '.';
  const PHRSIZE = 16; // FIXME
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
        //this.fnMsg(this.prgElems);
        //this.fnMsg('Melding...');
        this.prgElems.meld();
        //this.fnMsg(this.prgElems);
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
         this.dict[this.decl] = this.prgElems.cursor / 2;
       } else {
         this.dict[this.decl] = x;
       }
      if (this.decl === '0s000000') { // FIXME handling start decl for entry point
        if (this.dict[this.decl] > 1) {
          this.prgElems.putElem(SYMBOLS['jmp'],2);
          this.prgElems.putElem(this.dict[this.decl],3);
        } 
      }
       this.decl = "";
       this.expectDef = false;
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
      } else if (this.parsePhrAbs(token)) {
      } else if (this.parsePhrnum(token)) {
      } else if (this.parseForw(token)) {
      } else if (this.parseBackw(token)) {
      } else if (this.parseDef(token)) {
      } else if (this.parseRef(token)) {
      } else if (this.parseHere(token)) {
      } else if (this.parseHex2(token)) {
      } else if (this.parseHex6(token)) {
      } else {
        throw lineNum + ":Unknown symbol:" + token;
      }
    };

    expectingDecl(token, lineNum) {
      if (this.expectDecl) {
        if (this.dict[token]) {
          throw lineNum + ":Already defined:" + token;
        } else {
          this.decl = token;
          this.expectDecl = false;
          this.expectDef = true;
        }
        return true;
      }
      return false;
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
       } else if (this.dict[token] >= 0){
         this.use(this.dict[token]);
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
        melded.push(this.elems[i] + (this.elems[i+1] << 8));
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

