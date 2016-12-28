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
import java.net.ServerSocket;
import java.net.Socket;

public class ServerSocketPiper implements Piper {

  private final ServerSocket serverSocket;
  private final Socket clientSocket;
  private final OutputStream serverToClient;
  private final InputStream clientToServer;

  public ServerSocketPiper(int port) throws IOException {
    serverSocket = new ServerSocket(port);
    clientSocket = serverSocket.accept();
    this.serverToClient = new BufferedOutputStream(
        clientSocket.getOutputStream(), 1024);
    this.clientToServer = new BufferedInputStream(
        clientSocket.getInputStream(), 1024);
  }

  @Override
  public int send(int outByte) {
    try {
      serverToClient.write(outByte);
      serverToClient.flush();
      return Piper.SUCCESS;
    } catch (IOException e) {
      return Piper.FAILURE;
    }
  }

  @Override
  public int receive() {
    try {
      final int result;
      result = clientToServer.read();
      return result;
    } catch (IOException e) {
      return Piper.FAILURE;
    }
  }

  @Override
  public int avail() {
    try {
      final int avail;
      avail = clientToServer.available();
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
