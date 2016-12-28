/*

Copyright © 2016, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

License: GNU General Public License Version 3 or any later version
Author : Robert Gollagher  robert.gollagher@freeputer.net
Created: 20161228
Updated: 20161228
Version: pre-alpha-0.0.0.1

=========================================================================== */

package com.freeputer.io.piper;

import java.util.concurrent.locks.LockSupport;

public class CircularPiper {

  private final Piper leftPiper;
  private final Piper rightPiper;
  private final Long waitNanos;
  private final static long DEFAULT_WAIT_NANOS = 100000;

  public CircularPiper(Piper leftPiper, Piper rightPiper, Long waitNanos) {
    this.leftPiper = leftPiper;
    this.rightPiper = rightPiper;
    if (waitNanos == null) {
      this.waitNanos = DEFAULT_WAIT_NANOS;
    } else {
      this.waitNanos = waitNanos;
    }
  }

  public void run() {
    int fromLeft;
    int fromRight;
    while (true) { // FIXME endless, needs FAILURE checks, needs close() calls
      LockSupport.parkNanos(waitNanos);
      if (leftPiper.avail() > 0) {
        fromLeft = leftPiper.receive();
        rightPiper.send(fromLeft);
      }
      if (rightPiper.avail() > 0) {
        fromRight = rightPiper.receive();
        leftPiper.send(fromRight);
      }
    }
  }

}
