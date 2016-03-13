/*

Program:    tape.c
Copyright © Robert Gollagher 2016
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    2016
Updated:    20160313:2115
Version:    pre-alpha-0.0.0.1

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

WARNING: This is pre-alpha software and as such may well be incomplete,
unstable and unreliable. It is considered to be suitable only for
experimentation and nothing more.

==============================================================================

The tape is a textual user interface (TUI) as described in tape/README.md.

This implementation:

  * uses ncurses
  * is not portable, runs on Linux only
  * uses Linux system calls such as poll, open, close
  * supports physical serial devices (such as from an FVM on Arduino or chipKIT)
  * supports pseudo terminals (to connect to a local FVM instance on Linux)
  * supports UTF-8 display (if run within a UTF-8 terminal emulator)
  * supports multiplexing (an optional feature of FVM 1.1)
  * supports split-tape tracing (stdtrc display in second half of tape)

On Debian you will need these libraries installed:

  aptitude install libncursesw5 libncursesw5-dev

To build use:

  gcc -o tape tape.c -lncursesw

To connect to a locally running Freeputer instance, assuming that
socat reports it has created /dev/pts/0 and that your FVM executable
is located in the current directory and called fvm:

  socat -d -d pty,raw,echo=0 "exec:./fvm ...,pty,raw,echo=0"

  ./tape /dev/pts/0

To connect to a physical Freeputer instance (such as on Arduino or chipKIT)
attached to physical serial device such as /dev/ttyUSB0 you must first ensure
that Linux knows the configuration of the serial device. That can be achieved
either by connecting to /dev/ttyUSB0 at least once with the serial monitor
in the Arduino IDE set to 115200 baud or by briefly connecting at least
once with minicom thus:

  minicom -D /dev/ttyUSB0 -c on -b 115200

Thereafter you can simply do:

  ./tape /dev/ttyUSB0

For the tape to behave meaningfully, there must be an application running
on the FVM instance which the aforementioned device (such as /dev/pts/0 or
/dev/ttyUSB0) is connected to, and that application must be written
to control a tape. A simple example of such a Freeputer application is
the Freelang program 'ts.fl' (short for Tape Server) which selectively and
appropriately echos characters typed (or pasted) into the tape so
that they appear on the tape while respecting escape sequences
(which for a tape, unlike an ANSI terminal, begin with the DLE character
not the ESC character). Actually the application controlling the tape
via the aforementioned device does not necessary have to be a Freeputer
application; it could be any kind of application that appropriately
handles its stdin and stdout streams so as to control a tape.

This 'tape.c' implementation of the tape happens to communicate via
a serial device and accepts keyboard input; therefore it can be properly
described as a 'tape terminal'. A tape does not necessarily have to use
serial communication and does not necessarily have to accept input
(some tapes might be only for display not input). However, the most
common kind of tape implementation is a tape terminal since such
an implementation has many uses and favours modular rather than
monolithic system design (rather than building a display
into a standalone Freeputer instance).

Since this implementation uses ncurses and runs within an ANSI terminal
emulator, the tape is wrapped into a 2D geometry and therefore this
implementation tracks things such as row and column. It is important to
remember that the tape itself has no concept of multi-dimensional geometry,
it is just a sequence of cells. All matters of geometry are private to
and internal to an implementation and have nothing to do with the
logical nature of the tape. On a practical note: you should not resize
the terminal emulator window within which the tape is running while
the tape is running. Only resize the terminal emulator window
prior to starting the tape within it; with that proviso you can
run the tape in almost any size of terminal emulator window,
from the very small (say 16x1) to huge (full screen).

In the code comments below you will see the word 'server' being used.
This simply refers to the application (typically but not necessarily a
Freeputer application, such as the tape server 'ts.fl') controlling the tape
via the serial device. You will note that 'tape.c' has to keep track
of which characters are coming from local keyboard input and which
characters are coming from the server; both arrive via poll. It is worth
noting that a 'tape terminal' implementation on bare metal (such as an Arduino
wired up to a PS/2 keyboard and an alphanumeric LCD display) is actually
much simpler than this Linux implementation because such complexities
as poll (and, indeed, ncurses) are then not needed. It is often the
case that using bare metal is simpler and easier for simple tasks
than using complex operating systems such as Linux.

Furthermore you will note there is an option to use multiplexing.
In this context, multiplexing refers to a new optional feature in some
FVM 1.1 implementations that uses a simple multiplexing scheme to allow
a single physical serial connection to be used to support not only
the Freeputer stdin and stdout streams but also stdtrc (the Freeputer
output stream on which tracing output can be sent from an FVM instance
as an aid to debugging a Freeputer application). In this simple
multiplexing scheme each data byte is preceded by a byte identifying
the stream to which that data byte belongs. This 'tape.c' supports that
multiplexing and indeed you must have it enabled if you wish to see
stdtrc output in the second half of the tape when running in
split-tape mode (entered by pressing Ctrl-U). Since such multiplexing is
a new feature coming in FVM 1.1, you should ensrue MULTIPLEX is undefined
here in 'tape.c' for now; unfortunately this means you will never see
any stdtrc output in split-tape mode. Note that the purpose of the new
multiplexing scheme (the use of which is entirely optional) is to make it
easier to run Freeputer applications on Arduino and other development boards
without requiring multiple physical serial connections.

If the tape appears to be unresponsive and displays a red exclamation mark
this means an error has occurred; this will occur if you leave MULTIPLEX
defined yet the FVM you are connected to is not using multiplexing,
or if for any other reason the tape is sent garbage input that
cannot be meaningfully decoded.

If implementing your own tape on some other platform, it is important
to understand that the fields of the tape struct in this 'tape.c'
are particular to this implementation. The only fields of that
struct that belong to the logical nature of a tape are pos
(the current cell), maxPos (the length of the tape),
and arguably maybe some of the mode information.

Note on ncurses: although ncurses supports the use of mutliple windows,
they were intentionally not used to implement split-tape mode because
bare-metal implementations of the tape will not have that luxury
available (and indeed will not use ANSI terminals at all).
The lessons learned here will directly translate to
forthcoming bare-metal tape implementations.

Important: stdtrc output generally does not support UTF-8.
Only send 7-bit ASCII characters to stdtrc.

==============================================================================

  For reference: FVM 1.1 SIMPLE MULTIPLEXING

  FVM 1.1 may optionally use multiplexing to allow multiple virtual devices
  to share a single physical serial communication link. The simple multiplexing
  is that each data packet is preceded by a routing byte in which the
  most significant bit (bit 7) is always zero, the next two
  most significant bits (bits 6 and 5) identify the device type and
  the next bit (bit 4) identifies the packet size (essentially byte or word) and
  the four least significant bits (bit 3 to 0) identify the absolute value
  of the device ID (e.g. an ID of -1 is communicated as an ID of 1).
  Note that, by Freeputer convention, a device ID of 0 is only used for stdtrc
  and stdblk; an input stream never has a device ID of 0; and output streams
  have negative IDs. The crude multiplexing scheme used here only supports
  device IDs whose absolute value is between 0 and 15 inclusive.

  Most significant bits:

    0b0100 = device: output stream, packet size: 1 byte (byte)
    0b0101 = device: output stream, packet size: 4 bytes (word)
    0b0000 = device: input stream, packet size: 1 byte (byte)
    0b0001 = device: input stream, packet size: 4 bytes (word)
    0b0010 = device: block device, packet size: 5 bytes (word addr then byte)
    0b0011 = device: block device, packet size: 8 bytes (word addr then word)
    0b0110 = reserved (not used)
    0b0111 = reserved (not used) e.g. 0b01111111 unknown device
    0b1--- = reserved (not used)

  Thus byte communication uses:

    0b01000000 = stdtrc 0
    0b00000001 = stdin 1
    0b00000002 = stdimp 2
    0b00100000 = stdblk 0
    0b01000001 = stdout -1
    0b01000010 = stdexp -2
    0b0110----- = reserved (not used)

  Thus word communication uses:

    0b01010000 = stdtrc 0
    0b00010001 = stdin 1
    0b00010002 = stdimp 2
    0b00110000 = stdblk 0
    0b01010001 = stdout -1
    0b01010010 = stdexp -2
    0b0111----- = reserved (not used)

============================================================================== */

