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

public class Tile {

  private final Chr left;
  private final Chr right;

  public enum Side {
    LEFT, RIGHT
  }

  public static final int WIDTH_IN_CHRS = 2;

  public Tile() {
    this.left = Chr.EMPTY;
    this.right = Chr.EMPTY;
  }

  public Tile(Chr left, Chr right) {
    if (left == null || right == null) {
      throw new IllegalArgumentException("null not permitted");
    }
    this.left = left;
    this.right = right;
  }

  public Chr getLeft() {
    return left;
  }

  public Chr getRight() {
    return right;
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = prime * result + ((left == null) ? 0 : left.hashCode());
    result = prime * result + ((right == null) ? 0 : right.hashCode());
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
    Tile other = (Tile) obj;
    if (!left.equals(other.left))
      return false;
    if (!right.equals(other.right))
      return false;
    return true;
  }

  @Override
  public String toString() {
    return String.format("[\"%c%c\"]", left.getChr(), right.getChr());
  }

}
