/*
Program:    sender.c
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

A sender for receiver.c.

This program sends a text file one byte at a time to stdout,
using very simple software flow control.

Build using:

  gcc sender.c -o sender

Behavour is:
  (1) wait for ACK from stdin;
  (2) send 1 byte of text to stdout;
  (3) repeat (1) and (2) until whole text sent;
  (4) send EOT;
  (5) quit.

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
    puts("Usage: sender infile");
    return 1;
  }

  FILE *text = fopen(argv[1], "r");
  if (!text) {
    printf("File not found: %s\n", argv[1]);
    return 1;
  }

  while (1) {
    while(getchar() != CHAR_ACK){};
    // Go ahead and send a byte of text
    char c = fgetc(text);
    if (!feof(text)) {
      putchar(c);
      fflush(stdout);
    } else {
      putchar(CHAR_EOT);
      fflush(stdout);
      break;
    }
  }

  fclose(text);
  return 0;
}