#include <curses.h>
#include <fcntl.h>
#include <locale.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#define CHAR_TYPE unsigned char // Don't change this (other types unsupported)
#define TAPE_SPLIT_TRC // Uncomment to support split-tape mode (stdtrc display)
#define SUPPORT_UTF8 // Uncomment to support UTF-8 characters
#define TAPE_DEBUG // Uncomment to debug the tape itself
#ifdef TAPE_DEBUG
  FILE *trc; // File for debugging output from the tape itself
  FILE *fvmtrc; // File for stdtrc output from the connected FVM instance
#endif
#define MULTIPLEX // Uncomment to support FVM 1.1 multiplexing
#ifdef MULTIPLEX  //   (note: an FVM 1.1 may or may not support mutliplexing)
  #define STDIN_BYTE  0b00000001
  #define STDOUT_BYTE 0b01000001
  #define STDTRC_BYTE 0b01000000
  #define UNKNOWN_DEVICE_BYTE 0b01111111
  CHAR_TYPE multiRouteIn;
  int multiRouteInKnown;
#endif

#define DLE_LENGTH 6
#define SERVER_DLE_LEN 6
#define KEY_DLE_LEN 6

typedef struct sequence {
  CHAR_TYPE index;
  CHAR_TYPE seq[SERVER_DLE_LEN];
} sequence_t;
typedef struct tape {
  int pos; // Current cell (starts at 0)
  int maxPos; // Highest cell (not including split-tape mode stdtrc display)
  int lastPos; // Most recent cell
  int col; // Currrent column
  int row; // Current row
  int maxX; // Number of columns
  int maxY; // Number of rows
  bool wrap; // In wrap mode? (wrap = overwrite; no wrap = stops writing)
  bool over; // When in wrap mode, have we wrapped (that is, overwritten)?
  sequence_t keyseq; // Current escape sequence (from local keyboard input)
  sequence_t dleseq; // Current escape sequence (from server)
  bool serverReady; // Is the server ready to receive another character?
  char utf8buf[5]; // Zero-terminated buffer for input of a UTF-8 character
  int iUtf8buf; // Current index in the utf8buf (0 to 3)
  int lenUtf8char; // Byte length of current UTF-8 character (1 to 4)
  bool splitMode; // In split-tape mode? (allows stdtrc display)
  int maxPos2; // Highest stdtrc cell (only used in split-tape mode)
  int pos2; // Current stdtrc cell (only used in split-tape mode)
  bool visibleSpc; // Show spaces as visible characters?
} tape_t;

bool tape_run(tape_t *t, const CHAR_TYPE *deviceName);
bool tape_init(tape_t *t, const CHAR_TYPE *deviceName);
bool tape_putc(tape_t *t, CHAR_TYPE c);
bool tape_goto(tape_t *t, int pos);
bool tape_clear(tape_t *t);
bool tape_clearFrom(tape_t *t);
bool tape_shutdown(tape_t *t);
static void quit(int sig);
int ttrace(tape_t *t, CHAR_TYPE c);
void clearKeyseq(tape_t *t);
void handleOver(tape_t *t);
void notOver(tape_t *t);

#define NUM_DLE_SIMPLE_INSTRS 12
#define NUM_DLE_COMPLEX_INSTRS 1

#define CMD_DLE_CLEAR 1
#define CMD_DLE_CLEAR_FROM 2
#define CMD_DLE_NO_WRAP 3
#define CMD_DLE_WRAP 4
#define CMD_DLE_RIGHT 5
#define CMD_DLE_LEFT 6
#define CMD_DLE_UP 7
#define CMD_DLE_DOWN 8
#define CMD_DLE_REPORT_POS 9
#define CMD_DLE_REPORT_MAX_POS 10
#define CMD_DLE_SPLIT_MODE 11
#define CMD_DLE_VISIBLE_SPC 12
#define CMD_DLE_GOTO_POS 128

#define CHAR_HT 9 // Horizontal tab (an invisible character)
#define CHAR_NL 10 // Newline (an invisible character)
#define CHAR_VT 11 // Ctrl-k : Clear from cursor to end of tape
#define CHAR_FF 12 // Ctrl-l : Clear entire tape
#define CHAR_CR 13 // Return (an invisible character)
#define CHAR_DC2 18 // Ctrl-r : Turn off wrap mode
#define CHAR_DC4 20 // Ctrl-t : Toggle split-tape mode (stdtrc display)
#define CHAR_ETB 23 // Ctrl-w : Turn on wrap mode
#define CHAR_EM 25 // End of medium (indicates tape full when not in wrap mode)
#define CHAR_ESC 27 // Escape (not used by the tape, it uses DLE instead)
#define CHAR_FS 28 // Right arrow
#define CHAR_GS 29 // Left arrow
#define CHAR_RS 30 // Up arrow
#define CHAR_US 31 // Down arrow
#define CHAR_SPC 32 // Space
#define CHAR_VISIBLE_SPC '.' // Character to show to make spaces visible
#define CHAR_INVIS '*' // Character to show for invisible characters (not space)

#define CHAR_DLE 16 // Data Link Escape character (used for escape sequences)
#define CHAR_XON 17 // = DC1 (used by server to indicate ready)
#define CHAR_XOFF 19 // = DC3 (used by server to indicate not ready)

#define DISPLAY_COLOR true // Set this to true for colour output
#if DISPLAY_COLOR == true
  #define COLOR_BG_BLACK 8
  #define COLOR_BG_GREEN 9
  #define COLOR_BG_RED 10
  #define COLOR_BG_CYAN 11
  #define COLOR_BG_WHITE 12
  #define COLOR_BG_MAGENTA 13
  #define COLOR_BG_BLUE 14
  #define COLOR_BG_YELLOW 15
  #define COLOR_OK COLOR_BG_GREEN
  #define COLOR_OVER COLOR_BG_YELLOW
  #define COLOR_WRAP COLOR_BG_YELLOW
  #define COLOR_INVIS COLOR_BG_YELLOW
  #define COLOR_ILLEGAL_UTF8 COLOR_BG_RED
  #define COLOR_UNSUPPORTED_UFT8 COLOR_BG_CYAN
  #define COLOR_HT COLOR_BG_MAGENTA
  #define COLOR_NL COLOR_BG_BLUE
  #define COLOR_CR COLOR_BG_GREEN
  #define COLOR_ERR COLOR_BG_RED
  #define COLOR_VIS_SPACE COLOR_MAGENTA
#endif

#define FILE_ID_TYPE int
FILE_ID_TYPE server;
struct pollfd pfds[2];

typedef enum { ORIGIN_LOCAL, ORIGIN_SERVER } origin;
typedef struct {
  origin o;
  CHAR_TYPE c; 
} ochar;

