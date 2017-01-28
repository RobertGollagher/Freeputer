/*

Copyright © 2016, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

License: GNU General Public License Version 3 or any later version
Program: Grid.java, an early experimental Freeputer Grid implementation
Author : Robert Gollagher  robert.gollagher@freeputer.net
Created: 20161228
Updated: 20170128-1445
Version: pre-alpha-0.0.0.2

WARNING: THIS IS PRE-ALPHA SOFTWARE. It is inherently experimental
in nature, is likely to greatly change before alpha release or to never
make it to alpha release, and may well be incomplete, unstable and
unreliable. That is, it is considered to be suitable only for
experimentation and nothing more.

=========================================================================== */

package com.freeputer.ui.grid.javafx;

import java.io.IOException;
import java.nio.charset.Charset;
import java.util.Iterator;
import java.util.Vector;

import com.freeputer.FVM;
import com.freeputer.io.piper.ClientSocketPiper;
import com.freeputer.io.piper.LeftRightPiper;
import com.freeputer.io.piper.NativePiper;
import com.freeputer.io.piper.Piper;
import com.freeputer.io.piper.QueueLeftRightPiper;
import com.freeputer.io.piper.ServerSocketPiper;
import com.freeputer.io.piper.StreamPiper;
import com.freeputer.ui.grid.Plot;

import javafx.application.Application;
import javafx.application.Platform;
import javafx.concurrent.WorkerStateEvent;
import javafx.event.EventHandler;
import javafx.geometry.HPos;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.Label;
import javafx.scene.control.OverrunStyle;
import javafx.scene.input.Clipboard;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyCombination;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.Background;
import javafx.scene.layout.BackgroundFill;
import javafx.scene.layout.Border;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.BorderStroke;
import javafx.scene.layout.BorderStrokeStyle;
import javafx.scene.layout.BorderWidths;
import javafx.scene.layout.CornerRadii;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.StackPane;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;
import javafx.stage.Stage;

public class Grid extends Application {

  public static final int STDGRID_COLS = 40;
  public static final int STDGRID_ROWS = 12;
  public static final int DEFAULT_FONT_SIZE = 24;
  public static final String DEFAULT_FONT_NAME = "Monospaced Regular";
  public static final boolean ENFORCE_MIN_TILE_BOUNDS = false;
  public static final int MIN_TILE_WIDTH = 32;
  public static final int MIN_TILE_HEIGHT = 32;

  public Grid() {
    this(STDGRID_COLS, STDGRID_ROWS);
  }

  public Grid(int cols, int rows) {
    this.cols = cols;
    this.rows = rows;
    FONT_SIZE = DEFAULT_FONT_SIZE;
    FONT_NAME = DEFAULT_FONT_NAME;
    GRID_FONT = new Font(FONT_NAME, FONT_SIZE);
    connect();
  }

  // Grid foreground color ---------------------------------------------------
  public static final Color GRID_FGC = Color.web("#BBBBBB");
  static Color currentFgc = GRID_FGC;
  // -------------------------------------------------------------------------

  // Grid background ---------------------------------------------------------
  public static final Color GRID_BGC = Color.web("#222222");
  public static final BackgroundFill GRID_BGF = new BackgroundFill(GRID_BGC,
      null, null);
  public static final Background GRID_BG = new Background(GRID_BGF);
  static Background currentBg = GRID_BG;

  public static void setCurrentBg(Color col) {
    Grid.currentBg = new Background(new BackgroundFill(col, null, null));
  } // ------------------------------------------------------------------------

  // Gridlines (when visible) ------------------------------------------------
  public static final Color GRID_BDC = Color.web("#000000");
  Border b = new Border(new BorderStroke(GRID_BDC, BorderStrokeStyle.SOLID,
      CornerRadii.EMPTY, new BorderWidths(1)));
  // --------------------------------------------------------------------------

  // Rim around grid (when window resized larger than grid) ------------------
  public static final Color RIM_CLR = Color.web("#0E0E0E");
  public static final BackgroundFill RIM_BGF = new BackgroundFill(RIM_CLR,
      null, null);
  public static final Background RIM_BG = new Background(RIM_BGF);
  // --------------------------------------------------------------------------

  // Cursor (see cursorCv and cursorGc) --------------------------------------
  public static final Color CURSOR_CLR = Color.web("#FFFFFF", 0.25d);
  // --------------------------------------------------------------------------

