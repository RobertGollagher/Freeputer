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

public class Chr {

  private final char chr;
  private final Color fgColor;
  private final Color bgColor;
  public static final Chr EMPTY = new Chr(' ');

  public Chr(char chr) {
    this.chr = chr;
    this.fgColor = null;
    this.bgColor = null;
  }

  public Chr(char chr, Color fgColor, Color bgColor) {
    this.chr = chr;
    this.fgColor = fgColor;
    this.bgColor = bgColor;
  }

  public static Chr merge(Chr older, Chr newer) {
    final Chr result;
    final Color fg;
    final Color bg;
    if (newer.fgColor != null) {
      fg = newer.getFgColor();
    } else {
      fg = older.getFgColor();
    }
    if (newer.bgColor != null) {
      bg = newer.getBgColor();
    } else {
      bg = older.getFgColor();
    }
    result = new Chr(newer.getChr(), fg, bg);
    return result;
  }

  public char getChr() {
    return chr;
  }

  public Color getFgColor() {
    return fgColor;
  }

  public Color getBgColor() {
    return bgColor;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = prime * result + chr;
    return result;
  }

  @Override
  public boolean equals(Object obj) {
    if (this == obj)
      return true;
    if (obj == null)
      return false;
    if (getClass() != obj.getClass())
      return false;
    Chr other = (Chr) obj;
    if (chr != other.chr)
      return false;
    return true;
  }

  @Override
  public String toString() {
    return String.format("[chr=%08x fg=%s bg=%s : '%c']", (int) chr,
        fgColor == null ? "null   " : fgColor,
        bgColor == null ? "null   " : bgColor, chr);
  }

}