/*
  Logic to decode escape sequences generated by ncurses when
  various keys are pressed. This only handles a small subset of
  keys we happen to care about and while not particularly
  robust does the job well enough for intial experimentation
  with the tape as a user interface. In the fullness of
  time this logic could be expanded to handle more
  keys and to do more robust error handling of
  nonsensical sequences.

  Note: whether or not the recognized escape sequences here
  (which all originate from ncurses when keys are pressed)
  actually cause anything to happen is up to the server
  not the tape itself. The server may decide, for example,
  to refuse to turn off wrap mode despite the user
  having pressed Ctrl-R. The server (that is, the application
  typically running on a Freeputer instance at the other end of
  the serial device this tape talks to) decides how the
  tape shall behave; it does this by selectively and
  appropriately deciding which escape sequences it
  will and will not echo back to the tape. Indeed the
  same applies for ordinary characters; the server can choose
  to echo them back to the tape or not to do so,
  as it decides. The server is the controller.
*/
void filterKeyseq(tape_t *t, CHAR_TYPE c) {
  if (t->keyseq.seq[0] == CHAR_ESC) {
    // Already in an esc seq
    switch(t->keyseq.index) {    
      case(1):
        switch(c) {
        case('O'):
        case(013): // Ctrl-K : clear to end of tape
        case(014): // Ctrl-L : clear whole tape
        case(022): // Ctrl-R : turn off wrap mode
        case(024): // Ctrl-T : toggle visible spaces
        case(025): // Ctrl-U : toggle split-tape mode (for stdtrc output)
        case(027): // Ctrl-W : turn on wrap mode
          t->keyseq.seq[t->keyseq.index++] = c;
        break;
        default:
          clearKeyseq(t);
          #ifdef TAPE_DEBUG
            fprintf(trc,"bad key esc seq at 1\n");
            fflush(trc);
          #endif 
          break;
        }
      break;
      case(2):
        switch(c) {
        case('A'): // up arrow
        case('B'): // down arrow
        case('C'): // right arrow
        case('D'): // left arrow
          t->keyseq.seq[t->keyseq.index++] = c;
        break;
        default:
          clearKeyseq(t);
          #ifdef TAPE_DEBUG
            fprintf(trc,"bad key esc seq at 2\n");
            fflush(trc);
          #endif 
          break;
        }
      break;
      default:
        clearKeyseq(t);
        #ifdef TAPE_DEBUG
          fprintf(trc,"bad key esc seq at 3+\n");
          fflush(trc);
        #endif 
        break;
    }
  } else {
    // Not already in an esc seq
      if (t->keyseq.index == 0) {
        if (c == CHAR_ESC) {
        // Starting a new esc seq from a key esc seq (e.g. left arrow)
          t->keyseq.seq[t->keyseq.index++] = CHAR_ESC;
          #ifdef TAPE_DEBUG
            fprintf(trc,"started new key esc seq\n");
            fflush(trc);
          #endif 
        } else if (c == 013 // Ctrl-K
                || c == 014 // Ctrl-L
                || c == 022 // Ctrl-R
                || c == 024 // Ctrl-T
                || c == 025 // Ctrl-U
                || c == 027 ) { // Ctrl-W 
        // Starting a new esc seq from a key control char (e.g. Ctrl-L)
          t->keyseq.seq[t->keyseq.index++] = CHAR_ESC;
          t->keyseq.seq[t->keyseq.index++] = c;
          #ifdef TAPE_DEBUG
            fprintf(trc,"started new key esc seq, control char: %d\n",c);
            fflush(trc);
          #endif 
        }
      } else {
        // Normal input (not part of an esc seq)
        #ifdef TAPE_DEBUG
            fprintf(trc,"normal, not a key esc seq\n");
            fflush(trc);
          #endif 
      }
  }
}

/*
  Decode escape sequences (note: these start with DLE not ESC)
  being received from the server. These are essentially
  commands from the server to the tape. This logic is good enough
  to experiment with the tape as a user interface but is not
  particularly robust with respect to the error handling
  of nonsensical sequences. In future this logic could
  be expanded so as to improve error handling.
*/
void filterDleseq(tape_t *t, CHAR_TYPE c) {
  if (t->dleseq.seq[0] == CHAR_DLE) {
    // Already in an esc seq
    switch(t->dleseq.index) {    
      case(1):
        if ((c > 0 && c <= NUM_DLE_SIMPLE_INSTRS) ||
            (c > 127 && c<= NUM_DLE_COMPLEX_INSTRS+127)) {
          t->dleseq.seq[t->dleseq.index++] = c;
        } else {
          #ifdef TAPE_DEBUG
            fprintf(trc,"bad dle esc seq at 1 (unknown instruction)\n");
            fflush(trc);
          #endif 
        }
        break;
      case(2):
      case(3):
      case(4):
      case(5):
          t->dleseq.seq[t->dleseq.index++] = c;
        break;
      default:
        #ifdef TAPE_DEBUG
          fprintf(trc,"bad dle esc seq, too long (sextet maximum)\n");
          fflush(trc);
        #endif 
      break;
    }
  } else {
    // Not already in an esc seq
      if (c == CHAR_DLE && t->dleseq.index == 0) {
        // Starting a new esc seq        
          t->dleseq.seq[t->dleseq.index++] = c;
        #ifdef TAPE_DEBUG
          fprintf(trc,"started new dle esc seq\n");
          fflush(trc);
        #endif 
      } else {
        // Normal input (not part of an esc seq)
        #ifdef TAPE_DEBUG
          fprintf(trc,"normal, not a dle esc seq\n");
          fflush(trc);
        #endif 
      }
  }
}

/*
  To get correct behaviour of the interaction between server and tape,
  it is sometimes necessary to wait before sending a character to the server.
  This is important whenever the server is sending some multi-byte sequence,
  most notably escape sequences (commands, see filterDleseq) but also
  presumably multi-byte UTF-8 characters. Otherwise occasionally
  you will see errors/failure when, for example, deliberately hammering
  away at the keyboard (including key combinations which themselves
  generate escape sequences) while a large amount of text is
  being communicated from the server to the tape. The strategy of
  using this setServerReady function generally seems to be working to
  prevent these problems but it is not clear whether or not
  the strategy is fully effective (concurrent sending and receiving
  of multi-byte UTF-8 characters probably needs more investigation).
  However, typical use cases seem to be working well.
*/
void setServerReady(tape_t* t, bool boolValue) {
    t->serverReady = boolValue;
    #ifdef TAPE_DEBUG
      fprintf(trc,"SETTING t->serverReady to: %d\n",boolValue);
      fflush(trc);
    #endif 
}

/*
  This input function uses poll to wait for input coming from
  either the local keyboard (or pasted into the tape) or the server.
  Each received character is put in an ochar struct which is
  tagged with either ORIGIN_LOCAL or ORIGIN_SERVER so
  we know where the character came from.
*/
ochar input(tape_t *t) {

  CHAR_TYPE c;
  ochar oc;
  CHAR_TYPE buf[1];
  int countRead;

    while(1) {
      pfds[0].fd = 0;
      pfds[0].events = POLLIN;
      pfds[1].fd = server;
      pfds[1].events = POLLIN;
      poll(pfds, 2, -1);

      if ((pfds[0].revents & POLLIN) && t->serverReady) {
          countRead = read(0, buf, 1);
          if (!countRead) {
              quit(0);
          }
          c = buf[0];
          oc.o = ORIGIN_LOCAL;
          oc.c = c;
          filterKeyseq(t,c);
          #ifdef TAPE_DEBUG
            fprintf(trc,"origin local:%d '%c' keyseq: %s\n ",c,c,t->keyseq.seq);
            fflush(trc);
          #endif  
          break;
       }

       if (pfds[1].revents & POLLIN) {
          countRead = read(server, buf, 1);
          if (!countRead) {
              quit(0);
          }
          c = buf[0];
          oc.o = ORIGIN_SERVER;
          oc.c = c;
          #ifdef TAPE_DEBUG
            fprintf(trc,"origin server:%d '%c'\n ",c,c);
            fflush(trc);
          #endif
          break;
       }
    }
    return oc;
}

