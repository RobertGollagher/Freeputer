/*
 * Copyright © 2017, Robert Gollagher.
 * SPDX-License-Identifier: GPL-3.0+
 *
 * Program:    fpnode.js
 * Author :    Robert Gollagher   robert.gollagher@freeputer.net
 * Created:    20180515
 * Updated:    20180515+
 * Version:    pre-alpha-0.0.0.1+ for FVM 2.0
 *
 *                               This Edition:
 *                                JavaScript
 *                               for Node.js
 *
 *                                ( ) [ ] { }
 *
 * This script allows the 'fvm2.js' implementation of the FVM to be run
 * (without modification) within Node.js. Thus the exact same 'fvm2.js'
 * implementation can be run within the browser (see 'fvmui.html')
 * or within Node.js, whichever is desired, very conveniently.
 *
 * As an initial demonstration this 'fpnode.js' shall simply use Node's
 * process.stdin, process.stdout and process.stderr for all I/O.
 *
 * This 'fpnode.js' is for use by Node.js to run the FVM thus:
 *
 *   node fpnode.js
 *
 * However, it is better to redirect stderr thus:
 *
 *   node fpnode.js 2> std.trc
 *
 * ===========================================================================
 *
 * WARNING: This is pre-alpha software and as such may well be incomplete,
 * unstable and unreliable. It is considered to be suitable only for
 * experimentation and nothing more.
 *
 * ===========================================================================
 */

var execute = function() { 'use strict';

  const fnStdout = function(str) {
    process.stdout.write.call(process.stdout,str+"\n");
  }
  const fnStdin = function(str) {
    return process.stdin.read.call(process.stdin);
  }
  const fnTrc = function(str) {
    process.stderr.write.call(process.stderr,str+"\n");
  }

  // A tiny default program
  const TRON  = 0x45000000|0;
  const NOP   = 0x00000000|0;
  const HALT  = 0x36000000|0;
  const prg = new Uint32Array([TRON,NOP,HALT]).buffer;

  const modFVM = require ("./fvm2.js")
  const hold = "FIXMEdummyHold";
  const cf = modFVM.makeConfig(prg, hold);
  cf.fnStdout = fnStdout;
  cf.fnStdin = fnStdin;
  cf.fnTrc = fnTrc;
  const fvm = modFVM.makeFVM(cf);

  fnTrc('About to run FVM...');

  const exitCode = fvm.run();
  process.exitCode = exitCode;

  fnTrc('Ran VM. Exit code: ' + exitCode);

};
execute();
