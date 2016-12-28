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

public class Color {

  private final int value;

  public Color(int value) {
    this.value = value & 0x00ffffff;
  }

  public int getValue() {
    return value;
  }

  @Override
  public String toString() {
    return "#" + String.format("%06x", value);
  }
  
}