/*
  Display a message on the tape. By convention (since some tapes may be
  extremely short, only a few characters long), a tape 'message' is always
  only 1 character long and is always displayed in the last normal cell
  of the tape (that is, at maxPos; that is, prior to any stdtrc output
  should the tape happen to be in split-tape mode at the time).

  This allows the user to glance at the last normal cell of the tape
  and to easily see (depending on what character is displayed there,
  often with the help of colour-highlighting) whether all is well,
  whether an error has occurred, whether text has wrapped and
  therefore some text has been overwritten, or whether text
  cannot be displayed because the tape is full and
  not in wrap mode. See msgOK, msgOver, msgWrapped, msgErr.
*/
void msg(tape_t *t, CHAR_TYPE c) {
  tape_goto(t, t->maxPos);
  addch(c); // ncurses: add character to screen
  tape_goto(t, t->lastPos);
  refresh(); // ncurses: refresh screen
}

#if DISPLAY_COLOR == true
void msgColor(tape_t *t, CHAR_TYPE c, int colorPairId) {
  attron(COLOR_PAIR(colorPairId)); // ncurses: attribute on
  msg(t,c);
  attroff(COLOR_PAIR(colorPairId)); // ncurses: attribute off
}

bool tape_putc_color(tape_t *t, CHAR_TYPE c, int colorPairId);
#endif

void msgOK(tape_t *t) {
  #if DISPLAY_COLOR == true
    msgColor(t, '#', COLOR_OK);
  #else
    msg(t, '#');
  #endif
}

void msgOver(tape_t *t) {
  #if DISPLAY_COLOR == true
    msgColor(t, '+', COLOR_OVER);
  #else
    msg(t, '+');
  #endif
}

void msgWrapped(tape_t *t) {
  #if DISPLAY_COLOR == true
    msgColor(t, '&', COLOR_WRAP);
  #else
    msg(t, '&');
  #endif
}

void msgErr(tape_t *t) {
  #if DISPLAY_COLOR == true
    msgColor(t, '!', COLOR_ERR);
  #else
    msg(t, '!');
  #endif
}

void clearUtf8buf(tape_t *t) {
  for (int i=0; i++; i<6) {t->utf8buf[i] = 0;}
  t->iUtf8buf = 0;
  t->lenUtf8char = 0;
}

int showUtfchar(tape_t *t, CHAR_TYPE c) {
#ifdef SUPPORT_UTF8
  if (addstr(t->utf8buf) == ERR) {
    if (!t->wrap) {
      handleOver(t);
    }
  } else {
    getyx(stdscr, t->row, t->col); // ncurses: get cursor location
    t->pos = (t->row*t->maxX) + t->col; 
  }
#else
  #if DISPLAY_COLOR == true
    tape_putc_color(t, '~', COLOR_UNSUPPORTED_UFT8);
  #else
    tape_putc(t, '~');
  #endif
#endif
}

/*
  Show a UTF-8 character. This simple implementation is sufficient for
  most purposes but doesn't do robust checking for invalid UTF-8 characters
  and is incapable of recognizing combining characters such as ɔ̃
  as a single character (you will notice such combining characters
  end up getting displayed as 2 unknown characters).

  Note: this will only show a character on the normal part of the tape,
  no higher than maxPos. It will not show a character on the stdtrc
  part of the tape in split-tape mode.
*/
void utf(tape_t *t, CHAR_TYPE c) {
  #ifdef TAPE_DEBUG
    fprintf(trc,"utf byte:%d\n ",c);
    fflush(trc);
  #endif

  t->utf8buf[t->iUtf8buf] = c; 
  if (t->iUtf8buf == 0) {
    if ((c & 0b11100000) == 0b11000000) {
      t->lenUtf8char = 2;
    } else if ((c & 0b11110000) == 0b11100000) {
      t->lenUtf8char = 3;
    } else if ((c & 0b11111000) == 0b11110000) {
      t->lenUtf8char = 4;
    } else {
      // invalid char
      #if DISPLAY_COLOR == true
        tape_putc_color(t, '?', COLOR_ILLEGAL_UTF8);
      #else
        tape_putc(t, '?');
      #endif
      clearUtf8buf(t);
      return;
    }
  } else if (t->iUtf8buf == t->lenUtf8char - 1 ) {
    showUtfchar(t,c);
    clearUtf8buf(t);
    return;
  }
  ++(t->iUtf8buf);
}

/*
  Show an ordinary visible character.

  Note: this will only show a character on the normal part of the tape,
  no higher than maxPos. It will not show a character on the stdtrc
  part of the tape in split-tape mode.
*/
int vis(tape_t *t, CHAR_TYPE c) {
  if (c == CHAR_SPC && t->visibleSpc) {
    #if DISPLAY_COLOR == true
      tape_putc_color(t, CHAR_VISIBLE_SPC, COLOR_VIS_SPACE);
    #else
      tape_putc(t, CHAR_VISIBLE_SPC);
    #endif
  } else {
    tape_putc(t,c);
  }
}

/*
  Show an invisible character by subsituting it with the appropriate
  visible substitute. By convention the only times a cell is allowed to
  appear empty is: (1) if it contains a space; (2) if it is empty.

  Note: in future the tape conventions might change so as
  a cell containing a space must if posible always appear visually
  different to an empty cell but for now it is not yet clear
  whether such a rule is necessary. For now there is at least
  Ctrl-T which turns on visible spaces.

  Note: this will only show a character on the normal part of the tape,
  no higher than maxPos. It will not show a character on the stdtrc
  part of the tape in split-tape mode.
*/
int invis(tape_t *t, CHAR_TYPE c) { // é
  switch(c) {
    case(CHAR_HT):
      #if DISPLAY_COLOR == true
        tape_putc_color(t, '*', COLOR_HT);
      #else
        tape_putc(t, '*');
      #endif
      break;
    case(CHAR_NL):
      #if DISPLAY_COLOR == true
        tape_putc_color(t, '*', COLOR_NL);
      #else
        tape_putc(t, '*');
      #endif
      break;
    case(CHAR_CR):
      #if DISPLAY_COLOR == true
        tape_putc_color(t, '*', COLOR_CR);
      #else
        tape_putc(t, '*');
      #endif
      break;
    default:
      #if DISPLAY_COLOR == true
        tape_putc_color(t, '?', COLOR_INVIS);
      #else
        tape_putc(t, '?');
      #endif
      break;
  }
}

/*
  Show a character on the tape.

  Note: this will only show a character on the normal part of the tape,
  no higher than maxPos. It will not show a character on the stdtrc
  part of the tape in split-tape mode.
*/
void show(tape_t *t, CHAR_TYPE c) {
  #ifdef TAPE_DEBUG
    fprintf(trc,"show:%d t->row %d t->col %d t->pos %d \n ",
      c,t->row,t->col,t->pos);
    fprintf(trc,"and multi:%d '%c'\n",c,c);
    fflush(trc);
  #endif
  if (c > 127) { // WARNING: this test only works for CHAR_TYPE of unsigned char
    utf(t, c);
  } else if (c < CHAR_SPC) {
    invis(t, c);
  } else {
    vis(t, c);
  }
  refresh();
}

