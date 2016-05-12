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
# Updated:    20160512:1557
# Version:    pre-alpha-0.0.0.3 for FVM 1.1
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
# After running all the individual subsections, manually tally up the
# total number of FVM restarts (not counting any values of -1 as these
# indicate 0 restarts) and ensure the total is correct when compared
# to the total number of FVM restarts that would occur when running
# the fvmtest suite all in one go on FVM Heavy (but accounting for
# the fact that running the test suite in many small subsections
# causes an expected discrepancy in the total number of restarts
# because running each subsection is a start and stop).
# See the constant EXPECTED_RESETS in 'fvmtest.fl'.
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
#   CREATE '4-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S4_FVMLITE===
#
#   \ ===XT_S4_FVMLITE
sed 's/\\ XT_S4_FVMLITE===/XT_S4_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S4_FVMLITE/((( ===XT_S4_FVMLITE/' > 4-fvmtest.fl

# ============================================================================
#   CREATE '5-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S5_FVMLITE===
#
#   \ ===XT_S5_FVMLITE
sed 's/\\ XT_S5_FVMLITE===/XT_S5_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S5_FVMLITE/((( ===XT_S5_FVMLITE/' > 5-fvmtest.fl

# ============================================================================
#   CREATE '6-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S6_FVMLITE===
#
#   \ ===XT_S6_FVMLITE
sed 's/\\ XT_S6_FVMLITE===/XT_S6_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S6_FVMLITE/((( ===XT_S6_FVMLITE/' > 6-fvmtest.fl

# ============================================================================
#   CREATE '7-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S7_FVMLITE===
#
#   \ ===XT_S7_FVMLITE
sed 's/\\ XT_S7_FVMLITE===/XT_S7_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S7_FVMLITE/((( ===XT_S7_FVMLITE/' > 7-fvmtest.fl

# ============================================================================
#   CREATE '8-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S8_FVMLITE===
#
#   \ ===XT_S8_FVMLITE
sed 's/\\ XT_S8_FVMLITE===/XT_S8_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S8_FVMLITE/((( ===XT_S8_FVMLITE/' > 8-fvmtest.fl

# ============================================================================
#   CREATE '9-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S9_FVMLITE===
#
#   \ ===XT_S9_FVMLITE
sed 's/\\ XT_S9_FVMLITE===/XT_S9_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S9_FVMLITE/((( ===XT_S9_FVMLITE/' > 9-fvmtest.fl

# ============================================================================
#   CREATE '10-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S10_FVMLITE===
#
#   \ ===XT_S10_FVMLITE
sed 's/\\ XT_S10_FVMLITE===/XT_S10_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S10_FVMLITE/((( ===XT_S10_FVMLITE/' > 10-fvmtest.fl

# ============================================================================
#   CREATE '11-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S11_FVMLITE===
#
#   \ ===XT_S11_FVMLITE
sed 's/\\ XT_S11_FVMLITE===/XT_S11_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S11_FVMLITE/((( ===XT_S11_FVMLITE/' > 11-fvmtest.fl

# ============================================================================
#   CREATE '12-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S12_FVMLITE===
#
#   \ ===XT_S12_FVMLITE
sed 's/\\ XT_S12_FVMLITE===/XT_S12_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S12_FVMLITE/((( ===XT_S12_FVMLITE/' > 12-fvmtest.fl

# ============================================================================
#   CREATE '13-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S13_FVMLITE===
#
#   \ ===XT_S13_FVMLITE
sed 's/\\ XT_S13_FVMLITE===/XT_S13_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S13_FVMLITE/((( ===XT_S13_FVMLITE/' > 13-fvmtest.fl

# ============================================================================
#   CREATE '14-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S14_FVMLITE===
#
#   \ ===XT_S14_FVMLITE
sed 's/\\ XT_S14_FVMLITE===/XT_S14_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S14_FVMLITE/((( ===XT_S14_FVMLITE/' > 14-fvmtest.fl

# ============================================================================
#   CREATE '15-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S15_FVMLITE===
#
#   \ ===XT_S15_FVMLITE
sed 's/\\ XT_S15_FVMLITE===/XT_S15_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S15_FVMLITE/((( ===XT_S15_FVMLITE/' > 15-fvmtest.fl

# ============================================================================
#   CREATE '16-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S16_FVMLITE===
#
#   \ ===XT_S16_FVMLITE
sed 's/\\ XT_S16_FVMLITE===/XT_S16_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S16_FVMLITE/((( ===XT_S16_FVMLITE/' > 16-fvmtest.fl

# ============================================================================
#   CREATE '17-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S17_FVMLITE===
#
#   \ ===XT_S17_FVMLITE
sed 's/\\ XT_S17_FVMLITE===/XT_S17_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S17_FVMLITE/((( ===XT_S17_FVMLITE/' > 17-fvmtest.fl

# ============================================================================
#   CREATE '18-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S18_FVMLITE===
#
#   \ ===XT_S18_FVMLITE
sed 's/\\ XT_S18_FVMLITE===/XT_S18_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S18_FVMLITE/((( ===XT_S18_FVMLITE/' > 18-fvmtest.fl

