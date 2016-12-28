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

import com.freeputer.ui.grid.Tile.Side;

public class GridModel {

  private static final int TILES_PER_ROW = 20;
  private static final int COLS = TILES_PER_ROW * Tile.WIDTH_IN_CHRS;
  private static final int ROWS = 12;

  private final Tile[][] tiles;

  public GridModel() {
    this.tiles = new Tile[TILES_PER_ROW][ROWS];
    for (int i = 0; i < TILES_PER_ROW; i++) {
      for (int j = 0; j < ROWS; j++) {
        tiles[i][j] = new Tile();
      }
    }
  }

  public Chr getChr(int col, int row, Chr chr) {
    final Tile tile = getTileContaining(col, row);
    final Chr result;
    final Side side = getSideForCol(col);
    if (side == Side.LEFT) {
      result = tile.getLeft();
    } else {
      result = tile.getRight();
    }
    return result;
  }

  public void setChr(int col, int row, Chr chr) {
    final Tile olderTile = getTileContaining(col, row);
    final Tile resultTile;
    final Chr resultChr;
    final Side side = getSideForCol(col);
    if (side == Side.LEFT) {
      resultChr = Chr.merge(olderTile.getLeft(), chr);
      resultTile = new Tile(resultChr, olderTile.getRight());
    } else {
      resultChr = Chr.merge(olderTile.getRight(), chr);
      resultTile = new Tile(olderTile.getLeft(), resultChr);
    }
    setTileContaining(col, row, resultTile);
  }

  public Tile getTile(int tilesCol, int tilesRow) {
    validateTilePos(tilesCol, tilesRow);
    return tiles[tilesCol][tilesRow];
  }

  public void setTile(int tilesCol, int tilesRow, Tile tile) {
    validateTilePos(tilesCol, tilesRow);
    tiles[tilesCol][tilesRow] = tile;
  }

  public Tile getTileContaining(int col, int row) {
    final int tilesCol = getTileColumn(col);
    final int tilesRow = getTileRow(row);
    return tiles[tilesCol][tilesRow];
  }

  public void setTileContaining(int col, int row, Tile tile) {
    final int tilesCol = getTileColumn(col);
    final int tilesRow = getTileRow(row);
    tiles[tilesCol][tilesRow] = tile;
  }

  public int getTileColumn(int col) {
    validateCol(col);
    final int result = (col - 1) / Tile.WIDTH_IN_CHRS;
    return result;
  }

  public int getTileRow(int row) {
    validateRow(row);
    final int result = row - 1;
    return result;
  }

  public Side getSideForCol(int col) {
    final Side result;
    int n = (col - 1) % Tile.WIDTH_IN_CHRS;
    if (n == 0) {
      result = Side.LEFT;
    } else {
      result = Side.RIGHT;
    }
    return result;
  }

  private void validateRow(int row) {
    if (row < 1 || row > ROWS) {
      throw new IllegalArgumentException("Row out of range: " + row);
    }
  }

  private void validateCol(int col) {
    if (col < 1 || col > COLS) {
      throw new IllegalArgumentException("Column out of range: " + col);
    }
  }

  private void validateTilePos(int tileCol, int tileRow) {
    validateTileCol(tileCol);
    validateTileRow(tileRow);
  }

  private void validateTileRow(int tileRow) {
    if (tileRow < 0 || tileRow > ROWS - 1) {
      throw new IllegalArgumentException("Row out of range: " + tileRow);
    }
  }

  private void validateTileCol(int tileCol) {
    if (tileCol < 0 || tileCol > COLS - 1) {
      throw new IllegalArgumentException("Column out of range: " + tileCol);
    }
  }

  public static int getCols() {
    return COLS;
  }

  public static int getRows() {
    return ROWS;
  }

  public static int getTilesPerRow() {
    return TILES_PER_ROW;
  }

}