#ifdef TAPE_SPLIT_TRC
  void splitModeOn(tape_t *t) {
    t->splitMode = true;
    getmaxyx(stdscr, t->maxY, t->maxX); // ncurses: get screen size
    t->maxPos2 = (t->maxX * t->maxY) - 1;
    t->maxPos = t->maxPos2 / 2; // Leave room for stdtrc display
    if (t->maxPos % t->maxX != t->maxX - 1) {
      if (t->maxY > 1) {
        // Avoid split mid-way through row
        t->maxPos = t->maxPos - (t->maxPos % t->maxX) - 1;
      }
    }    
    t->col = 0;
    t->row = 0;
    t->lastPos = 0;
    t->pos = 0;
    t->pos2 = t->maxPos + 1;
    tape_clear(t);
    refresh(); // ncurses: refresh
  }

  void splitModeOff(tape_t *t) {
    t->splitMode = false;
    getmaxyx(stdscr, t->maxY, t->maxX); // ncurses: get screen size
    t->maxPos = (t->maxX * t->maxY) - 1;
    t->col = 0;
    t->row = 0;
    t->lastPos = 0;
    t->pos = 0;
    tape_clear(t);
    refresh(); // ncurses: refresh
  }
#endif

void visSpcOn(tape_t *t) {
  t->visibleSpc = true;
  tape_clear(t);
  refresh();  // ncurses: refresh
}

void visSpcOff(tape_t *t) {
  t->visibleSpc = false;
  tape_clear(t);
  refresh();  // ncurses: refresh
}

/*
  Initialize the tape. Quite a lot is done in this function, including
  hooking the tape up appropriately to the serial device, the name
  of which must be passed as the first argument on the command
  line when running the tape, such as:

    ./tape /dev/pts/0

*/
bool tape_init(tape_t *t, const CHAR_TYPE *deviceName) {

  t->wrap = true;
  t->serverReady = false;

  setlocale(LC_ALL,""); // ncurses: needed for UTF-8 support

  #ifdef TAPE_DEBUG
    trc = fopen("trc", "w");
    fvmtrc = fopen("fvmtrc", "w");
  #endif

  #ifdef MULTIPLEX
    multiRouteIn = UNKNOWN_DEVICE_BYTE;
    multiRouteInKnown = FALSE;
  #endif
  struct stat sb;
  if (stat(deviceName,&sb) == -1) {
    printf("Server unreachable at: %s\n", deviceName);
    return false;
  }
  server = open(deviceName, O_RDWR);
  if (!server) { // apparently this never fails here
    printf("Server unreachable at: %s\n", deviceName);
    return false;
  }

  signal(SIGINT, quit); // ncurses: respond to Ctrl-C by closing application
  initscr(); // ncurses: initialization
  cbreak(); // ncurses input: do not wait for Enter key
  noecho(); // ncurses input: no local echo
  nonl(); // ncurses output: do not translate \n into \r\n
  nodelay(stdscr, true); // ncurses input: getch will be non-blocking
  keypad(stdscr, TRUE); // ncurses input: special values for key presses
  #if DISPLAY_COLOR == true
    #define COLOR_DEFAULT -1
    if (has_colors()) { // ncurses: can the display manipulate colors?
      start_color(); // ncurses: initialize color support
      use_default_colors(); // ncurses: support use of COLOR_DEFAULT constant

      // Set up convenient color pairs for subsequent use
      //   (not all of these are actually used, and more combinations
      //      are possible than those defined here)
      init_pair(COLOR_BG_BLACK, COLOR_DEFAULT, COLOR_BLACK);
      init_pair(COLOR_BG_RED, COLOR_DEFAULT, COLOR_RED);
      init_pair(COLOR_BG_GREEN, COLOR_DEFAULT, COLOR_GREEN);
      init_pair(COLOR_BG_YELLOW, COLOR_DEFAULT, COLOR_YELLOW);
      init_pair(COLOR_BG_BLUE, COLOR_DEFAULT, COLOR_BLUE);
      init_pair(COLOR_BG_MAGENTA, COLOR_DEFAULT, COLOR_MAGENTA);
      init_pair(COLOR_BG_CYAN, COLOR_DEFAULT, COLOR_CYAN);
      init_pair(COLOR_BG_WHITE, COLOR_DEFAULT, COLOR_WHITE);

      init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_DEFAULT);
      init_pair(COLOR_RED, COLOR_RED, COLOR_DEFAULT);
      init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_DEFAULT);
      init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_DEFAULT);
      init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_DEFAULT);
      init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_DEFAULT);
      init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_DEFAULT);
      init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_DEFAULT);
    }
  #endif

  #ifdef TAPE_SPLIT_TRC
    splitModeOff(t);
  #else
    clear(); // ncurses: clear screen
    refresh(); // ncurses: update view to match model (stdscr)
    getmaxyx(stdscr, t->maxY, t->maxX); // ncurses: get screen size
    t->maxPos = (t->maxX * t->maxY) - 1;
    t->col = 0;
    t->row = 0;
    t->lastPos = 0;
    t->pos = 0;
  #endif
  msgOK(t);

  return true;
}

void complexDle(tape_t *t, CHAR_TYPE dleCmd, int val);

/*
  Respond to a command from the server. That command, given by escape
  sequence (starting with DLE) must already have been fully received by
  the tape. Only then should this handleServerEscseq function be
  be called to carry out the action the server has commanded (such as
  clearing the tape or entering wrap mode).

  Like other functions in this tape implementation which deal with
  escape sequences, the error handling here is not particularly
  robust and could be expanded in future. But for now this
  works well under normal usage conditions with a
  sever that is properly implemented.
*/
void handleServerEscseq(tape_t *t) {
  #ifdef TAPE_DEBUG
    fprintf(trc,"handleServerEscseq:%d %d %d %d %d %d\n",
      t->dleseq.seq[0],t->dleseq.seq[1],t->dleseq.seq[2],t->dleseq.seq[3],
      t->dleseq.seq[4],t->dleseq.seq[5]);
    fflush(trc);
  #endif 
  switch(t->dleseq.seq[1]) {
    case(CMD_DLE_CLEAR):
        tape_clear(t);
      break;
    case(CMD_DLE_CLEAR_FROM):
      tape_clearFrom(t);
      break;
    case(CMD_DLE_NO_WRAP):
      t->wrap = false;
      break;
    case(CMD_DLE_SPLIT_MODE):
      #ifdef TAPE_SPLIT_TRC
        if (t->splitMode == true) {
          splitModeOff(t);
        } else {
          splitModeOn(t);
        }
      #endif
      break;
    case(CMD_DLE_VISIBLE_SPC):
      if (t->visibleSpc == true) {
        visSpcOff(t);
      } else {
        visSpcOn(t);
      }
      break;
    case(CMD_DLE_WRAP):
      t->wrap = true;
      break;
    case(CMD_DLE_RIGHT):
      if (t->pos+1 > t->maxPos) {
        tape_goto(t,0);
      } else {
        tape_goto(t,t->pos+1);
      }
      break;
    case(CMD_DLE_LEFT):
      if (t->pos-1 < 0) {
        tape_goto(t,t->maxPos);
      } else {
        tape_goto(t,t->pos-1);
      }
      break;
    case(CMD_DLE_UP):
      if (t->pos - t->maxX < 0) { 
        tape_goto(t,0);
      } else {
        tape_goto(t,t->pos - t->maxX);
      }
      break;
    case(CMD_DLE_DOWN):
      if (t->pos + t->maxX > t->maxPos) { 
        tape_goto(t,t->maxPos);
      } else {
        tape_goto(t,t->pos + t->maxX);
      }
      break;
    case(CMD_DLE_REPORT_POS):
      complexDle(t, t->dleseq.seq[1], t->pos);
      break;
    case(CMD_DLE_REPORT_MAX_POS):
      complexDle(t, t->dleseq.seq[1], t->maxPos);
      break;
    case(CMD_DLE_GOTO_POS):
      tape_goto(t,*(int *)&(t->dleseq.seq[2]));
      break;
    default:
      #ifdef TAPE_DEBUG
        fprintf(trc,"bad command in dle esc seq\n");
        fflush(trc);
      #endif 
      break;
  }
}

