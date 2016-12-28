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
import java.net.Socket;

public class ClientSocketPiper implements Piper {

  private Socket clientSocket = null;
  private final InputStream serverToClient;
  private final OutputStream clientToServer;

  public ClientSocketPiper(String serverHostname, int serverPort)
      throws IOException {
    
    while (clientSocket == null) {
      try {
        clientSocket = new Socket(serverHostname, serverPort);
      } catch (IOException keepWaiting) {
        // FIXME park for a while here
      }
    }
    this.clientToServer = new BufferedOutputStream(
        clientSocket.getOutputStream(), 1024);
    this.serverToClient = new BufferedInputStream(
        clientSocket.getInputStream(), 1024);
  }

  @Override
  public int send(int outByte) {
    try {
      clientToServer.write(outByte);
      clientToServer.flush();
      return Piper.SUCCESS;
    } catch (IOException e) {
      return Piper.FAILURE;
    }
  }

  @Override
  public int receive() {
    try {
      final int result;
      result = serverToClient.read();
      return result;
    } catch (IOException e) {
      return Piper.FAILURE;
    }
  }

  @Override
  public int avail() {
    try {
      final int avail;
      avail = serverToClient.available();
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
