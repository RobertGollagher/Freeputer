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

public class NativePiper implements Piper {

  private final Process nativeProcess;
  private final OutputStream sender;
  private final InputStream receiver;

  public NativePiper(String... cmd) throws IOException {

    nativeProcess = new ProcessBuilder(cmd).start();
    this.sender = new BufferedOutputStream(nativeProcess.getOutputStream(),
        1024); // stdin of nativeProcess
    this.receiver = new BufferedInputStream(nativeProcess.getInputStream(),
        1024); // stdout of nativeProcess
  }

  @Override
  public int send(int outByte) { // TODO maybe delegate to StreamPiper
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
  public void close() {
    // FIXME implement
  }

}
