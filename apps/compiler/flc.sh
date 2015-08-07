#!/bin/bash
# Compile Freelang source file $1 to create Freeputer rom $2
#   (read 'dev/freelang/compiler/src/flc.sh' for more complete instructions)
./fvm16-0MB < $1 > $2