  private final int cols;
  private final int rows;
  private Piper leftPiper;
  private Piper rightPiper;
  private int FONT_SIZE;
  private final String FONT_NAME;
  private final Font GRID_FONT;
  private int col = 1;
  private int row = 1;
  private int prevCol = 1;
  private int prevRow = 1;
  private double totalTextWidth;
  private double totalTextHeight;
  private double fontWidth;
  private double fontHeight;

  private Clipboard clipbd;
  private BorderPane bp;
  private Scene scene;
  private StackPane sp;
  private GridPane gp;
  private Label[][] tiles;
  private Canvas cursorCv;
  private GraphicsContext cursorGc;

  @Override
  public void start(Stage mainStage) throws IOException {
    configueClipboard();
    configureScene();
    configureStage(mainStage);
    configureKeyPressed(mainStage);
    configureKeyTyped();
    configureReadService(mainStage);
    configureOther(mainStage);
    drawCursor();
  }

  private void configureScene() {
    sp = new StackPane();
    cursorCv = new Canvas();
    bp = new BorderPane();
    bp.setBackground(RIM_BG);
    bp.setCenter(sp);
    scene = new Scene(bp);
    configureGrid();
    sp.getChildren().addAll(gp, cursorCv);
  }

  private void configueClipboard() {
    clipbd = Clipboard.getSystemClipboard();
  }

  private void configureStage(Stage mainStage) {
    mainStage.setTitle("Freeputer Grid");
    mainStage.setScene(scene);
    mainStage.setFullScreenExitKeyCombination(KeyCombination.NO_MATCH);
  }

  private void configureOther(Stage mainStage) {
    mainStage.show();
    configureCursorCanvas();
    recalculateFontBounds();
    enforceSizeLimits(mainStage);
  }

  private void configureCursorCanvas() {
    cursorCv.setWidth(scene.getWidth());
    cursorCv.setHeight(scene.getHeight());
    cursorGc = cursorCv.getGraphicsContext2D();
  }

  private void enforceSizeLimits(Stage mainStage) {
    sp.setMaxHeight(sp.getHeight());
    sp.setMaxWidth(sp.getWidth());
    mainStage.setMinHeight(mainStage.getHeight());
    mainStage.setMinWidth(mainStage.getWidth());
  }

  private void recalculateFontBounds() {
    totalTextWidth = gp.getLayoutBounds().getWidth();
    totalTextHeight = gp.getLayoutBounds().getHeight();
    fontWidth = totalTextWidth / cols;
    fontHeight = totalTextHeight / rows;
  }

  private void configureGrid() {
    gp = new GridPane();
    gp.setBackground(GRID_BG);
    gp.setAlignment(Pos.CENTER);
    tiles = new Label[cols / 2][rows];
    for (int i = 0; i < cols / 2; i++) {
      for (int j = 0; j < rows; j++) {
        tiles[i][j] = new Label();
        tiles[i][j].setTextFill(GRID_FGC);
        tiles[i][j].setFont(GRID_FONT);
        tiles[i][j].setTextOverrun(OverrunStyle.CLIP);
        if (ENFORCE_MIN_TILE_BOUNDS) {
          double minw = MIN_TILE_WIDTH;
          double miny = MIN_TILE_HEIGHT;
          tiles[i][j].setMinSize(minw, miny);
          tiles[i][j].setPrefSize(minw, miny);
          tiles[i][j].setMaxSize(minw, miny);
        }
        tiles[i][j].setText("  ");
        gp.setHalignment(tiles[i][j], HPos.CENTER); // FIXME
        tiles[i][j].setAlignment(Pos.CENTER_LEFT);
        tiles[i][j].setBorder(b);
        gp.add(tiles[i][j], i, j);
      }
    }
  }

  private void configureKeyTyped() {
    scene.addEventFilter(KeyEvent.KEY_TYPED, event -> {
      String cs = event.getCharacter();
      char c = cs.charAt(0);
      if (c == 22) { // Ctrl-v
        Platform.runLater(() -> {
          writeToQueue(clipbd.getString());
        });
      } else if (c == 3) { // Ctrl-c
        Platform.runLater(() -> {
          /*
           * // FIXME final ClipboardContent content = new ClipboardContent();
           * content.putString(textHere); clipboard.setContent(content);
           */
        });
      } else {
        writeToQueue(c);
      }
    });
  }

