/*

Copyright © 2016, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

License: GNU General Public License Version 3 or any later version
Author : Robert Gollagher  robert.gollagher@freeputer.net
Created: 20161228
Updated: 20161229-1627+
Version: pre-alpha-0.0.0.1

=========================================================================== */

package com.freeputer.ui.grid.javafx;

import java.io.UnsupportedEncodingException;
import java.util.Vector;
import java.util.concurrent.locks.LockSupport;

import com.freeputer.io.piper.Piper;
import com.freeputer.ui.grid.Chr;
import com.freeputer.ui.grid.Plot;

import javafx.concurrent.Service;
import javafx.concurrent.Task;

public class ReadService extends Service<Vector<Plot>> {

  private final Piper piper;

  public ReadService(Piper piper) {
    this.piper = piper;
  }

  protected Task<Vector<Plot>> createTask() {
    return new Task<Vector<Plot>>() {
      protected Vector<Plot> call() {
        Vector<Plot> result = new Vector<Plot>();

        while (piper.avail() == 0) {
          LockSupport.parkNanos(100000); // FIXME
        }

        while (piper.avail() != 0) {
          int col = (byte) piper.receive(); // FIXME check for err
          int row = (byte) piper.receive(); // FIXME check for err

          // Read UTF-8 character (FIXME refactor to use CharsetDecoder)
          byte[] chrBytes = new byte[4];
          int i = 0;
          int limit = 1;
          byte b = (byte) piper.receive(); // FIXME check for err
          chrBytes[i++] = b;
          if (b < 0) {
            // This UTF-8 char is >=2 bytes long
            if ((b & 0b11100000) == 0b11000000) {
              // This UTF-8 char is exactly 2 bytes long
              limit = 2;
            } else if ((b & 0b11110000) == 0b11100000) {
              // This UTF-8 char is exactly 3 bytes long
              limit = 3;
            } else if ((b & 0b11111000) == 0b11110000) {
              // This UTF-8 char is exactly 4 bytes long
              limit = 4; // FIXME not working for 4
            } else {
              // TODO handle invalid UTF-8 character
            }
            for (; i < limit; i++) {
              b = (byte) piper.receive(); // FIXME check for err
              chrBytes[i] = b;
            }
          }
          String sc;
          try {
            // (FIXME refactor to use CharsetDecoder)
            sc = new String(chrBytes, "UTF-8");
            char chr = sc.charAt(0);
            Plot plot = new Plot(col, row, new Chr(chr));
            result.add(plot);
          } catch (UnsupportedEncodingException e) {
            // FIXME handle corrupt data format
          }
        }
        return result;
      }
    };
  }

}
