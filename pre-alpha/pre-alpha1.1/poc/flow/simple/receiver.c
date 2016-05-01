/*
Program:    receiver.c
Copyright © Robert Gollagher 2016
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20150430
Updated:    20160501:1510

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
[at your option] any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

==============================================================================

A receiver for sender.c.

This program receives a text file one byte at a time from stdin,
using very simple software flow control.

This is a proof of concept. Since it has succeeded on Linux, the next step is
that this same behaviour will be tried in the I/O implementation of the
bare-metal FVM for U-Boot (targeting Raspberry Pi) in 'fvm.c'.

Build using:

  gcc receiver.c -o receiver

Behavour is:
  (1) send ACK to stdout;
  (2) receive 1 byte only;
  (3) repeat until EOT is received;
  (4) quit.

The following instructions assume:
  - the text to be sent by the sender is in 'textfile'
  - the text received by the receiver shall be stored to 'received'
  - optionally, logging of ACKs by tee shall be stored to 'acklog'

For debugging, this can be run with logging of ACKs using:

  mkfifo mypipe
  ./receiver received < mypipe | tee acklog | ./sender textfile > mypipe

Normally just do:

  mkfifo mypipe
  ./receiver received < mypipe | ./sender textfile > mypipe

*/

#include <stdio.h>

#define CHAR_ACK 6
#define CHAR_EOT 4

int main(int argc, char *argv[]) {

  if (argc < 2) {
    puts("Usage: receiver outfile");
    return 1;
  }

  FILE *outfile = fopen(argv[1], "w");
  if (!outfile) {
    printf("File could not be used for output: %s\n", argv[1]);
    return 1;
  }

  while (1) {
    // Let the sender know we are ready to receive a byte
    putchar(CHAR_ACK);
    fflush(stdout);
    // Wait for a byte from the sender
    char c = getchar();
    if (c != CHAR_EOT) {
      fputc(c, outfile);
      fflush(outfile);
    } else {
      break;
    }
  }

  fclose(outfile);
  return 0;
}