  private void configureKeyPressed(Stage mainStage) {
    scene.addEventFilter(KeyEvent.KEY_PRESSED, event -> {
      KeyCode code = event.getCode();
      if (!code.equals(KeyCode.UNDEFINED)) {
        switch (code) {
        case UP:
          writeToQueue('w');
          break;
        case DOWN:
          writeToQueue('s');
          break;
        case LEFT:
          writeToQueue('a');
          break;
        case RIGHT:
          writeToQueue('d');
          break;
        case F11:
          mainStage.setFullScreen(!mainStage.isFullScreen());
        default:
          break;
        }
      }
    });
  }

  private void configureReadService(Stage mainStage) {

    ReadService service = new ReadService(rightPiper);
    service.setOnSucceeded(new EventHandler<WorkerStateEvent>() {

      @Override
      public void handle(WorkerStateEvent ev) {

        @SuppressWarnings("unchecked")
        Vector<Plot> result = (Vector<Plot>) ev.getSource().getValue();

        if (result == null || result.isEmpty()) {
          mainStage.setTitle("PROBLEM1!"); // FIXME error handling
          return;
        }

        handlePlots(mainStage, result);
        service.restart();
      }
    });

    service.start();
  }

  private void handlePlots(Stage mainStage, Vector<Plot> result) {
    Iterator<Plot> it = result.iterator();
    while (it.hasNext()) {
      Plot val = it.next();

      if (val.col < 0 || val.row < 0 || val.col > cols || val.row > rows) {
        mainStage.setTitle("PROBLEM2!"); // FIXME error handling
        continue;
      }
      if (val.col == 0 || val.row == 0) {
        if (val.col == 0 && val.row == 0) {
          // Plot is a command

          if (val.chr.getChr() == 0) {
            clearScreen();
          } else if (val.chr.getChr() == 1) {
            eraseCursor();
            continue;
          } else if (val.chr.getChr() == 4) {
            hideGridlines();
            continue;
          } else if (val.chr.getChr() == 5) {
            showGridlines();
            continue;
          } else {
            mainStage.setTitle("PROBLEM3!"); // FIXME error handling
            continue;
          }
        } else if (val.col == 0 && val.row == 1) {
          changeFgColor(val);
        } else if (val.col == 0 && val.row == 2) {
          changeBgColor(val);
        } else {
          mainStage.setTitle("PROBLEM4!"); // FIXME error handling
          continue;
        }
        
      } else {
        // Plot is a character to be displayed 
        // (or a command to draw the cursor) at row, col
        
        char c = val.chr.getChr();
        col = val.col;
        row = val.row;
        if (c == 1) { 
          // Plot is a command to draw the cursor at row, col
          drawCursor();
          continue;
        } else if (c > 0 && c < 32) { // FIXME reconsider range here
          // Grid does not support display of this character
          // so instead show a standard substitute character
          c = '*';
        }
        // Plot is a character to be displayed
        Label tile = tiles[tileCol(col)][tileRow(row)];
        String ts = tile.getText();
        StringBuffer sb = new StringBuffer(ts);
        if (sb.length() < 2) {
          sb = new StringBuffer("  ");
        }
        if (col % 2 == 0) {
          sb.replace(1, 2, c + "");
        } else {
          sb.replace(0, 1, c + "");
        }
        tile.setTextFill(currentFgc);
        tile.setBackground(currentBg);
        tile.setText(sb.toString());
      }
    }
  }

  private void changeFgColor(Plot val) {
    // FIXME find a better protocol for this using more colours
    switch (val.chr.getChr()) {
    case 0:
      currentFgc = Color.BLACK;
      break;
    case 1:
      currentFgc = Color.RED;
      break;
    case 2:
      currentFgc = Color.GREEN;
      break;
    case 3:
      currentFgc = Color.YELLOW;
      break;
    case 4:
      currentFgc = Color.DODGERBLUE;
      break;
    case 5:
      currentFgc = Color.MAGENTA;
      break;
    case 6:
      currentFgc = Color.CYAN;
      break;
    case 7:
      currentFgc = GRID_FGC;
      break;
    default:
      break;
    }
  }

