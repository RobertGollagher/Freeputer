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

public interface Piper {

  public static final int SUCCESS = 1;
  public static final int FAILURE = -1;

  public int send(int c);

  public int receive();

  public int avail();

  public void close();

}
