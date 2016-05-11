#
# xt.sh
# =====
# 
# A script to conveniently generate subsets of the 'fvmtest.fl' test suite
# for running on FVM Lite one at a time. This is needed because FVM Lite,
# unlike FVM Heavy, does not have enough ROM to run the whole fvmtest
# test suite at once. See 'fvmtest.fl' for details of the suite.
# 
# Copyright © Robert Gollagher 2015
# Author :    Robert Gollagher   robert.gollagher@freeputer.net
# Created:    20160511
# Updated:    20160512:0030
# Version:    pre-alpha-0.0.0.1 for FVM 1.1
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
# '1-fvmtest.fl' which represents the first subset of the test suite.
# You can then compile an FVM instance incorporating that subset by:
#
#   ./romMake.sh 1-fvmtest.fl && make
#
# And then you can run that subset of the test suite, once suitably
# colocated with the necessary files (as detailed in 'fvmtest.fl'),
# in the usual manner by:
#
#   ./fvmtest.sh
# 
# ============================================================================

# Automatically activate (uncomment) code between:
#   ((( XTFVMLITE===
#
#   ===XTFVMLITE )))
sed 's/[(][(][(] XTFVMLITE===/((( XTFVMLITE=== )))/' fvmtest.fl |
sed 's/===XTFVMLITE [)][)][)]/((( ===XTFVMLITE )))/' |

# Automatically deactivate (comment out) code between:
#   ((( XTFVMLITE---
#
#   ---XTFVMLITE )))
sed 's/\\ [(][(][(] XTFVMLITE---/((( XTFVMLITE---/' |
sed 's/\\ ---XTFVMLITE [)][)][)]/---XTFVMLITE )))/' > xt-fvmtest.fl

# Automatically activate (uncomment) code between:
#   ((( XT_S1_FVMLITE===
#
#   ===XT_S1_FVMLITE )))
sed 's/[(][(][(] XT_S1_FVMLITE===/((( XT_S1_FVMLITE=== )))/' xt-fvmtest.fl |
sed 's/===XT_S1_FVMLITE [)][)][)]/((( ===XT_S1_FVMLITE )))/' |

# Automatically deactivate (comment out) code between:
#   ((( XT_S1_FVMLITE---
#
#   ---XT_S1_FVMLITE )))
sed 's/\\ [(][(][(] XT_S1_FVMLITE---/((( XT_S1_FVMLITE---/' |
sed 's/\\ ---XT_S1_FVMLITE [)][)][)]/---XT_S1_FVMLITE )))/' > 1-fvmtest.fl

# Delete intermediate file
rm xt-fvmtest.fl