  private void changeBgColor(Plot val) {
    // FIXME find a better protocol for this using more colours
    switch (val.chr.getChr()) {
    case 0:
      setCurrentBg(Color.BLACK);
      break;
    case 1:
      setCurrentBg(Color.RED);
      break;
    case 2:
      setCurrentBg(Color.GREEN);
      break;
    case 3:
      setCurrentBg(Color.YELLOW);
      break;
    case 4:
      setCurrentBg(Color.DODGERBLUE);
      break;
    case 5:
      setCurrentBg(Color.MAGENTA);
      break;
    case 6:
      setCurrentBg(Color.CYAN);
      break;
    case 7:
      setCurrentBg(GRID_BGC);
      break;
    default:
      break;
    }
  }

  void drawCursor() { // FIXME not accurate when gridlines hidden
    eraseCursor();
    cursorGc.setFill(CURSOR_CLR);
    cursorGc.fillRect(((col - 1) * fontWidth) + 1,
        ((row - 1) * fontHeight) - 0, fontWidth - 1, fontHeight - 1);
    prevCol = col;
    prevRow = row;
  }

  void eraseCursor() {
    cursorGc.clearRect(((prevCol - 1) * fontWidth) + 1,
        ((prevRow - 1) * fontHeight) - 0, fontWidth, fontHeight);
  }

  private void showGridlines() {
    for (int i = 0; i < cols / 2; i++) {
      for (int j = 0; j < rows; j++) {
        tiles[i][j].setBorder(b);
      }
    }
    recalculateFontBounds();
  }

  private void hideGridlines() {
    for (int i = 0; i < cols / 2; i++) {
      for (int j = 0; j < rows; j++) {
        tiles[i][j].setBorder(null);
      }
    }
    recalculateFontBounds();
  }

  private int tileCol(int col) {
    return (col - 1) / 2;
  }

  private int tileRow(int row) {
    return row - 1;
  }

  private void writeToQueue(String s) {
    if (s == null) {
      return;
    }
    for (int i = 0; i < s.length(); i++) {
      writeToQueue(s.charAt(i));
    }
  }

  private void writeToQueue(char c) {
    Platform.runLater(() -> {
      String sc = "" + c; // FIXME JVM might not be using UTF-8
      byte[] bytes = sc.getBytes(Charset.forName("UTF-8"));
      byte b;
      for (int i = 0; i < bytes.length; i++) {
        b = bytes[i];
        rightPiper.send(b);
      }
    });
  }

  private void clearScreen() {
    for (int i = 0; i < cols / 2; i++) {
      for (int j = 0; j < rows; j++) {
        tiles[i][j].setText("  ");
        tiles[i][j].setBackground(GRID_BG);
      }
    }
  }

  // This instantiates an FVM
  private void connectToNewFVM() {
    LeftRightPiper lrp = new QueueLeftRightPiper();
    leftPiper = lrp.getLeft();
    rightPiper = lrp.getRight();
    FVM fvm = new FVM(leftPiper);
    new Thread(fvm).start();
  }

  // This executes an ordinary native FVM process
  private void connectToNewNativeFVM() {
    try {
      rightPiper = new NativePiper("./fvm"); // FIXME POSIX dependency
    } catch (IOException e) {
      System.out.println(e.getMessage());
    }
  }

  // Hint: You can use the ClientEnsock utility to start the FVM process
  private void connectToClientSocketFVM() throws IOException {
    rightPiper = new ServerSocketPiper(1234); // FIXME hardcoded
  }

  // Hint: You can use the ServerEnsock utility to start the FVM process
  private void connectToServerSocketFVM() throws IOException {
    rightPiper = new ClientSocketPiper("localhost", 1234); // FIXME hardcoded
  }

  // Hint: Use the Encircle utility to start this Grid and an FVM process
  private void connectToStdinAndStdout() throws IOException {
    rightPiper = new StreamPiper(System.in, System.out);
  }
  
  private void connect() {
    try {
      // Only uncomment 1 of the following lines
      connectToNewFVM();
      // connectToNewNativeFVM();
      // connectToClientSocketFVM();
      // connectToServerSocketFVM();
      // connectToStdinAndStdout();
    } catch (Exception e) {
      System.err.println(e.getMessage());
    }
  }

  public static void main(String[] args) throws IOException {
    Grid grid = new Grid();
    launch(args);
  }
}
