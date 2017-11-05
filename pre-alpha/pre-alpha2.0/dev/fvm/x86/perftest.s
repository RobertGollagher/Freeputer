/*
Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    perftest.s
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170902
Updated:    20170902++
Version:    pre-alpha-0.0.0.2+ for FVM 2.0

This is an example program using the 'qmisc.s' virtual machine definition.
This example program is a performance test of noop repeats.
See 'qmisc.c' for build instructions.

Build and run by:

  ./go.sh perftest

This requires the build flag NO_PROGRAM to be NO in 'qmisc.c'.

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
.include "qmisc.s"

# Runs in less than 1.4 seconds
r(0x7fffffff)
foo:
.ifeq TRACING_ENABLED
  TRACE_INSTR
.endif
  noop
  rpt foo
jmp vm_success