void clearEscseq(tape_t *t) {
  for (int i=0; i<SERVER_DLE_LEN; i++) {
    t->dleseq.seq[i] = 0;
  }
  t->dleseq.index = 0; 
}

void clearKeyseq(tape_t *t) {
  for (int i=0; i<KEY_DLE_LEN; i++) {
    t->keyseq.seq[i] = 0;
  }
  t->keyseq.index = 0; 
}

/*
  Take appropriate action given that the specified character c has
  been received from the server. This handleServerChar delegates to
  various other functions to ensure correct behaviour. This is moderately
  complex as it must take into account multiplexing (if enabled),
  lead to appropriate character display or lead to appropriate
  escape-sequence processing. Some characters may be destined only
  to be logged or even ignored, depending on what features of the tape
  are enabled and what mode the tape is in. For example,
  a character from stdtrc will only be displayed if in split-tape mode.
  Another example is that a normal character from stdout of the
  server will only be displayed (if not in wrap mode) if the
  tape is not yet full (as the tape has a finite length).
  To understand this, follow the calls below.
*/
void handleServerChar(tape_t *t, CHAR_TYPE c) {
  #ifdef TAPE_DEBUG
    fprintf(trc,"handleServerChar:%d '%c'\n",c,c);
    fflush(trc);
  #endif
#ifdef MULTIPLEX
  if (!multiRouteInKnown) {
    switch(c) {
      case(STDOUT_BYTE):
        multiRouteIn = c;
        multiRouteInKnown = TRUE;
        return;
      case(STDTRC_BYTE):
        multiRouteIn = c;
        multiRouteInKnown = TRUE;
        return;
      break;    
      default:
        msgErr(t);
        return;
      break;
    }
  }

  if (multiRouteIn == STDTRC_BYTE) {
    // We received a data byte from server's STDTRC
    ttrace(t, c);
  } else { // We received a data byte from server's STDOUT
#endif
    if (t->dleseq.seq[0] == CHAR_DLE) {
      // We are in an esc seq sent by the server
      if (t->dleseq.index < SERVER_DLE_LEN-1) {
        // We have not yet received the complete esc seq
        t->dleseq.seq[(t->dleseq.index)++] = c;
        setServerReady(t, false);
      } else {
        // We have received the complete esc seq
        handleServerEscseq(t);
        clearEscseq(t);
        setServerReady(t,true);
      }
    } else {
      if (c == CHAR_DLE ) {
        // This appears to be the start of an esc seq sent by the server
        clearEscseq(t);
        t->dleseq.seq[0] = CHAR_DLE;
        t->dleseq.index = 1;
        setServerReady(t, false);
      } else {
        // Not an esc seq at all
        if (c == CHAR_XON) {
          setServerReady(t,true);
        } else if ( c == CHAR_XOFF ) {
          setServerReady(t, false);
        } else {
          show(t,c);
          setServerReady(t,true);
        }
      }
    }
#ifdef MULTIPLEX
  }
  multiRouteInKnown = FALSE;
#endif
}

/*
  Write a byte out over the serial device so that it is sent to the server.
  This code assumes that CHAR_TYPE is the size of a byte.
*/
void writeByteServer(tape_t *t, CHAR_TYPE c) {
  #ifdef TAPE_DEBUG
    fprintf(trc,"writeByteServer char: %d\n",c);
    fflush(trc);
  #endif
  CHAR_TYPE buf[1];
  int countWritten;
#ifdef MULTIPLEX
    buf[0] = STDIN_BYTE;
    countWritten = write(server, buf, 1);
    if (countWritten < 1) {
      msgErr(t);
      #ifdef TAPE_DEBUG
        fprintf(trc,"writeByteServer muxId FAILED: %d\n",buf[0]);
        fflush(trc);
      #endif
      return;
    }
  #ifdef TAPE_DEBUG
    fprintf(trc,"writeByteServer muxId: %d '%c'\n",buf[0],buf[0]);
    fflush(trc);
  #endif
#endif
  buf[0] = c;
  countWritten = write(server, buf, 1);
  if (countWritten < 1) {
    msgErr(t);
  #ifdef TAPE_DEBUG
    fprintf(trc,"writeByteServer data FAILED: %d\n",buf[0]);
    fflush(trc);
  #endif
  }
  setServerReady(t, false); // Not ready until get a server response
  #ifdef TAPE_DEBUG
    fprintf(trc,"writeByteServer data: %d '%c'\n",buf[0],buf[0]);
    fflush(trc);
  #endif
}

/*
  Send a simple DLE escape sequence to the server.
  The format of escape sequences communicated between the server and
  the tape is always DLE_LENGTH bytes long (currently 6 bytes).
  The first byte is CHAR_DLE, the second byte is a command number,
  and the last 4 bytes are zeroes since a simple command takes no argument.
*/
void simpleDle(tape_t *t, CHAR_TYPE dleCmd) {
  writeByteServer(t,CHAR_DLE);
  writeByteServer(t,dleCmd);
  for (int i=0; i<DLE_LENGTH-2; i++) {
    writeByteServer(t,0);
  }
}

/*
  Send a complex DLE escape sequence to the server.
  The format of escape sequences communicated between the server and
  the tape is always DLE_LENGTH bytes long (currently 6 bytes).
  The first byte is CHAR_DLE, the second byte is a command number,
  and the last 4 bytes are represent a Freeputer WORD argument
  (two's complement, little-endian, 32-bit integer).

  Note: the format of a complex DLE escape sequence is used not only
  for the server to command the tape with an argument but also for the
  tape to respond to a query by providing information as an argument.
*/
void complexDle(tape_t *t, CHAR_TYPE dleCmd, int val) {
  writeByteServer(t,CHAR_DLE);
  writeByteServer(t,dleCmd);
  writeByteServer(t,(CHAR_TYPE)(val & 0xff));
  writeByteServer(t,(CHAR_TYPE)((val>>8)&0xff));
  writeByteServer(t,(CHAR_TYPE)((val>>16)&0xff));
  writeByteServer(t,(CHAR_TYPE)((val>>24)&0xff));
}

