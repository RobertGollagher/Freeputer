#
# xt.sh
# =====
# 
# A script to conveniently generate subsets of the 'fvmtest.fl' test suite
# for running on FVM Lite one at a time. This is needed because FVM Lite,
# unlike FVM Heavy, does not have enough ROM to run the whole fvmtest
# test suite at once. See 'fvmtest.fl' for details of the suite.
# 
# Copyright © Robert Gollagher 2016
# Author :    Robert Gollagher   robert.gollagher@freeputer.net
# Created:    20160511
# Updated:    20160512:1419
# Version:    pre-alpha-0.0.0.2 for FVM 1.1
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# [at your option] any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# 
# ============================================================================
# 
# WARNING: This is pre-alpha software and as such may well be incomplete,
# unstable and unreliable. It is considered to be suitable only for
# experimentation and nothing more.
# 
# ============================================================================
# 
# Instructions
# ============
#
# In the same directory as a copy of 'fvmtest.fl',
# run this 'xt.sh' bash script by:
#
#   ./xt.sh
#
# It generates numbered Freelang source files, each one representing
# one subset of the fvmtest test suite. It does this by using the special
# comments already extant in 'fvmtest.fl' which demarkate subsets of
# sizes suitable for running on FVM Lite. For example it generates
# '1-fvmtest.fl' which is the first subset of the test suite.
#
# This script then automatically compiles those subsets and incorporates
# each of them into a numbered fvm instance such as '1-fvm'. Note: you
# would use a different approach if not using FVMO_INCORPORATE_ROM
# (see 'fvm.c') as then you would need to use rom files.
# 
# Actually running all these fvmtest subsections is then a manual process
# (although you could easily script it on Linux but not on the Arduino IDE).
# Assuming you have a subdirectory called fvmtest which contains all
# that is normally needed to run fvmtest (see 'fvmtest.fl') then
# you can run each one, one at a time, by:
#
#   cp 1-fvm fvmtest/fvm
#   cd fvmtest
#   ./fvmtest.sh
#
# Obviously all this is a very tedious process compared to the quick and
# easy running of fvmtest for FVM Heavy all in one go. However, the purpose
# of running fvmtest is to see if an FVM implementation is behaving
# correctly, so it is not something that needs to be done often
# except if you are porting the FVM to a new platform or for
# the first time testing an FVM implementation.
# 
# ============================================================================

# ============================================================================
#   CREATE TEMPORARY FILES 'xt-fvmtest.fl' AS BASIS FOR OTHER FILES.
#   In this temporary file, all the subsections which will subsequently
#   get automatically activated (uncommented) one at time are
#   enclosed by ((( ... ))) in bulk to comment them out.
#
#   Note: some parts of the source are already (hard-coded as) commented out
#   by ((( ... ))) and therefore do not use this XTFVMLITE--- mechanism.
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ ((( XTFVMLITE===
#
#   \ ===XTFVMLITE )))
sed 's/[(][(][(] XTFVMLITE===/((( XTFVMLITE=== )))/' fvmtest.fl |
sed 's/===XTFVMLITE [)][)][)]/((( ===XTFVMLITE )))/' |

# Automatically deactivate (comment out) code between:
#   \ ((( XTFVMLITE---
#
#   \ ---XTFVMLITE )))
sed 's/\\ [(][(][(] XTFVMLITE---/((( XTFVMLITE---/' |
sed 's/\\ ---XTFVMLITE [)][)][)]/---XTFVMLITE )))/' > xt-fvmtest.fl

# ============================================================================
#   CREATE '1-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S1_FVMLITE===
#
#   \ ===XT_S1_FVMLITE
sed 's/\\ XT_S1_FVMLITE===/XT_S1_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S1_FVMLITE/((( ===XT_S1_FVMLITE/' > 1-fvmtest.fl

# ============================================================================
#   CREATE '2-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S2_FVMLITE===
#
#   \ ===XT_S2_FVMLITE
sed 's/\\ XT_S2_FVMLITE===/XT_S2_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S2_FVMLITE/((( ===XT_S2_FVMLITE/' > 2-fvmtest.fl

# ============================================================================
#   CREATE '3-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S3_FVMLITE===
#
#   \ ===XT_S3_FVMLITE
sed 's/\\ XT_S3_FVMLITE===/XT_S3_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S3_FVMLITE/((( ===XT_S3_FVMLITE/' > 3-fvmtest.fl

# ============================================================================
#   CLEAN UP
# ============================================================================
# Delete intermediate file
# FIXME rm xt-fvmtest.fl

# ============================================================================
#   BUILD FVMs: Make a rom and incorporate it into an fvm executable
#               for each source file. This will only work if the
#               usual required dependencies are present.
#
#   Note: of course, this BUILD FVMs section is useless for Arduino FVMs
#   as you will have to build each one manually using the Arduino IDE.
#   However, this BUILD FVMs section is very convenient on Linux.
# ============================================================================
./romMake.sh 1-fvmtest.fl && make && cp rom.fp 1-rom.fp && cp fvm 1-fvm
./romMake.sh 2-fvmtest.fl && make && cp rom.fp 2-rom.fp && cp fvm 2-fvm
./romMake.sh 3-fvmtest.fl && make && cp rom.fp 3-rom.fp && cp fvm 3-fvm

