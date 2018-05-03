#!/bin/bash
m4 -d exampleProgram.m4 > exampleProgram.c && make good OBJ=fvm2 && time ./fvm2; echo $?

