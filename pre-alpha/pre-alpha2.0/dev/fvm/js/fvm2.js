/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvm2.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170303
 * Updated:    20170401-0943
 * Version:    pre-alpha-0.0.0.5 for FVM 2.0
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

  class FVM {
    constructor(config) {
      this.fnStdout = config.fnStdout;
      this.fnUsrout = config.fnUsrout;
      this.fnGrdout = config.fnGrdout;
      this.fnLog = config.fnLog;
      this.fnTrc = config.fnTrc;
    };

    run() {
      this.fnTrc('FVM ran\n');
    };
  }

  class Config {
    constructor() {
      this.fnStdout = null;
      this.fnUsrout = null;
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







