#! /bin/bash
./build.sh $1 && time ./$1 ; echo $?