# ============================================================================
#   CREATE '19-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S19_FVMLITE===
#
#   \ ===XT_S19_FVMLITE
sed 's/\\ XT_S19_FVMLITE===/XT_S19_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S19_FVMLITE/((( ===XT_S19_FVMLITE/' > 19-fvmtest.fl

# ============================================================================
#   CREATE '20-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S20_FVMLITE===
#
#   \ ===XT_S20_FVMLITE
sed 's/\\ XT_S20_FVMLITE===/XT_S20_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S20_FVMLITE/((( ===XT_S20_FVMLITE/' > 20-fvmtest.fl

# ============================================================================
#   CREATE '21-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S21_FVMLITE===
#
#   \ ===XT_S21_FVMLITE
sed 's/\\ XT_S21_FVMLITE===/XT_S21_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S21_FVMLITE/((( ===XT_S21_FVMLITE/' > 21-fvmtest.fl

# ============================================================================
#   CREATE '22-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S22_FVMLITE===
#
#   \ ===XT_S22_FVMLITE
sed 's/\\ XT_S22_FVMLITE===/XT_S22_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S22_FVMLITE/((( ===XT_S22_FVMLITE/' > 22-fvmtest.fl

# ============================================================================
#   CREATE '23-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S23_FVMLITE===
#
#   \ ===XT_S23_FVMLITE
sed 's/\\ XT_S23_FVMLITE===/XT_S23_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S23_FVMLITE/((( ===XT_S23_FVMLITE/' > 23-fvmtest.fl

# ============================================================================
#   CREATE '24-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S24_FVMLITE===
#
#   \ ===XT_S24_FVMLITE
sed 's/\\ XT_S24_FVMLITE===/XT_S24_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S24_FVMLITE/((( ===XT_S24_FVMLITE/' > 24-fvmtest.fl

# ============================================================================
#   CREATE '25-fvmtest.fl'
# ============================================================================
# Automatically activate (uncomment) code between:
#   \ XT_S25_FVMLITE===
#
#   \ ===XT_S25_FVMLITE
sed 's/\\ XT_S25_FVMLITE===/XT_S25_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/\\ ===XT_S25_FVMLITE/((( ===XT_S25_FVMLITE/' > 25-fvmtest.fl

# ============================================================================
#   CLEAN UP
# ============================================================================
# Delete intermediate file
rm xt-fvmtest.fl

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
./romMake.sh 4-fvmtest.fl && make && cp rom.fp 4-rom.fp && cp fvm 4-fvm
./romMake.sh 5-fvmtest.fl && make && cp rom.fp 5-rom.fp && cp fvm 5-fvm
./romMake.sh 6-fvmtest.fl && make && cp rom.fp 6-rom.fp && cp fvm 6-fvm
./romMake.sh 7-fvmtest.fl && make && cp rom.fp 7-rom.fp && cp fvm 7-fvm
./romMake.sh 8-fvmtest.fl && make && cp rom.fp 8-rom.fp && cp fvm 8-fvm
./romMake.sh 9-fvmtest.fl && make && cp rom.fp 9-rom.fp && cp fvm 9-fvm
./romMake.sh 10-fvmtest.fl && make && cp rom.fp 10-rom.fp && cp fvm 10-fvm
./romMake.sh 11-fvmtest.fl && make && cp rom.fp 11-rom.fp && cp fvm 11-fvm
./romMake.sh 12-fvmtest.fl && make && cp rom.fp 12-rom.fp && cp fvm 12-fvm
./romMake.sh 13-fvmtest.fl && make && cp rom.fp 13-rom.fp && cp fvm 13-fvm
./romMake.sh 14-fvmtest.fl && make && cp rom.fp 14-rom.fp && cp fvm 14-fvm
./romMake.sh 15-fvmtest.fl && make && cp rom.fp 15-rom.fp && cp fvm 15-fvm
./romMake.sh 16-fvmtest.fl && make && cp rom.fp 16-rom.fp && cp fvm 16-fvm
./romMake.sh 17-fvmtest.fl && make && cp rom.fp 17-rom.fp && cp fvm 17-fvm
./romMake.sh 18-fvmtest.fl && make && cp rom.fp 18-rom.fp && cp fvm 18-fvm
./romMake.sh 19-fvmtest.fl && make && cp rom.fp 19-rom.fp && cp fvm 19-fvm
./romMake.sh 20-fvmtest.fl && make && cp rom.fp 20-rom.fp && cp fvm 20-fvm
./romMake.sh 21-fvmtest.fl && make && cp rom.fp 21-rom.fp && cp fvm 21-fvm
./romMake.sh 22-fvmtest.fl && make && cp rom.fp 22-rom.fp && cp fvm 22-fvm
./romMake.sh 23-fvmtest.fl && make && cp rom.fp 23-rom.fp && cp fvm 23-fvm
./romMake.sh 24-fvmtest.fl && make && cp rom.fp 24-rom.fp && cp fvm 24-fvm
./romMake.sh 25-fvmtest.fl && make && cp rom.fp 25-rom.fp && cp fvm 25-fvm

