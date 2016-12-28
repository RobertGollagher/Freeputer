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
import com.freeputer.io.piper.ClientSocketPiper;
import com.freeputer.io.piper.NativePiper;
import com.freeputer.io.piper.Piper;

public class ClientEnsock {

  private final CircularPiper circularPiper;

  public ClientEnsock(String serverHostname, int serverPort, String... cmd)
      throws IOException {
    Piper leftPiper = new ClientSocketPiper(serverHostname, serverPort);
    Piper rightPiper = new NativePiper(cmd);
    circularPiper = new CircularPiper(leftPiper, rightPiper, null);
  }

  public void run() {
    circularPiper.run();
  }

  public static void main(String[] args) throws IOException {
    if (args.length != 3) {
      System.out.println("Must specify HOSTNAME PORT COMMAND such as by:\n"
          + "java com.freeputer.io.util.ClientEnsock localhost 1234 ./fvm");
      return;
    }
    ClientEnsock ensock;
    try {
      int port = Integer.parseInt(args[1]);
      ensock = new ClientEnsock(args[0], port, args[2]);
      ensock.run();
    } catch (Exception e) {
      System.err.println(e.getMessage());
    } finally {
      // FIXME close Pipers here
    }
  }
}