/*
  Write to the server. This is quite complicated as the character c
  being written might be part of a escape sequence generated by ncurses
  as a result of keyboard input, in which case this writeServer function
  does not write that character to the server but waits until
  a complete escape sequence from ncurses has already
  been decoded, then sends (rather than the character c)
  an appropriate simple or complex DLE escape sequence that the
  sever will recognize. The server, upon seeing such a command escape
  sequence, will make its own decision to either ignore the command or
  echo the command escape sequence back to the tape so that the
  tape takes action (such as clearing the tape) appropriate
  as commanded. See 'ts.fl' for an example of how that is done.
*/
void writeServer(tape_t *t, CHAR_TYPE c) {
  if (t->keyseq.seq[0] == CHAR_ESC) {
    // We are in a key esc seq
    const CHAR_TYPE *seq = &(t->keyseq.seq[0]);
    if (!strcmp(seq,"\033")) { return; } // await seq completion
    if (!strcmp(seq,"\033O")) { return; } // await seq completion
    if (!strcmp(seq,"\033\013")) { // Ctrl-K
      simpleDle(t, CMD_DLE_CLEAR_FROM); clearKeyseq(t); return;
    }
    if (!strcmp(seq,"\033\014")) { // Ctrl-L
      simpleDle(t, CMD_DLE_CLEAR); clearKeyseq(t); return;
    }
    if (!strcmp(seq,"\033\022")) { // Ctrl-R
      simpleDle(t, CMD_DLE_NO_WRAP); clearKeyseq(t); return;
    }
    if (!strcmp(seq,"\033\024")) { // Ctrl-T
      simpleDle(t, CMD_DLE_VISIBLE_SPC); clearKeyseq(t); return;
    }
    if (!strcmp(seq,"\033\025")) { // Ctrl-U
      simpleDle(t, CMD_DLE_SPLIT_MODE); clearKeyseq(t); return;
    }
    if (!strcmp(seq,"\033\027")) { // Ctrl-W
      simpleDle(t, CMD_DLE_WRAP); clearKeyseq(t); return;
    }
    if (!strcmp(seq,"\033OC")) {
      simpleDle(t, CMD_DLE_RIGHT); clearKeyseq(t); return;
    }
    if (!strcmp(seq,"\033OD")) {
      simpleDle(t, CMD_DLE_LEFT); clearKeyseq(t); return;
    }
    if (!strcmp(seq,"\033OA")) {
      simpleDle(t, CMD_DLE_UP); clearKeyseq(t); return;
    }
    if (!strcmp(seq,"\033OB")) {
      simpleDle(t, CMD_DLE_DOWN); clearKeyseq(t); return;
    }
  }
  writeByteServer(t,c);
}

/*
  This handleOver function should be called when not in wrap mode and
  output has reached the end of the tape (not including the message cell
  and not including, if in split-tape mode, the second half of the
  tape which is used for stdtrc output). This handleOver function
  displays a message (by calling msgOver()) so the user knows
  overflow occurred and it also informs the server that overflow
  occurred by sending CHAR_EM to the server). The aim of this
  is to inform both the user and the server that some
  information the server wanted the user to see was
  not seen because the tape was full. What happens next is up to the
  server (it might keep sending characters anyway, knowing they
  will not be visible, or it might stop sending characters,
  or it might reposition the cursor, or command the tape to clear,
  or whatever is the appropriate behaviour for the application).

  In short: when in wrap mode, previous output is simply overwritten
  as the tape automatically would clear itself and keep displaying
  characters received (thus handleOver should never be called in wrap mode).
  That means the most recent output is always visible but earlier
  output might be overwriten. Whereas when not in wrap mode,
  earlier output is visible and more recent output is not visible
  once the tape becomes full (unless the server decides to make
  something else happen when handleOver is called and it subsequently
  receives CHAR_EM as notification). Note that EM is End of Medium.
*/
void handleOver(tape_t *t) {
  t->over = true;
  msgOver(t);
  writeServer(t,CHAR_EM);
}

/*
  This function could equally well have been called interact.
  It causes the tape to listen for bytes and respond appropriately
  to them, taking into account whether they originate locally from
  ncurses (keyboard input or pasting into the tape) or
  from the server (via the serial device). This is the main
  loop for the tape user interface interaction.
*/
void slave(tape_t *t) {
  for (;;)
  {
      ochar oc = input(t);
      switch(oc.o) {
        case(ORIGIN_SERVER):
          handleServerChar(t, oc.c);
        break;
        case(ORIGIN_LOCAL):
          writeServer(t, oc.c);
        break;
        default:
          msgErr(t);
        break;
      }
  }
}

/* 
  This is the entry point for starting and running a tape.
  It initializes the tape by calling tape_init and then enters interactive
  mode by calling slave. The user can then happily use the tape
  as the user interface to the application controlling it
  via the serial device specified by deviceName.
*/
bool tape_run(tape_t *t, const CHAR_TYPE *deviceName) {
  bool initialized = tape_init(t, deviceName);
  if (initialized) {
    slave(t);
  } else {
    tape_shutdown(t);
  }
  return initialized;
}

/*
  Display a character on the tape. Monochrome only.

  Note: this will only show a character on the normal part of the tape,
  no higher than maxPos. It will not show a character on the stdtrc
  part of the tape in split-tape mode.
*/
bool tape_putc(tape_t *t, CHAR_TYPE c) {
  if (t->pos >= t->maxPos) {
    if (t->wrap) {
      tape_clear(t);
      msgWrapped(t);
    } else {
      handleOver(t);
      return false;
    }
  }
  t->lastPos = t-> pos;
  addch(c); // ncurses: attempt to show character
  getyx(stdscr, t->row, t->col); // ncurses: get cursor location
  ++(t->pos);
      #ifdef TAPE_DEBUG
        fprintf(trc,"in tape_putc just did ++(t->pos) so now: %d\n",t->pos);
        fflush(trc);
      #endif 
  refresh(); // ncurses: update view
  return true;
}

/*
  Only used in split-tape mode.

  Goto the specified position pos2 in the the second half of the tape where
  stdtrc output is displayed. The ncurses cursor is sent to that cell.
*/
#ifdef TAPE_SPLIT_TRC
bool tape_goto2(tape_t *t, int pos2) {
  if (pos2 > t->maxPos2 || pos2 < t->maxPos+1 ) {
    return false;
  }
  //t->lastPos = t-> pos;
  t->pos2 = pos2;
      #ifdef TAPE_DEBUG
        fprintf(trc,"just did t->pos2 = pos2 in tape_goto so now: %d\n",t->pos);
        fflush(trc);
      #endif 
  int col2 = pos2 % t->maxX;
  int row2 = pos2 / t->maxX;
  move(row2,col2); // ncurses: relocate cursor
  refresh(); // ncurses: update view
  return true;
}

/*
  Only used in split-tape mode.

  Display a character in the second half of the tape where
  stdtrc output is displayed. Only 7-bit ASCII should be used here.

  Monochrome only.
*/
bool tape_putc2(tape_t *t, CHAR_TYPE c) {
  if (t->pos2 >= t->maxPos2) { // Always wrap tracing
    t->pos2 = t->maxPos+1;
    tape_goto2(t,t->pos2);
    clrtobot(); // ncurses: clear from cursor to end of screen inclusive
  } else {
    tape_goto2(t,t->pos2);
  }
  addch(c); // ncurses: attempt to show character
  ++(t->pos2);
  #ifdef TAPE_DEBUG
    fprintf(trc,"in tape_putc2 just did ++(t->pos2) so now: %d\n",t->pos2);
    fflush(trc);
  #endif 
  refresh(); // ncurses: update view
  return true;
}
#endif

