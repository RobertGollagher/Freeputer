/*
Program:    sender.c
Copyright © Robert Gollagher 2016
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20160430
Updated:    20160502:1128

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

  // To ease debugging here is a second log (other than: tee acklog)
  // which contains all characters (not just ACKs) received from receiver
  FILE *login = fopen("log.in", "w");
  if (!login) {
    printf("Could not open log.in for writing");
    return 1;
  }

  // To ease debugging here is a third log (other than: tee acklog)
  // which contains all characters sent to the receiver
  FILE *logout = fopen("log.out", "w");
  if (!logout) {
    printf("Could not open log.out for writing");
    return 1;
  }

  while (1) {
    char cin = 0;
    while(cin != CHAR_ACK){
      // Log incoming characters until received an ACK
      cin = getchar();
      fputc(cin, login);
      fflush(login);
    };
    // Go ahead and send a byte of text
    char c = fgetc(text);
    if (!feof(text)) {
      putchar(c);
      fflush(stdout);
      fputc(c, logout);
      fflush(logout);
    } else {
      putchar(CHAR_EOT);
      fflush(stdout);
      fputc(CHAR_EOT, logout);
      fflush(logout);
      break;
    }
  }

// FIXME A hack: instead of quitting, just keep logging everything
// returned from the receiver. This is not needed when communicating with
// receiver.c but is helpful for testing self-hosted compiling of flc
// on Raspberry Pi bare metal. Said testing was successful.
// This now never exits, so use Ctrl-C to kill it.
fputs("\nsender.c DONE SENDING\n",login);
while(1) {
  char cin = 0;
  cin = getchar();
  fputc(cin, login);
  fflush(login);
}

  fclose(text);
  return 0;
}

