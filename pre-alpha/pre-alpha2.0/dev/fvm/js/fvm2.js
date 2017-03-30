/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 * 
 * Program:    fvm2.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20170303
 * Updated:    20170330-1842
 * Version:    pre-alpha-0.0.0.2 for FVM 2.0
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
    constructor(fnStdout, fnUsrout, fnGrdout, fnLog, fnTrc) {
      this.fnStdout = fnStdout;
      this.fnUsrout = fnUsrout;
      this.fnGrdout = fnGrdout;
      this.fnLog = fnLog;
      this.fnTrc = fnTrc;
    };

    run() {
      this.fnTrc('FVM ran\n');
    };
  }

  return {
    makeFVM: function(fnStdout, fnUsrout, fnGrdout, fnLog, fnTrc) {
      return  new FVM(fnStdout, fnUsrout, fnGrdout, fnLog, fnTrc);
    }
  };

})(); // modFVM







