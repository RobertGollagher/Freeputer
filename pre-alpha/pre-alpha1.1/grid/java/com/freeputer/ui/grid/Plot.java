/*

Copyright © 2016, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

License: GNU General Public License Version 3 or any later version
Author : Robert Gollagher  robert.gollagher@freeputer.net
Created: 20161228
Updated: 20161228
Version: pre-alpha-0.0.0.1

=========================================================================== */

package com.freeputer.ui.grid;

public class Plot {

  public final int col;
  public final int row;
  public final Chr chr;

  public Plot(int col, int row, Chr chr) {
    this.col = col;
    this.row = row;
    this.chr = chr;
  }

  @Override
  public String toString() {
    return "Plot [col=" + col + ", row=" + row + " : " + chr + "]";
  }

}