/*
  Display a character on the tape in colour.

  Note: this will only show a character on the normal part of the tape,
  no higher than maxPos. It will not show a character on the stdtrc
  part of the tape in split-tape mode.
*/
#if DISPLAY_COLOR == true
bool tape_putc_color(tape_t *t, CHAR_TYPE c, int colorPairId) {
  attron(COLOR_PAIR(colorPairId)); // ncurses: attribute on
  bool outcome = tape_putc(t,c);
  attroff(COLOR_PAIR(colorPairId)); // ncurses: attribute off
  return outcome;
}
  #ifdef TAPE_SPLIT_TRC
  bool tape_putc_color2(tape_t *t, CHAR_TYPE c, int colorPairId) {
    attron(COLOR_PAIR(colorPairId)); // ncurses: attribute on
    bool outcome = tape_putc2(t,c);
    attroff(COLOR_PAIR(colorPairId)); // ncurses: attribute off
    return outcome;
  }
  #endif
#endif

/*
  Goto the specified position pos in the tape.
  The ncurses cursor is sent to that cell.

  Note: this will only goto a cell in the normal part of the tape,
  no higher than maxPos. It will goto a cell on the stdtrc
  part of the tape in split-tape mode.
*/
bool tape_goto(tape_t *t, int pos) {
  if (pos > t->maxPos || pos < 0 ) {
    return false;
  }
  t->lastPos = t-> pos;
  t->pos = pos;
      #ifdef TAPE_DEBUG
        fprintf(trc,"just did t->pos = pos in tape_goto so now: %d\n",t->pos);
        fflush(trc);
      #endif 
  t->col = pos % t->maxX;
  t->row = pos / t->maxX;
  move(t->row,t->col); // ncurses: relocate cursor
  refresh(); // ncurses: update view
  return true;
}

/*
  Clear the whole tape.

  The OK message (a single character at maxPos) will be displayed
  to indicate all is well.
*/
bool tape_clear(tape_t *t) {
  clear(); // ncurses: clear screen
  msgOK(t);
  #ifdef TAPE_SPLIT_TRC
    if (t->splitMode == true) {
      t->pos2 = t->maxPos+1;
    }
  #endif
  tape_goto(t,0);
  return true;
}

/*
  Clear from the cursor to the end of the tape.

  The OK message (a single character at maxPos) will be displayed
  to indicate all is well.
*/
bool tape_clearFrom(tape_t *t) {
  clrtobot(); // ncurses: clear from cursor to end of screen inclusive
  msgOK(t);
  #ifdef TAPE_SPLIT_TRC
    if (t->splitMode == true) {
      t->pos2 = t->maxPos+1;
    }
  #endif
  return true;
}

/* Gracefully exit, shutting down the tape. */
bool tape_shutdown(tape_t *t) {
  endwin(); // ncurses: graceful exit
  if (server) {
    close(server);
  }
  #ifdef TAPE_DEBUG
    fclose(trc);
    fclose(fvmtrc);
  #endif
  return true;
}

/*
  The following functions (vis2, invis2, show2) serve the same
  purpose for displaying characters on the second half of the tape
  in split-tape mode, where stdtrc output is displayed, as the
  corresponding (vis, invis, show) normal functions do but
  are only used for stdtrc output.

  Important: UTF-8 is not supported here.
  The second half of the tape, in this implementation,
  in split-tape mode only supports 7-bit ASCII. 
*/
#ifdef TAPE_SPLIT_TRC
int vis2(tape_t *t, CHAR_TYPE c) {
    if (c == CHAR_SPC && t->visibleSpc) {
      #if DISPLAY_COLOR == true
        tape_putc_color2(t, CHAR_VISIBLE_SPC, COLOR_VIS_SPACE);
      #else
        tape_putc2(t, CHAR_VISIBLE_SPC);
      #endif
    } else {
      tape_putc2(t,c);
    }
}

int invis2(tape_t *t, CHAR_TYPE c) { // é
  switch(c) {
    case(CHAR_HT):
      #if DISPLAY_COLOR == true
        tape_putc_color2(t, '*', COLOR_HT);
      #else
        tape_putc2(t, '*');
      #endif
      break;
    case(CHAR_NL):
      #if DISPLAY_COLOR == true
        tape_putc_color2(t, '*', COLOR_NL);
      #else
        tape_putc2(t, '*');
      #endif
      break;
    case(CHAR_CR):
      #if DISPLAY_COLOR == true
        tape_putc_color2(t, '*', COLOR_CR);
      #else
        tape_putc2(t, '*');
      #endif
      break;
    default:
      #if DISPLAY_COLOR == true
        tape_putc_color2(t, '?', COLOR_INVIS);
      #else
        tape_putc2(t, '?');
      #endif
      break;
  }
}

void show2(tape_t *t, CHAR_TYPE c) {
  #ifdef TAPE_DEBUG
    fprintf(trc,"show2:%d t->pos2 %d \n ",c,t->pos2);
    fprintf(trc,"and in show2 multi:%d '%c'\n",c,c);
    fflush(trc);
  #endif
  // Important: note the deliberate lack of UTF-8 support here.
  // The second half of the tape (in split-mode) only supports 7-bit ASCII
  // in this implementation. This keeps things simple.
  if (c > 127 || c < CHAR_SPC) {
    invis2(t, c);
  } else {
    vis2(t, c);
  }
  refresh();
}
#endif // #ifdef TAPE_SPLIT_TRC

/*
  Character c is assumed to be from stdtrc of the FVM instance
  the tape is connected to over the serial device. This ttrace function
  either logs, or (in split-tape mode only) displays, or logs and displays,
  that character, depending on whether TAPE_DEBUG and TAPE_SPLIT_TRC
  are defined. None of this will work, however, unless MULTIPLEX is also
  defined and the connected FVM instance properly supports multiplexing,
  an optional feature new in FVM 1.1. So this will not work with
  current FVM 1.0 implementations.

  Note: if you are using a pseudoterminal device (such as /dev/pts/0)
  to connect to a local FVM instance also running on Linux, you don't need
  to use any of this because the tracing output from your local
  FVM instance will be sent by it to std.trc as usual (unless,
  of course, it is itself set to MULTIPLEX mode).

  So when would you use this?

  This ttrace function (and split-tape mode in general) is mainly
  intended to be used when you are using multiplexing to connect this tape
  via a physical serial device to a remote computer or remote microcontroller
  and so you do not have direct access to the std.trc output file
  of the FVM instance running on that device. Therefore you would arrange
  for that remote FVM instance to be started up in multiplexing mode
  and then use split-tape mode (which uses this ttrace function)
  to happily view tracing output from the remote FVM.
*/
int ttrace(tape_t *t, CHAR_TYPE c) {
  #ifdef TAPE_DEBUG
    fputc(c,fvmtrc);
    fflush(fvmtrc);
  #endif

  #ifdef TAPE_SPLIT_TRC
  if (t->splitMode == true) {
    curs_set(0); // ncurses: make cursor invisible
    show2(t,c);
    tape_goto(t,t->pos);
    curs_set(1); // ncurses: make cursor visible
    refresh();
  }
  #endif
}

tape_t currentTape = {0};
static void quit(int sig) { tape_shutdown(&currentTape); exit(0); }

/*
  Run the tape. You must supply one command line argument, being the
  name of the serial device the tape should connect to. For example:

    ./tape /dev/ttyUSBO

  or

    ./tape /dev/pts/0

  Note: do not resize the terminal emulator in which you are running
  the tape while the tape is running. Resize your terminal emulator to
  your preferred size (even full-screen if you like) before
  running the tape.
  
*/
int main(int argc, CHAR_TYPE *argv[]) {
  if (argc < 2) {
    printf("You must specify a serial device name\n");
    return 2;
  }
  const CHAR_TYPE *deviceName = argv[1];
  if (tape_run(&currentTape, deviceName)) {
    return 0;
  } else {
    return 1;
  }
}


