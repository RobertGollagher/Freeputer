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

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class StreamPiper implements Piper {

  private final InputStream receiver;
  private final OutputStream sender;

  public StreamPiper(InputStream inStream, OutputStream outStream)
      throws IOException {
    this.receiver = new BufferedInputStream(inStream, 1024);
    this.sender = new BufferedOutputStream(outStream, 1024);
  }

  @Override
  public int send(int outByte) {
    try {
      sender.write(outByte);
      sender.flush();
      return Piper.SUCCESS;
    } catch (IOException e) {
      return Piper.FAILURE;
    }
  }

  @Override
  public int receive() {
    try {
      final int result;
      result = receiver.read();
      return result;
    } catch (IOException e) {
      return Piper.FAILURE;
    }
  }

  @Override
  public int avail() {
    try {
      final int avail;
      avail = receiver.available();
      return avail;
    } catch (IOException e) {
      return 0;
    }
  }

  @Override
  public void close() throws IllegalStateException {
    try {
      sender.close();
    } catch (IOException e) {
      throw new IllegalStateException(e);
    }
    try {
      receiver.close();
    } catch (IOException e) {
      throw new IllegalStateException(e);
    }
  }

}
