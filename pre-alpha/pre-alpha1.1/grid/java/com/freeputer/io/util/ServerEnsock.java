/*

Copyright © 2016, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

License: GNU General Public License Version 3 or any later version
Author : Robert Gollagher  robert.gollagher@freeputer.net
Created: 20161228
Updated: 20161229
Version: pre-alpha-0.0.0.1

=========================================================================== */

package com.freeputer.io.util;

import java.io.IOException;

import com.freeputer.io.piper.CircularPiper;
import com.freeputer.io.piper.NativePiper;
import com.freeputer.io.piper.Piper;
import com.freeputer.io.piper.ServerSocketPiper;

public class ServerEnsock {

  private final CircularPiper circle;

  public ServerEnsock(int port, String... cmd) throws IOException {
    Piper leftPiper = new ServerSocketPiper(port);
    Piper rightPiper = new NativePiper(cmd);
    circle = new CircularPiper(leftPiper, rightPiper, null);
  }

  public void run() {
    circle.run();
  }

  public static void main(String[] args) throws IOException {
    if (args.length != 2) {
      System.out.println("Must specify PORT COMMAND such as by:\n"
          + "java com.freeputer.io.util.ServerEnsock 1234 ./fvm");
      return;
    }
    ServerEnsock ensock;
    try {
      int port = Integer.parseInt(args[0]);
      ensock = new ServerEnsock(port, args[1]);
      ensock.run();
    } catch (Exception e) {
      System.err.println(e.getMessage());
    } finally {
      // FIXME close Pipers here
    }

  }
}
