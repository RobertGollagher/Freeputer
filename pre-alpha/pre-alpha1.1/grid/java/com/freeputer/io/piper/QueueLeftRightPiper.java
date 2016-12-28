/*

Copyright © 2016, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

License: GNU General Public License Version 3 or any later version
Author : Robert Gollagher  robert.gollagher@freeputer.net
Created: 20161228
Updated: 20161228
Version: pre-alpha-0.0.0.2

=========================================================================== */

package com.freeputer.io.piper;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

public class QueueLeftRightPiper implements LeftRightPiper {

  private final Piper leftPiper;
  private final Piper rightPiper;

  public QueueLeftRightPiper() {
    LinkedBlockingQueue<Byte> leftToRight = 
        new LinkedBlockingQueue<Byte>(1024);
    LinkedBlockingQueue<Byte> rightToLeft =
        new LinkedBlockingQueue<Byte>(1024);
    leftPiper = new QueuePiper(leftToRight, rightToLeft);
    rightPiper = new QueuePiper(rightToLeft, leftToRight);
  }

  @Override
  public Piper getLeft() {
    return leftPiper;
  }

  @Override
  public Piper getRight() {
    return rightPiper;
  }

  class QueuePiper implements Piper {

    private final BlockingQueue<Byte> sender;
    private final BlockingQueue<Byte> receiver;

    public QueuePiper(LinkedBlockingQueue<Byte> sender,
        LinkedBlockingQueue<Byte> receiver) {
      this.sender = sender;
      this.receiver = receiver;
    }

    @Override
    public int send(int outByte) {
      try {
        sender.put((byte) outByte);
        return Piper.SUCCESS;
      } catch (Exception e) {
        return Piper.FAILURE;
      }
    }

    @Override
    public int receive() {
      try {
        int result;
        result = receiver.take();
        return result;
      } catch (Exception e) {
        return Piper.FAILURE;
      }
    }

    @Override
    public int avail() {
      final int result;
      result = receiver.size();
      return result;
    }

    @Override
    public void close() {
      // FIXME implement
    }
  }
}
