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

public class Encircle {

  private final CircularPiper circle;

  public Encircle(String leftCmd, String rightCmd) throws IOException {
    Piper leftPiper = new NativePiper(leftCmd);
    Piper rightPiper = new NativePiper(rightCmd);
    circle = new CircularPiper(leftPiper, rightPiper, null);
  }

  public void run() {
    circle.run();
  }

  public static void main(String[] args) throws IOException {
    if (args.length != 2) {
      System.out.println(
          "Must specify exactly 2 commands without arguments such as by:\n"
              + "java com.freeputer.io.util.Encircle leftCmd rightCmd");
      return;
    }
    Encircle encircle;
    try {
      encircle = new Encircle(args[0], args[1]);
      encircle.run();
    } catch (Exception e) {
      System.err.println(e.getMessage());
    } finally {
      // FIXME close Pipers here
    }
  }

}
