/*
                        FREEPUTER VIRTUAL MACHINE

Program:    FVM.java
Copyright © Robert Gollagher 2015
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20150906
Updated:    20150916:1241
Version:    0.1.0.2 alpha for FVM 1.0

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
[at your option] any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details. 

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

                              This Edition:
                                  Java
                     for Java 6 / Java 7 / Java 8

                               ( ) [ ] { }

==============================================================================

WARNING: This is alpha software. It is somewhat incomplete and
relatively unreliable. For a more complete and more reliable implementation
of an FVM 1.0 please use the x86 assembly language implementation
found in 'dev/fvm/x86/src/fvm.s'. That is the reference implementation.
Where the behaviour of this implementation differs from that of
the reference implementation, the behaviour of the reference implementation
(unless obviously a bug) should be taken to be the correct behaviour.

WARNING: Due to constraints on development time, the arithmetic operations
in this implementation have not yet been fully tested and are probably 
at least partially incorrect. Specifically, this implementation contains
algorithms intended to replicate the behaviour of the x86 instruction set
with respect to detecting integer overflow; those algorithms have not
been extensively tested and are probably incorrect in some cases.

WARNING: The performance of this Java implementation is relatively poor.
It is generally about 5 times slower than the reference implementation.
Furthermore, the size of a Java FVM implementation (including the size of
the underlying JRE) is typically measured in megabytes whereas the size of
the reference implementation is measured in kilogytes. In other words,
a Java implementation might well be 5 times slower and 1000 times larger
than the reference implementation, not to mention have far greater
runtime memory requirements. Nevertheless it is sometimes convenient and
appropriate to use a Java FVM implementation such as this one.
Note: a portable C FVM implementation is also available,
see 'dev/fvm/c/src/fvm.c'.

IMPORTANT NOTE: Don't forget that you may have to increase the amount of
memory allocated to your JRE depending on how many FVM instances you wish
to instantiate and how much system memory each FVM instance uses.
For example, increase JRE heap size with -Xms and -Xmx.
See http://docs.oracle.com/javase/8/docs/technotes/tools/unix/java.html

PLATFORMS: This 'FVM.java' code is known to run on the following platforms.
It should also run on numerous devices that support Java 6 or higher.

  (1) Java 6 (OpenJDK) on 32-bit x86 Linux (Debian 7.8)
      running on Intel i5 CPU (typical desktop computer)
  (2) Java 8 (Oracle)  on 32-bit x86 Linux (Debian 7.8)
      running on Intel i5 CPU (typical desktop computer)
  (3) Java 8 (Oracle)  on 32-bit ARM Linux (Raspbian GNU/Linux 7)
      running on ARM11 CPU (Raspberry Pi Rev2 Model B, 512MB RAM)
  (4) Java 8 (Oracle)  on 64-bit Windows 8
      running on Microsoft Surface Pro 3 (tablet)

=========================================================================== */

package com.freeputer;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;

/**
 * <p>
 * Class <code>FVM</code> implements the Freeputer Virtual Machine 1.0.
 * </p>
 * 
 * <h2>WARNING</h2>
 * <p>
 * This is alpha software (see source code for details).
 * </p>
 * 
 * <h2>INTRODUCTION</h2>
 * 
 * <p>
 * The Freeputer Virtual Machine (FVM) is primarily intended to be
 * implemented in assembly language. It is not hard to do so; indeed it is
 * much easier (for an expert in assembly language for a target platform)
 * than using C. This is mainly because C has undefined behaviour and
 * does not expose useful CPU status flags. The reference implementation
 * of the FVM 1.0 is written in x86 assembly language for 32-bit Linux.
 * See 'dev/fvm/x86/src/fvm.s'. It is stable betaware.
 * </p>
 * 
 * <p>
 * There is now also a portable C implementation of the FVM 1.0.
 * See 'dev/fvm/c/src/fvm.c'. It is a little larger and somewhat slower than the
 * reference implementation but is portable and should run on most
 * devices that gcc can target. It is alpha software.
 * </p>
 * 
 * <p>
 * A Java FVM implementation should <i>not</i> be your first resort. This Java
 * implementation is about 5 times slower than the reference implementation
 * and might well be 100 or 1000 times larger (that is, including the JRE).
 * On the other hand, this Java implementation is portable and should run on
 * numerous devices that support Java 6 or higher. Also, you might choose
 * to use this Java implementation if you wish to embed Freeputer
 * software modules in a larger Java application (rather than
 * communicating with them remotely as distributed services).
 * </p>
 * 
 * <p>
 * Before using this Java FVM implementation you should read the
 * 'README.html' file in the Freeputer project root directory for a comprehesive
 * introduction to the Freeputer platform. You should do the Quick Start
 * tutorial therein using the reference implementation.
 * </p>
 * 
 * <h2>USAGE IN JAVA</h2>
 * 
 * <p>
 * The examples below show a few of the many ways that FVM instances
 * can be created and run. They correspond to
 * typical current use of the reference implementation.
 * </p>
 * <p>
 * This implementation does not provide all possible constructors.
 * For example, it does not provide a constructor to
 * configure an FVM instance to hard reset (rather than soft reset or halt)
 * when a trap occurs; however, you could very easily achieve that
 * by adding your own constructor and/or slightly modifying the
 * "SYSTEM RESET" section of this class.
 * </p>
 * 
 * <h3>To create an FVM instance:</h3>
 *
 * <pre><code>
 * // Create a small FVM with 16 kB ROM, 16 kB RAM and a zero-sized stdblk 
 * FVM fvm16_0kB = new FVM();
 *
 * // Create an FVM with 16 MB ROM, 16 MB RAM and a zero-sized stdblk
 * FVM fvm16_0MB = new FVM(0x01000000,0);
 *
 * // Create an FVM with 16 MB ROM, 16 MB RAM and 16 MB stdblk
 * FVM fvm16_16MB = new FVM(0x01000000,0x01000000);
 *
 * // Create an FVM sized and configured to run the fvmtest suite.
 * // It will have 16 MB ROM, 16 MB RAM, 16 MB stdblk, will append to (rather
 * // than truncating) stdtrc when it restarts and it will soft reset
 * // rather than halt when a trap occurs.
 * FVM fvm16_16MB-sr-append = FVM.getFvmtestInstance(); 
 * </code></pre>
 * 
 * <h3>To run FVM instances synchronously:</h3>
 *
 * <pre><code>
 * // Create 4 identified FVM instances 
 * FVM fvm1 = new FVM(1);
 * FVM fvm2 = new FVM(2);
 * FVM fvm3 = new FVM(3);
 * FVM fvm4 = new FVM(4);
 *
 * // Run them sequentially:
 * fvm1.run();
 * fvm1.getSystemExitCode(); // 0 indicates success
 * 
 * fvm2.run();
 * fvm2.getSystemExitCode(); // 0 indicates success
 * 
 * fvm3.run();
 * fvm3.getSystemExitCode(); // 0 indicates success 
 * 
 * fvm4.run();
 * fvm4.getSystemExitCode(); // 0 indicates success
 * </code></pre>
 * 
 * <p>
 * <b>Important:</b>
 * For the runs to succeed the following local files must exist:</p>
 * 
 * <pre><code>
 * rom.fp1   rom.fp2    rom.fp3    rom.fp4 
 * std.blk1  std.blk2   std.blk3   std.blk4 
 * std.imp1  std.imp2   std.imp3   std.imp4 
 * </code></pre>
 * <p>
 * Each 'rom.fp' file must contain the compiled Freelang program
 * which the respective FVM instance shall run. Each 'std.blk'
 * file represents a standard block device (here an empty file).  Each
 * 'std.imp' file represents a standard import device (here an empty file).
 * See <b>DEPLOYMENT</b> below.
 * </p>
 * 
 * <h3>To run FVM instances asynchronously:</h3>
 *
 * <pre><code>
 * // Create 4 identified FVM instances
 * FVM fvm1 = new FVM(1);
 * FVM fvm2 = new FVM(2);
 * FVM fvm3 = new FVM(3);
 * FVM fvm4 = new FVM(4);
 * 
 * // Run them concurrently:
 * new Thread(fvm1).start();
 * new Thread(fvm2).start();    
 * new Thread(fvm3).start();
 * new Thread(fvm4).start();
 *
 * // Determine if runs succeeded
 * fvm1.getSystemExitCode(); // null = still running, 0 = success
 * fvm2.getSystemExitCode(); // null = still running, 0 = success
 * fvm3.getSystemExitCode(); // null = still running, 0 = success
 * fvm4.getSystemExitCode(); // null = still running, 0 = success
 * </code></pre>
 * 
 * <p><b>Important note:</b>
 * This FVM implementation simply uses <code>System.in</code>
 * and <code>System.out</code> for Freeputer stdin and stdout. This is
 * <i>not appropriate</i> for running co-located FVM instances concurrently
 * <i>unless</i> you intend them to asynchronously share those streams.
 * </p>
 * 
 * <h2>DEPLOYMENT</h2>
 * 
 * <p>
 * To successfully run an FVM instance the following must exist:
 * </p>
 * <ul>
 * <li>a 'rom.fp' file containing the compiled Freelang program to be run</li>
 * <li>a 'std.blk' file of the correct size for that instance (can be empty)</li>
 * <li>a 'std.imp' file (can be empty)</li>
 * </ul>
 *
 * <p>
 * If those files are 'foo/rom.fp',
 * 'foo/std.blk' and 'foo/std.imp' and you have compiled this 'FVM.java' file to
 * 'foo/com/freeputer/FVM.class' then you might run your FVM instance from the
 * 'foo' directory by: <code>java com.freeputer.FVM</code>
 * </p>
 *
 * <p>
 * If you specify an <code>fvmID</code> (when you instantiate your FVM instance)
 * then those files must each be suffixed accordingly (e.g.
 * 'rom.fp1', 'std.blk1' and 'std.imp1').
 * </p>
 * 
 * <h2>FURTHER INFORMATION</h2>
 * 
 * <p>
 * For further information please see:
 * </p>
 * <ul>
 * <li>the source code of this 'FVM.java' class ('dev/fvm/java/src/com/freeputer/FVM.java')</li>
 * <li>the extensive comments in the reference implementation ('dev/fvm/x86/src/fvm.s')</li>
 * <li>the extensive comments in the C implementation ('dev/fvm/c/src/com/freeputer/fvm.c')</li>
 * <li>the 'README.html' in the Freeputer project root directory</li>
 * </ul>
 * 
 * @author Robert Gollagher
 * @version 0.1.0.2 alpha for FVM 1.0
 *
 */
public class FVM implements Runnable {

  public static final String version = "fvm java version 0.1.0.2 alpha for FVM 1.0";
  
  // Change TRON_ENABLED to false if you do not require your FVM instance
  // to support tracing at runtime; the FVM will then be smaller and faster.
  private static final boolean TRON_ENABLED = true;
	  
  private final Integer fvmID;

  /**
   * <p>Gets the fvmID of this FVM instance.</p>
   * 
   * <p>Note: the fvmID is not part of the implied FVM 1.0 platform standard.
   * It is simply a convenient means by which this Java implementation
   * can instantiate multiple FVM instances and identify the system
   * resources associated with each instance.</p>
   * 
   * @return the fvmID this FVM instance was instantiated with
   */
  public Integer getFvmID() {
    return fvmID;
  }

  /**
   * Constructs a small unidentified FVM instance with no stdblk capacity.
   *
   * <pre>
   * Constructs a small VM instance having:
   *   16 kB RAM, 16 kB ROM, 0-sized stdblk, no memory-mapped devices.
   * Typically designated as:
   *   fvm16_0kB
   * </pre>
   */
  public FVM() {
    this(0x4000, 0x4000, null, false, false);
  }
  
  /**
   * Constructs a small identified FVM instance with no stdblk capacity.
   *
   * <pre>
   * Constructs a small VM instance having:
   *   16 kB RAM, 16 kB ROM, 0-sized stdblk, no memory-mapped devices.
   * Typically designated as:
   *   fvm16_0kB
   * </pre>
   * 
   * @param fvmID
   *            the ID for this FVM instance (or null if unidentified)
   */
  public FVM(int fvmID) {
    this(0x4000, 0x4000, fvmID, false, false);
  }

  /**
   * Constructs an unidentified FVM instance with the specified sizing and
   * having no memory-mapped devices.
   * 
   * @param arbitraryMemorySize
   *            size of ROM in bytes and size of RAM in bytes
   * @param stdblkSize
   *            size of stdblk in bytes (the 'std.blk' file)
   * @throws IllegalArgumentException
   *            if arbitraryMemorySize is 0 or not a power of 2 or not an even multiple of word size;
   *            if stdblk size is not either 0 or a power of 2 and an even multiple of word size.
   */
  public FVM(int arbitraryMemorySize, int stdblkSize) {
    this(arbitraryMemorySize, stdblkSize, null, false, false);
  }

  /**
   * Constructs an FVM instance with the specified sizing and having no
   * memory-mapped devices.
   * 
   * @param arbitraryMemorySize
   *            size of ROM in bytes and size of RAM in bytes
   * @param stdblkSize
   *            size of stdblk in bytes (the 'std.blk' file)
   * @param fvmID
   *            the ID for this FVM instance (or null if unidentified)
   * @throws IllegalArgumentException
   *            if arbitraryMemorySize is 0 or not a power of 2 or not an even multiple of word size;
   *            if stdblk size is not either 0 or a power of 2 and an even multiple of word size.
   */
  public FVM(int arbitraryMemorySize, int stdblkSize, Integer fvmID) {
    this(arbitraryMemorySize, stdblkSize, fvmID, false, false);
  }

  /**
   * Convenience method for instantiating an FVM correctly sized and
   * configured to run the fvmtest suite.
   * 
   * @return an FVM ready to use to run the fvmtest suite.
   */
  public static final FVM getFvmtestInstance() {
    return new FVM(0x01000000, 0x01000000, null, true, true);
  }

  /**
   * Constructs an FVM instance with the specified sizing and configuration
   * and having no memory-mapped devices.
   * 
   * @param arbitraryMemorySize
   *            size of ROM in bytes and size of RAM in bytes
   * @param stdblkSize
   *            size of stdblk in bytes (the 'std.blk' file)
   * @param fvmID
   *            the ID for this FVM instance (or null if unidentified)
   * @param appendStdtrc
   *            if true, stdtrc will append not truncate upon FVM restart
   * @param softResetOnTrap
   *            if true, the FVM will soft reset instead of halt upon a trap
   * @throws IllegalArgumentException
   *            if arbitraryMemorySize is 0 or not a power of 2 or not an even multiple of word size;
   *            if stdblk size is not either 0 or a power of 2 and an even multiple of word size.
   */
  public FVM(int arbitraryMemorySize, int stdblkSize, Integer fvmID,
      boolean appendStdtrc, boolean softResetOnTrap)
      throws IllegalArgumentException {

    this.fvmID = fvmID;
    this.appendStdtrc = appendStdtrc;
    this.softResetOnTrap = softResetOnTrap;
    
    
    // To conform to the conventions of the reference implementation
    // the values of ARBITRARY_MEMORY_SIZE and STDBLK_SIZE *should* be
    // powers of 2 (easy for other implementations to support)
    // and *must* be even multiples of WORD_SIZE.
    // Note: STDBLK_SIZE can also be 0.

    
    // Validate specified arbitraryMemorySize: -------------------------------
    if (arbitraryMemorySize == 0) { // FIXME test and refactor this
    	throw new IllegalArgumentException(
   	         "arbitraryMemorySize must be > 0");
    } else {
    	if ((arbitraryMemorySize>=WORD_SIZE) // must be at least WORD_SIZE
    		&& ((arbitraryMemorySize % WORD_SIZE) == 0) // and a multiple of WORD_SIZE
    		&& ((arbitraryMemorySize & (arbitraryMemorySize-1)) == 0)) { // and a power of 2
    		  // Specified arbitraryMemorySize is valid so do nothing here
    	} else {
    	      throw new IllegalArgumentException(
    	         "arbitraryMemorySize must be > 0 and a multiple of 4 and a power of 2");    		
    	}
    } // --------------------------------------------------------------------- 
    
    // Validate specified stdblkSize: ----------------------------------------
    if (stdblkSize == 0) { // FIXME test and refactor this
      STDBLK_SIZE = 0; // stdblk size can legally be 0
    } else {
    	if ((stdblkSize>=WORD_SIZE) // must be at least WORD_SIZE
    		&& ((stdblkSize % WORD_SIZE) == 0) // and a multiple of WORD_SIZE
    		&& ((stdblkSize & (stdblkSize-1)) == 0)) { // and a power of 2
    		  STDBLK_SIZE = stdblkSize; // Specified stdblkSize is valid
    	} else {
    	      throw new IllegalArgumentException(
    	         "stdblkSize must be 0 or a multiple of 4 and a power of 2");    		
    	}
    } // ---------------------------------------------------------------------

    // This VM implementation happens to make ROM and RAM the same size
    // as each other but that is not at all mandatory. Furthermore,
    // STDBLK_SIZE does not have to be related to these sizes
    // but must be a word-multiple or zero.
    ROM_SIZE = arbitraryMemorySize;// ROM, RAM, MAP must be word-multiples
    RAM_SIZE = arbitraryMemorySize;

    LOWEST_WRITABLE_BYTE = ROM_SIZE; // RAM immediately follows ROM
    HIGHEST_WRITABLE_BYTE = ROM_SIZE + RAM_SIZE + MAP_SIZE - 1;
    HIGHEST_WRITABLE_WORD = ROM_SIZE + RAM_SIZE + MAP_SIZE - WORD_SIZE;

    // System memory (power of 2)
    memory = ByteBuffer.allocate(ROM_SIZE + RAM_SIZE + MAP_SIZE);

    final String idSuffix;
    if (fvmID == null) {
      idSuffix = "";
    } else {
      idSuffix = "" + fvmID;
    }
    stdblkFilename = "std.blk" + idSuffix;
    romFilename = "rom.fp" + idSuffix;
    stdtrcFilename = "std.trc" + idSuffix;
    stdexpFilename = "std.exp" + idSuffix;
    stdimpFilename = "std.imp" + idSuffix;

    // The FVM is little endian
    memory.order(ByteOrder.LITTLE_ENDIAN);
    readBuf.order(ByteOrder.LITTLE_ENDIAN);
    writeBuf.order(ByteOrder.LITTLE_ENDIAN);
    getBuf.order(ByteOrder.LITTLE_ENDIAN);
    putBuf.order(ByteOrder.LITTLE_ENDIAN);
    readbBuf.order(ByteOrder.LITTLE_ENDIAN);
    writebBuf.order(ByteOrder.LITTLE_ENDIAN);
    getbBuf.order(ByteOrder.LITTLE_ENDIAN);
    putbBuf.order(ByteOrder.LITTLE_ENDIAN);
  }

  private boolean keepRunning = true;
  private Integer systemExitCode = null;
  private final boolean appendStdtrc;
  private final boolean softResetOnTrap;

  private static final int NEG_INT_MAX = -2147483648;
  private static final int POS_INT_MAX = 2147483647;

  private final int STDBLK_SIZE; // bytes (word-multiple, power of 2)
  
  private final int ROM_SIZE; // bytes (ROM, RAM, MAP must be word-multiples)
  private final int RAM_SIZE;
  // This VM implementation does not provide any memory-mapped device,
  // therefore we set MAP_SIZE to 0 here
  private final int MAP_SIZE = 0; // MAP immediately follows RAM
  private final int LOWEST_WRITABLE_BYTE; // RAM immediately follows ROM
  private final int HIGHEST_WRITABLE_BYTE;
  private final int HIGHEST_WRITABLE_WORD;

  private static final int WORD_SIZE = 4; // bytes
  private static final int TWO_WORDS_SIZE = 8; // bytes
  private static final int MAX_DEPTH_DS = 32; // elements (words) (power of 2)
  private static final int MAX_DEPTH_RS = 32; // elements (words) (power of 2)
  private static final int MAX_DEPTH_SS = 32; // elements (words) (power of 2)
  private static final int STDBLK = 0;
  private static final int STDIN = 1;
  private static final int STDIMP = 2;
  private static final int STDOUT = -1;
  private static final int STDEXP = -2;
  private static final int FALSE = 0;
  private static final int TRUE = 1;
  
  // Indicates program-requested RESET
  private final int LAST_RESTART_CODE_RESET = 1;
  // Indicates program-requested REBOOT
  private final int LAST_RESTART_CODE_REBOOT = 2;
  
  private final String stdblkFilename; // Name of file for standard block
  private final String romFilename; // Name of file for system ROM
  private final String stdtrcFilename; // Name of file for standard trace
  private final String stdexpFilename; // Name of file for standard export
  private final String stdimpFilename; // Name of file for standard import
  
  // Error messages for traps (these go to stdtrc)
  private static final String msgTrapIllegalOpcode = "ILLEGAL OPCODE    ";
  private static final String msgTrapMathOverflow = "MATH OVERFLOW     ";
  private static final String msgTrapDsOverflow = "DS OVERFLOW       ";
  private static final String msgTrapDsUnderflow = "DS UNDERFLOW      ";
  private static final String msgTrapRsOverflow = "RS OVERFLOW       ";
  private static final String msgTrapRsUnderflow = "RS UNDERFLOW      ";
  private static final String msgTrapSsOverflow = "SS OVERFLOW       ";
  private static final String msgTrapSsUnderflow = "SS UNDERFLOW      ";
  private static final String msgTrapXsBitshift = "XS BITSHIFT       ";
  private static final String msgTrapMemBounds = "BEYOND MEM BOUNDS ";
  private static final String msgTrapRAMBounds = "BEYOND RAM BOUNDS ";
  private static final String msgTrapCantOpenStdblk = "CAN'T OPEN STDBLK ";
  private static final String msgTrapCantCloseStdblk = "CAN'T CLOSE STDBLK";
  private static final String msgTrapCantOpenRom = "CAN'T OPEN ROM    ";
  private static final String msgTrapCantCloseRom = "CAN'T CLOSE ROM   ";
  private static final String msgTrapCantReadRom = "CAN'T READ ROM    ";
  private static final String msgTrapCantOpenStdexp = "CAN'T OPEN STDEXP ";
  private static final String msgTrapCantCloseStdexp = "CAN'T CLOSE STDEXP";
  private static final String msgTrapCantOpenStdimp = "CAN'T OPEN STDIMP ";
  private static final String msgTrapCantCloseStdimp = "CAN'T CLOSE STDIMP";
  private static final String msgTrapDivideByZero = "DIVIDE BY ZERO    ";
  private static final String msgTrapWall = "HIT WALL          ";
  private static final String msgTrapData = "HIT DATA          ";
  private static final String msgTrapPcOverflow = "PC OVERFLOW       ";
  private static final String msgBefore = " just before:     ";

  // =========================================================================
  // VARIABLES
  // =========================================================================
  private int rchannel; // Channel for READOR
  private int wchannel; // Channel for WRITOR
  private int gchannel; // Channel for GETOR
  private int pchannel; // Channel for PUTOR
  private ByteBuffer readBuf = ByteBuffer.allocate(WORD_SIZE);
  private ByteBuffer writeBuf = ByteBuffer.allocate(WORD_SIZE);
  private ByteBuffer getBuf = ByteBuffer.allocate(WORD_SIZE);
  private ByteBuffer putBuf = ByteBuffer.allocate(WORD_SIZE);
  private ByteBuffer readbBuf = ByteBuffer.allocate(1);
  private ByteBuffer writebBuf = ByteBuffer.allocate(1);
  private ByteBuffer getbBuf = ByteBuffer.allocate(1);
  private ByteBuffer putbBuf = ByteBuffer.allocate(1);
  private FileChannel stdblkChannel; // File handle for stdblk file
  private FileInputStream romStream; // File handle for ROM file
  private PrintStream stdtrcChannel; // File handle for stdtrc file
  private FileChannel stdexpChannel; // File handle for stdexp file
  private FileChannel stdimpChannel; // File handle for stdimp file

  // Java version declares these explicitly
  private InputStream stdinStream; // File handle for stdinp file
  private OutputStream stdoutStream; // File handle for stdout file

  private int rsp; // Return stack pointer
  private int rsStop = MAX_DEPTH_RS; // rs index that bookends its start
  private int[] rs = new int[rsStop + 1]; // Return stack plus one word of
  private int RS_EMPTY = rsStop;
  private int RS_HAS_ONE = rsStop - 1;
  private int RS_FULL = 0;

  private int ssp; // Software stack pointer
  private int ssStop = MAX_DEPTH_SS; // ss index that bookends its start
  private int[] ss = new int[ssStop + 1]; // Software stack plus one word of
  private int SS_EMPTY = ssStop;
  private int SS_HAS_ONE = ssStop - 1;
  private int SS_HAS_TWO = ssStop - 2;
  private int SS_HAS_THREE = ssStop - 3;
  private int SS_HAS_FOUR = ssStop - 4;
  private int SS_FULL = 0;
  private int SS_TWO_LESS_FULL = 2;
  private int SS_THREE_LESS_FULL = 3;
  private int SS_FOUR_LESS_FULL = 4;

  private int dsp; // Data stack pointer
  private int dsStop = MAX_DEPTH_DS; // ds index that bookends its start
  private int[] ds = new int[dsStop + 1]; // Data stack plus one word of empty
  private ByteBuffer memory; // System memory (power of 2)
  private int DS_EMPTY = dsStop;
  private int DS_HAS_ONE = dsStop - 1;
  private int DS_HAS_TWO = dsStop - 2;
  private int DS_HAS_THREE = dsStop - 3;
  private int DS_HAS_FOUR = dsStop - 4;
  private int DS_FULL = 0;
  private int DS_TWO_LESS_FULL = 2;
  private int DS_THREE_LESS_FULL = 3;
  private int DS_FOUR_LESS_FULL = 4;

  //Last automated exit code (if any).
  //  Not preserved after a hard reset.
  private int lastExitCode;
  //Program-requested restart code (if any).
  //  Preserved even after a hard reset.
  private int lastRestartCode; 

  // =========================================================================
  // FOR TRACING (optional for production VM)
  // =========================================================================
  int vmFlags = 0; // Flags -------1 = trace on

  private static String[] traceTable;
  static {
    if (TRON_ENABLED) {
      traceTable = new String[] { // Must be in same order as opcodeTable
          "===     ",
          "lit     ",
          "call    ",
          "go      ",
          "go[>0]  ",
          "go[>=0] ",
          "go[==0] ",
          "go[!=0] ",
          "go[<=0] ",
          "go[<0]  ",
          "go[>]   ",
          "go[>=]  ",
          "go[==]  ",
          "go[!=]  ",
          "go[<=]  ",
          "go[<]   ",
          "go>0    ",
          "go>=0   ",
          "go==0   ",
          "go!=0   ",
          "go<=0   ",
          "go<0    ",
          "go>     ",
          "go>=    ",
          "go==    ",
          "go!=    ",
          "go<=    ",
          "go<     ",
          "reador  ",
          "writor  ",
          "tracor  ",
          "getor   ",
          "putor   ",
          "readorb ",
          "writorb ",
          "tracorb ",
          "getorb  ",
          "putorb  ",
          "        ", // Start of 107 empty fillers in blocks of 10...
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ", // 10 empty fillers
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ", // 10 empty fillers
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ", // 10 empty fillers
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ", // 10 empty fillers
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ", // 10 empty fillers
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ", // 10 empty fillers
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ", // 10 empty fillers
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ", // 10 empty fillers
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ", // 10 empty fillers
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ", // 7 empty fillers
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ", // End of 107 empty fillers in blocks of 10
          "ret     ",
          "invoke  ",
          "[invoke]",
          "fly     ",
          "swap    ",
          "over    ",
          "rot     ",
          "tor     ",
          "leap    ",
          "nip     ",
          "tuck    ",
          "rev     ",
          "rpush   ",
          "rpop    ",
          "drop    ",
          "drop2   ",
          "drop3   ",
          "drop4   ",
          "dup     ",
          "dup2    ",
          "dup3    ",
          "dup4    ",
          "hold    ",
          "hold2   ",
          "hold3   ",
          "hold4   ",
          "speek   ",
          "speek2  ",
          "speek3  ",
          "speek4  ",
          "spush   ",
          "spush2  ",
          "spush3  ",
          "spush4  ",
          "spop    ",
          "spop2   ",
          "spop3   ",
          "spop4   ",
          "dec     ",
          "decw    ",
          "dec2w   ",
          "inc     ",
          "incw    ",
          "inc2w   ",
          "@       ",
          "!       ",
          "[@]     ",
          "@b      ",
          "!b      ",
          "[@b]    ",
          "@@      ",
          "@!      ",
          "[@@]    ",
          "@@b     ",
          "@!b     ",
          "[@@b]   ",
          "+       ",
          "-       ",
          "*       ",
          "/       ",
          "%       ",
          "/%      ",
          "[+]     ",
          "[-]     ",
          "[*]     ",
          "[/]     ",
          "[%]     ",
          "[/%]    ",
          "neg     ",
          "abs     ",
          "&       ",
          "|       ",
          "^       ",
          "[&]     ",
          "[|]     ",
          "[^]     ",
          "<<      ",
          ">>      ",
          "[<<]    ",
          "[>>]    ",
          "move    ",
          "fill    ",
          "find    ",
          "match   ",
          "moveb   ",
          "fillb   ",
          "findb   ",
          "matchb  ",
          "homio   ",
          "rchan   ",
          "wchan   ",
          "gchan   ",
          "pchan   ",
          "ecode?  ",
          "rcode?  ",
          "rom?    ",
          "ram?    ",
          "map?    ",
          "stdblk? ",
          "ds?     ",
          "ss?     ",
          "rs?     ",
          "dsn?    ",
          "ssn?    ",
          "rsn?    ",
          "tron    ",
          "troff   ",
          "reset   ",
          "reboot  ",
          "halt    ",
          "data    "};
    } // end if (TRON_ENABLED)
  }; // end of static block
  
  // Trace if trace flag set in vmFlags
  private final void optTrace() {
    if (TRON_ENABLED) {
      if ((vmFlags & 0x00000001) == 1) {
        traceInfo();
        traceStacks();
      }
    }
  }

  // =========================================================================
  // REGISTERS
  // =========================================================================
  
  // Note: The FVM is a stack machine. It has no exposed registers.
  // The registers referred to below are physical or virtual registers of
  // the target architecture for which this FVM implementation is to compiled.
  int pc = 0; // Program counter (was %esi in original x86 implementation) in bytes
  int rA = 0; // Register A (was %eax)
  int rB = 0; // Register B (was %ebx)
  int rC = 0; // Register C (was %ecx)
  int rD = 0; // Register D (was %edx)
  int rE = 0; // Register E (was %esi)
  int rF = 0; // Register F (was %edi)

  // =========================================================================
  // "MACROS" converted to private methods
  // =========================================================================

  // Increments the program counter
  private final void incPc() {
    pc = pc + WORD_SIZE;
    if ((pc < 0) || (pc > HIGHEST_WRITABLE_WORD)) {
      trapPcOverflow();
    }
  }

  // Puts word at current program cell into rA
  private final void wordAtPc() {
    rA = memory.getInt(pc);
  }

  // Branches to address in the next program cell
  // Note: the check below ensures branch address lies within program memory
  private final void branch() {
    pc = memory.getInt(pc);
    if ((pc < 0) || (pc > HIGHEST_WRITABLE_WORD)) {
      trapPcOverflow();
    }
  }

  // Skips the next program cell (rather than branching)
  private final void dontBranch() {
    incPc();
  }

  // Reset to using default IO channels
  private final void setIOdefaults() {
    gchannel = 0;
    pchannel = 0;
    rchannel = 1;
    wchannel = -1;
  }

  // Pushes value in rA onto the return stack
  private final void pushRs() {
    if (rsp <= RS_FULL) {
      trapRsOverflow();
    }
    rsp--;
    rs[rsp] = rA;
  }

  // Pushes value in rA onto the return stack
  private final void pushSs() {
    if (ssp <= SS_FULL) {
      trapSsOverflow();
    }
    ssp--;
    ss[ssp] = rA;
  }

  // Pushes value in rA onto the data stack
  private final void pushDs() {
    if (dsp <= DS_FULL) {
      trapDsOverflow();
    }
    dsp--;
    ds[dsp] = rA;
  }

  // Peeks at the top of the data stack into rA
  private final void peekDs() {
    if (dsp > DS_HAS_ONE) {
      trapDsUnderflow();
    }
    rA = ds[dsp];
  }

  // Peeks at the second element from the top of the data stack into rB
  private final void peekSecondDs() {
    if (dsp > DS_HAS_TWO) {
      trapDsUnderflow();
    }
    rB = ds[dsp + 1];
  }

  // Peeks at the third element from the top of the data stack into rC
  private final void peekThirdDs() {
    if (dsp > DS_HAS_THREE) {
      trapDsUnderflow();
    }
    rC = ds[dsp + 2];
  }

  // Peeks twice from the data stack, first into rA and second into rB.
  // WARNING: THIS IS INTENTIONALLY OPPOSITE ORDER TO twoPopDS.
  private final void twoPeekDs() {
    if (dsp > DS_HAS_TWO) {
      trapDsUnderflow();
    }
    rA = ds[dsp];
    rB = ds[dsp + 1];
  }

  // Peeks thrice from the data stack, first into rA, second into rB
  // and third into rC.
  // WARNING: THIS IS INTENTIONALLY OPPOSITE ORDER TO threePopDS.
  private final void threePeekDs() {
    if (dsp > DS_HAS_THREE) {
      trapDsUnderflow();
    }
    rA = ds[dsp];
    rB = ds[dsp + 1];
    rC = ds[dsp + 2];
  }

  // Peeks four times from the data stack, first into rA, second into rB,
  // third into rC and fourth into rD.
  // WARNING: THIS IS INTENTIONALLY OPPOSITE ORDER TO fourPopDS.
  private final void fourPeekDs() {
    if (dsp > DS_HAS_FOUR) {
      trapDsUnderflow();
    }
    rA = ds[dsp];
    rB = ds[dsp + 1];
    rC = ds[dsp + 2];
    rD = ds[dsp + 3];
  }

  // Replaces the value on top of the data stack with value in %eax
  private final void replaceDs() {
    if (dsp > DS_HAS_ONE) {
      trapDsUnderflow();
    }
    ds[dsp] = rA;
  }

  // Replaces the value on top of the data stack with value in rA
  // and second-top value on the data stack with value in rB
  // WARNING: THIS IS INTENTIONALLY OPPOSITE ORDER TO twoPopDS.
  private final void twoReplaceDs() {
    if (dsp > DS_HAS_TWO) {
      trapDsUnderflow();
    }
    ds[dsp] = rA;
    ds[dsp + 1] = rB;
  }

  // Pushes first rB and second rA onto the data stack
  // WARNING: THIS IS INTENTIONALLY SAME ORDER TO twoPopSS.
  private final void twoPushDs() {
    if (dsp < DS_TWO_LESS_FULL) {
      trapDsOverflow();
    }
    ds[--dsp] = rB;
    ds[--dsp] = rA;
  }

  // Pushes first rB and second rA onto the software stack
  // WARNING: THIS IS INTENTIONALLY SAME ORDER TO twoPopSS.
  private final void twoPushSs() {
    if (ssp < SS_TWO_LESS_FULL) {
      trapSsOverflow();
    }
    ss[--ssp] = rB;
    ss[--ssp] = rA;
  }

  // Pushes first rC and second rB and third rA onto the data stack
  // WARNING: THIS IS INTENTIONALLY SAME ORDER TO threePopSS.
  private final void threePushDs() {
    if (dsp < DS_THREE_LESS_FULL) {
      trapDsOverflow();
    }
    ds[--dsp] = rC;
    ds[--dsp] = rB;
    ds[--dsp] = rA;
  }

  // Pushes first rD and second rC and third rB and fourth rA
  // onto the data stack
  // WARNING: THIS IS INTENTIONALLY SAME ORDER AS fourPopSS.
  private final void fourPushDs() {
    if (dsp < DS_FOUR_LESS_FULL) {
      trapDsOverflow();
    }
    ds[--dsp] = rD;
    ds[--dsp] = rC;
    ds[--dsp] = rB;
    ds[--dsp] = rA;
  }

  // Pushes first rC and second rB and third rA onto the software stack
  // WARNING: THIS IS INTENTIONALLY SAME ORDER AS threePopSS.
  private final void threePushSs() {
    if (ssp < SS_THREE_LESS_FULL) {
      trapSsOverflow();
    }
    ss[--ssp] = rC;
    ss[--ssp] = rB;
    ss[--ssp] = rA;
  }

  // Pushes first rD and second rC and third rB and fourth rA
  // onto the software stack
  // WARNING: THIS IS INTENTIONALLY SAME ORDER AS fourPopSS.
  private final void fourPushSs() {
    if (ssp < SS_FOUR_LESS_FULL) {
      trapSsOverflow();
    }
    ss[--ssp] = rD;
    ss[--ssp] = rC;
    ss[--ssp] = rB;
    ss[--ssp] = rA;
  }

  // Replaces the value on top of the data stack with value in rC
  // and second-top value on the data stack with value in rA
  // and third-top value on the data stack with value in rB
  // WARNING: THIS IS INTENTIONALLY DIFFERENT TO threePeekDs().
  private final void threeReplaceDs() {
    if (dsp > DS_HAS_THREE) {
      trapDsUnderflow();
    }
    ds[dsp] = rC;
    ds[dsp + 1] = rA;
    ds[dsp + 2] = rB;
  }

  // Replaces the value on top of the data stack with value in rB
  // and second-top value on the data stack with value in rC
  // and third-top value on the data stack with value in rA
  // WARNING: THIS IS INTENTIONALLY DIFFERENT TO threeReplaceDs().
  private final void threeRReplaceDs() {
    if (dsp > DS_HAS_THREE) {
      trapDsUnderflow();
    }
    ds[dsp] = rB;
    ds[dsp + 1] = rC;
    ds[dsp + 2] = rA;
  }

  // Pop from the data stack into rA.
  private final void popDs() {
    if (dsp > DS_HAS_ONE) {
      trapDsUnderflow();
    }
    rA = ds[dsp];
    dsp++;
  }

  // Pop from the return stack into rA.
  private final void popRs() {
    if (rsp > RS_HAS_ONE) {
      trapRsUnderflow();
    }
    rA = rs[rsp];
    rsp++;
  }

  // Peek from the software stack into rA.
  private final void peekSs() {
    if (ssp > SS_HAS_ONE) {
      trapSsUnderflow();
    }
    rA = ss[ssp];
  }

  // Peeks twice from the software stack, first into rB and second into rA.
  private final void twoPeekSs() {
    if (ssp > SS_HAS_TWO) {
      trapSsUnderflow();
    }
    rB = ss[ssp];
    rA = ss[ssp + 1];
  }

  // Peeks thrice from the software stack, first into rC and second into rB
  // and third into rA.
  private final void threePeekSs() {
    if (ssp > SS_HAS_THREE) {
      trapSsUnderflow();
    }
    rC = ss[ssp];
    rB = ss[ssp + 1];
    rA = ss[ssp + 2];
  }

  // Peeks 4 times from the software stack, first into rD and 2nd into rC
  // and third into rB and fourth into rA.
  private final void fourPeekSs() {
    if (ssp > SS_HAS_FOUR) {
      trapSsUnderflow();
    }
    rD = ss[ssp];
    rC = ss[ssp + 1];
    rB = ss[ssp + 2];
    rA = ss[ssp + 3];
  }

  // Pop from the software stack into rA.
  private final void popSs() {
    if (ssp > SS_HAS_ONE) {
      trapSsUnderflow();
    }
    rA = ss[ssp];
    ssp++;
  }

  // Pops twice from the software stack, first into rB and second into rA.
  private final void twoPopSs() {
    if (ssp > SS_HAS_TWO) {
      trapSsUnderflow();
    }
    rB = ss[ssp];
    ssp++;
    rA = ss[ssp];
    ssp++;
  }

  // Pops thrice from the software stack, first into rC and second into rB
  // and third into rA.
  private final void threePopSs() {
    if (ssp > SS_HAS_THREE) {
      trapSsUnderflow();
    }
    rC = ss[ssp];
    ssp++;
    rB = ss[ssp];
    ssp++;
    rA = ss[ssp];
    ssp++;
  }

  // Pops 4 times from the software stack, first into rD and second into rC
  // and third into rB and fourth into rA.
  private final void fourPopSs() {
    if (ssp > SS_HAS_FOUR) {
      trapSsUnderflow();
    }
    rD = ss[ssp];
    ssp++;
    rC = ss[ssp];
    ssp++;
    rB = ss[ssp];
    ssp++;
    rA = ss[ssp];
    ssp++;
  }

  // Pops twice from the data stack, first into rB and second into rA.
  // WARNING: INTENTIONALLY OPPOSITE ORDER TO twoPeekDS and twoReplaceDs().
  private final void twoPopDs() {
    if (dsp > DS_HAS_TWO) {
      trapDsUnderflow();
    }
    rB = ds[dsp];
    dsp++;
    rA = ds[dsp];
    dsp++;
  }

  // Pops thrice from the data stack, first into rC and second into rB
  // and third into rA.
  // WARNING: INTENTIONALLY OPPOSITE ORDER TO threeReplaceDs().
  private final void threePopDs() {
    if (dsp > DS_HAS_THREE) {
      trapDsUnderflow();
    }
    rC = ds[dsp];
    dsp++;
    rB = ds[dsp];
    dsp++;
    rA = ds[dsp];
    dsp++;
  }

  // Pops 4 times from the data stack, first into rD and second into rC
  // and third into rB and fourth into rA.
  // WARNING: INTENTIONALLY OPPOSITE ORDER TO threeReplaceDs().
  private final void fourPopDs() {
    if (dsp > DS_HAS_FOUR) {
      trapDsUnderflow();
    }
    rD = ds[dsp];
    dsp++;
    rC = ds[dsp];
    dsp++;
    rB = ds[dsp];
    dsp++;
    rA = ds[dsp];
    dsp++;
  }

  // Drops from the data stack
  private final void dropDs() {
    if (dsp > DS_HAS_ONE) {
      trapDsUnderflow();
    }
    dsp++;
  }

  // Drops 2 elements from the data stack
  private final void twoDropDs() {
    if (dsp > DS_HAS_TWO) {
      trapDsUnderflow();
    }
    dsp = dsp + 2;
  }

  // Drops 3 elements from the data stack
  private final void threeDropDs() {
    if (dsp > DS_HAS_THREE) {
      trapDsUnderflow();
    }
    dsp = dsp + 3;
  }

  // Drops 4 elements from the data stack
  private final void fourDropDs() {
    if (dsp > DS_HAS_FOUR) {
      trapDsUnderflow();
    }
    dsp = dsp + 4;
  }

  private final void ensureByteAddrWritable(int reg) {
    if ((reg < LOWEST_WRITABLE_BYTE) || (reg > HIGHEST_WRITABLE_BYTE)) {
      trapRAMBounds();
    }
  }

  private final void ensureWordAddrWritable(int reg) {
    if ((reg < LOWEST_WRITABLE_BYTE) || (reg > HIGHEST_WRITABLE_WORD)) {
      trapRAMBounds();
    }
  }

  private final void ensureByteAddressable(int reg) {
    if ((reg < 0) || (reg > HIGHEST_WRITABLE_BYTE)) {
      trapMemBounds();
    }
  }

  private final void ensureWordAddressable(int reg) {
    if ((reg < 0) || (reg > HIGHEST_WRITABLE_WORD)) {
      trapMemBounds();
    }
  }

  // =========================================================================
  // TRACING (optional for production VM)
  // =========================================================================
  private final void traceInfo() {
    if (TRON_ENABLED && (pc > 0)) {
      try {
        int opcode = memory.getInt(pc);
        if (opcode >= 0 && opcode <= 256) {
          stdtrcChannel.printf("%08x %-8s ", pc, traceTable[opcode]);
        } else {
          stdtrcChannel.printf("%08x  ", pc);
        }
        if (opcode < LOWEST_SIMPLE_OPCODE && (pc <= (HIGHEST_WRITABLE_WORD - WORD_SIZE))) {
          int cellValue = memory.getInt(pc + WORD_SIZE);
          stdtrcChannel.printf("%08x ", cellValue);
        } else {
          stdtrcChannel.printf("         ");
        }
      } catch (Exception e) {
        trapCantWriteToStdtrc();
      }
    }
  }

  private final void traceStacks() {
    if (TRON_ENABLED) {
      try {
        stdtrcChannel.printf("( ");
        int numElems = (DS_EMPTY - dsp);
        for (int i = 1; i <= numElems; i++) {
          stdtrcChannel.printf("%08x ", ds[dsStop - i]);
        }
        stdtrcChannel.printf(") ");
        stdtrcChannel.printf("[ ");
        numElems = (SS_EMPTY - ssp);
        for (int i = 1; i <= numElems; i++) {
          stdtrcChannel.printf("%08x ", ss[ssStop - i]);
        }
        stdtrcChannel.printf("] ");
        stdtrcChannel.printf("{ ");
        numElems = (RS_EMPTY - rsp);
        for (int i = 1; i <= numElems; i++) {
          stdtrcChannel.printf("%08x ", rs[rsStop - i]);
        }
        stdtrcChannel.printf("} \n");
      } catch (Exception e) {
        trapCantWriteToStdtrc();
      }
    }
  }

  // =========================================================================
  // INSTRUCTION SET
  // =========================================================================
  // opcodeTable
  // haltOpcode (1)
  private static final int iWALL = 0; // WALL must be zero
  // complexOpcodes (37)
  private static final int iLIT = 1;
  private static final int iCALL = 2;
  private static final int iJMP = 3;
  private static final int iBRGZ = 4;
  private static final int iBRGEZ = 5;
  private static final int iBRZ = 6;
  private static final int iBRNZ = 7;
  private static final int iBRLEZ = 8;
  private static final int iBRLZ = 9;
  private static final int iBRG = 10;
  private static final int iBRGE = 11;
  private static final int iBRE = 12;
  private static final int iBRNE = 13;
  private static final int iBRLE = 14;
  private static final int iBRL = 15;
  private static final int iJGZ = 16;
  private static final int iJGEZ = 17;
  private static final int iJZ = 18;
  private static final int iJNZ = 19;
  private static final int iJLEZ = 20;
  private static final int iJLZ = 21;
  private static final int iJG = 22;
  private static final int iJGE = 23;
  private static final int iJE = 24;
  private static final int iJNE = 25;
  private static final int iJLE = 26;
  private static final int iJL = 27;
  private static final int iREADOR = 28;
  private static final int iWRITOR = 29;
  private static final int iTRACOR = 30;
  private static final int iGETOR = 31;
  private static final int iPUTOR = 32;
  private static final int iREADORB = 33;
  private static final int iWRITORB = 34;
  private static final int iTRACORB = 35;
  private static final int iGETORB = 36;
  private static final int iPUTORB = 37;
  // complexOpcodesEnd
  // simpleOpcodes (111)
  private static final int iEXIT = 145;
  private static final int iDCALL = 146;
  private static final int iRDCALL = 147;
  private static final int iDJMP = 148;
  private static final int iSWAP = 149;
  private static final int iOVER = 150;
  private static final int iROT = 151;
  private static final int iTOR = 152;
  private static final int iLEAP = 153;
  private static final int iNIP = 154;
  private static final int iTUCK = 155;
  private static final int iREV = 156;
  private static final int iRPUSH = 157;
  private static final int iRPOP = 158;
  private static final int iDROP = 159;
  private static final int iDROP2 = 160;
  private static final int iDROP3 = 161;
  private static final int iDROP4 = 162;
  private static final int iDUP = 163;
  private static final int iDUP2 = 164;
  private static final int iDUP3 = 165;
  private static final int iDUP4 = 166;
  private static final int iHOLD = 167;
  private static final int iHOLD2 = 168;
  private static final int iHOLD3 = 169;
  private static final int iHOLD4 = 170;
  private static final int iSPEEK = 171;
  private static final int iSPEEK2 = 172;
  private static final int iSPEEK3 = 173;
  private static final int iSPEEK4 = 174;
  private static final int iSPUSH = 175;
  private static final int iSPUSH2 = 176;
  private static final int iSPUSH3 = 177;
  private static final int iSPUSH4 = 178;
  private static final int iSPOP = 179;
  private static final int iSPOP2 = 180;
  private static final int iSPOP3 = 181;
  private static final int iSPOP4 = 182;
  private static final int iDEC = 183;
  private static final int iDECW = 184;
  private static final int iDEC2W = 185;
  private static final int iINC = 186;
  private static final int iINCW = 187;
  private static final int iINC2W = 188;
  private static final int iLOAD = 189;
  private static final int iSTORE = 190;
  private static final int iRLOAD = 191;
  private static final int iLOADB = 192;
  private static final int iSTOREB = 193;
  private static final int iRLOADB = 194;
  private static final int iPLOAD = 195;
  private static final int iPSTORE = 196;
  private static final int iRPLOAD = 197;
  private static final int iPLOADB = 198;
  private static final int iPSTOREB = 199;
  private static final int iRPLOADB = 200;
  private static final int iADD = 201;
  private static final int iSUB = 202;
  private static final int iMUL = 203;
  private static final int iDIV = 204;
  private static final int iMOD = 205;
  private static final int iDIVMOD = 206;
  private static final int iRADD = 207;
  private static final int iRSUB = 208;
  private static final int iRMUL = 209;
  private static final int iRDIV = 210;
  private static final int iRMOD = 211;
  private static final int iRDIVMOD = 212;
  private static final int iNEG = 213;
  private static final int iABS = 214;
  private static final int iAND = 215;
  private static final int iOR = 216;
  private static final int iXOR = 217;
  private static final int iRAND = 218;
  private static final int iROR = 219;
  private static final int iRXOR = 220;
  private static final int iSHL = 221;
  private static final int iSHR = 222;
  private static final int iRSHL = 223;
  private static final int iRSHR = 224;
  private static final int iMOVE = 225;
  private static final int iFILL = 226;
  private static final int iFIND = 227;
  private static final int iMATCH = 228;
  private static final int iMOVEB = 229;
  private static final int iFILLB = 230;
  private static final int iFINDB = 231;
  private static final int iMATCHB = 232;
  private static final int iHOMIO = 233;
  private static final int iRCHAN = 234;
  private static final int iWCHAN = 235;
  private static final int iGCHAN = 236;
  private static final int iPCHAN = 237;
  private static final int iECODE = 238;
  private static final int iRCODE = 239;
  private static final int iROM = 240;
  private static final int iRAM = 241;
  private static final int iMAP = 242;
  private static final int iSTDBLK = 243;
  private static final int iDS = 244;
  private static final int iSS = 245;
  private static final int iRS = 246;
  private static final int iDSN = 247;
  private static final int iSSN = 248;
  private static final int iRSN = 249;
  private static final int iTRON = 250;
  private static final int iTROFF = 251;
  private static final int iRESET = 252;
  private static final int iREBOOT = 253;
  private static final int iHALT = 254;
  private static final int iDATA = 255;

  // simpleOpcodesEnd
  // opcodeTableEnd
  
  private static final int LOWEST_SIMPLE_OPCODE = iEXIT;

  // =========================================================================
  // EXAMPLE OF INDIRECT THREADED PROGRAM
  // =========================================================================
  // Uncomment this section, along with systemCopyProgram below, to copy
  // the below hardcoded example program into ROM rather than loading a program
  // into ROM from the ROM file (as systemLoadProgram normally does).
  // You must also comment out systemLoadProgram.
  //
  // Note: either way, performance is the same since the actual program
  // (once copied or loaded into ROM) executes from ROM just the same.
  // To obtain faster performance one could in theory either write a
  // just-in-time compiler for Freeputer programs that would nicely compile
  // FVM instructions into native machine code or use hardcoded direct
  // threading rather than hardcoded indirect threading. However,
  // one must bear in mind that the safety and lack of undefined behaviour
  // of the FVM must be retained (not discarded in favour of speed).
  //------------------------------------------------------------------------- 
  /*
  //-------------------------------------------------------------------------                                   
   private static final int COUNTER = 10; 		// 2147483647;
   private static final int PROGRAM_SIZE = 7;
   // Count down from COUNTER to 0
   private static final int[] itProg = {
	   iLIT,COUNTER,
	   iDEC, 			// countdown:
	   iBRGZ, 8, 		// branch to byte address of countdown label
	   iDROP, iHALT };
  // -------------------------------------------------------------------------
  */
  // =========================================================================
   
   
  // =========================================================================
  // PRIVATE SERVICES
  // =========================================================================

  /* Open stdtrc */
  private final void openStdtrc() {
    try {
      if (appendStdtrc) {
        stdtrcChannel = new PrintStream(new FileOutputStream(
          stdtrcFilename, true)); // Used for running fvmtest suite
      } else {
        stdtrcChannel = new PrintStream(new FileOutputStream(
          stdtrcFilename)); // Used normally
      }
    } catch (Exception e) {
      trapCantOpenStdtrc();
    }
  }

  /* Close stdtrc */
  private final void closeStdtrc() {
    try {
      stdtrcChannel.close();
    } catch (Exception e) {
      trapCantCloseStdtrc();
    }
  }

  /* Open stdexp */
  private final void openStdexp() {
    try {
      stdexpChannel = new FileOutputStream(stdexpFilename).getChannel();
    } catch (Exception e) {
      trapCantOpenStdexp();
    }
  }

  /* Close stdexp */
  private final void closeStdexp() {
    try {
      stdexpChannel.close();
    } catch (IOException e) {
      trapCantCloseStdexp();
    }
  }

  /* Open stdimp */
  private final void openStdimp() {
    try {
      stdimpChannel = new FileInputStream(stdimpFilename).getChannel();

    } catch (Exception e) {
      trapCantOpenStdimp();
    }
  }

  /* Close stdimp */
  private final void closeStdimp() {
    try {
      stdimpChannel.close();
    } catch (IOException e) {
      trapCantCloseStdimp();
    }
  }

  /* Open ROM */
  private final void openRom() {
    try {
      romStream = new FileInputStream(romFilename);
    } catch (Exception e) {
      trapCantOpenRom();
    }
  }

  /* Close ROM */
  private final void closeRom() {
    try {
      romStream.close();
    } catch (IOException e) {
      trapCantCloseRom();
    }
  }

  /* Open stdblk */
  private final void openStdblk() {
    try {
      stdblkChannel = new RandomAccessFile(stdblkFilename, "rw")
          .getChannel();
    } catch (Exception e) {
      trapCantOpenStdblk();
    }
  }

  /* Close stdblk */
  private final void closeStdblk() {
    try {
      stdblkChannel.close();
    } catch (IOException e) {
      trapCantCloseStdblk();
    }
  }

  /* Open stdin */
  private final void openStdin() {
    stdinStream = System.in;
  }

  /* Close stdin */
  private final void closeStdin() {
    // nothing to do here unless stdinStream is not System.in
  }

  /* Open stdin */
  private final void openStdout() {
    stdoutStream = System.out;
  }

  /* Close stdin */
  private final void closeStdout() {
    // nothing to do here unless stdoutStream is not System.out
  }

  // =========================================================================
  // RESET POINTS
  // =========================================================================
  private final void clearStateAndInitialize() {
    clearState();
    clearMem();
    systemInitDevices();
  }

  /* Wipe entire FVM memory, stacks and variables */
  private final void systemHardReset() {
    clearStateAndInitialize();
    throw new IllegalStateException("systemHardReset with exit code:"+rB);
  }

  /* Wipe FVM stacks and variables but not memory */
  private final void systemSoftReset() {
    clearState();
    systemInitDevices();
    throw new IllegalStateException("systemSoftReset with exit code:"+rB);
  }

  // =========================================================================
  // INSTRUCTION IMPLEMENTATIONS
  // =========================================================================
  private final void iBrgz() {
    peekDs();
    if (rA <= 0) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iDec() {
    peekDs();
    if (rA == NEG_INT_MAX) {
      trapMathOverflow();
    }
    rA--;
    replaceDs();
  }

  private final void iLit() {
    wordAtPc();
    pushDs();
    incPc();
  }

  private final void iTroff() {
    vmFlags = vmFlags & 0xFFFFFFFE; // 0b11111111111111111111111111111110;
  }

  private final void iTron() {
    vmFlags = vmFlags | 0x00000001; // 0b00000000000000000000000000000001;
  }

  private final void iWchan() {
    popDs();
    wchannel = rA;
  }

  private final void iRchan() {
    popDs();
    rchannel = rA;
  }

  private final void iGchan() {
    popDs();
    gchannel = rA;
  }

  private final void iPchan() {
    popDs();
    pchannel = rA;
  }

  private final void iEcode() {
    rA = lastExitCode;
    pushDs();
  }

  private final void iRcode() {
    rA = lastRestartCode;
    pushDs();
  }

  private final void iRom() {
    rA = ROM_SIZE;
    pushDs();
  }

  private final void iRam() {
    rA = RAM_SIZE;
    pushDs();
  }

  private final void iMap() {
    rA = MAP_SIZE;
    pushDs();
  }

  private final void iStdblk() {
    rA = STDBLK_SIZE;
    pushDs();
  }

  private final void iDs() {
    rA = MAX_DEPTH_DS;
    pushDs();
  }

  private final void iSs() {
    rA = MAX_DEPTH_SS;
    pushDs();
  }

  private final void iRs() {
    rA = MAX_DEPTH_RS;
    pushDs();
  }

  private final void iDsn() {
    if (dsp < DS_EMPTY) {
    // data stack is not empty
      rA = DS_EMPTY - dsp;
    } else {
      // data stack is empty
      rA = 0;
    }
    pushDs();
  }

  private final void iSsn() {
    if (ssp < SS_EMPTY) {
      // software stack is not empty
      rA = SS_EMPTY - ssp;
    } else {
      // software stack is empty
      rA = 0;
    }
    pushDs();
  }

  private final void iRsn() {
    if (rsp < RS_EMPTY) {
      // return stack is not empty
      rA = RS_EMPTY - rsp;
    } else {
      // return stack is empty
      rA = 0;
    }
    pushDs();
  }

  private final void iRev() {
    if (dsp > DS_HAS_THREE) {
      trapDsUnderflow();
    }
    rA = ds[dsp]; // n1
    rC = ds[dsp + 2]; // n3
    ds[dsp] = rC;
    ds[dsp + 2] = rA;
  }

  private final void iGetor() {
    switch (gchannel) {
    case STDBLK:
      getBuf.putInt(0, 0); // zero the buffer
      getBuf.clear(); // reset the buffer
      popDs();
      if (rA > STDBLK_SIZE) {
        branch(); // outside block device bounds, cannot get
        return;
      }
      try {
        stdblkChannel.position(rA);
      } catch (Exception e) {
        branch(); // seek failed
        return;
      }
      try {
        if (stdblkChannel.read(getBuf) == -1) {
          branch(); // got no bytes
          return;
        }
      } catch (Exception e) {
        branch(); // get failed
        return;
      }
      rA = getBuf.getInt(0);
      pushDs();
      dontBranch();
      return;
    default:
      branch(); // Unsupported gchannel
      return;
    }
  }

  private final void iGetorb() {
    switch (gchannel) {
    case STDBLK:
      getbBuf.put(0, (byte) 0); // zero the buffer
      getbBuf.clear(); // reset the buffer
      popDs();
      if (rA > STDBLK_SIZE) {
        branch(); // outside block device bounds, cannot get
        return;
      }
      try {
        stdblkChannel.position(rA);
      } catch (Exception e) {
        branch(); // seek failed
        return;
      }
      try {
        if (stdblkChannel.read(getbBuf) == -1) {
          branch(); // got no bytes
          return;
        }
      } catch (Exception e) {
        branch(); // get failed
        return;
      }
      rA = getbBuf.get(0);
      pushDs();
      dontBranch();
      return;
    default:
      branch(); // Unsupported gchannel
      return;
    }
  }

  private final void iPutor() {
    switch (pchannel) {
    case STDBLK:
      twoPopDs(); // Address in now in rB
      putBuf.clear(); // Reset the buf
      putBuf.putInt(0, rA); // Value to be put is now in putBuf
      if (rB > STDBLK_SIZE) {
        branch(); // outside block device bounds, cannot get
        return;
      }
      try {
        stdblkChannel.position(rB);
      } catch (Exception e) {
        branch(); // seek failed
        return;
      }
      try {
        stdblkChannel.write(putBuf);
      } catch (Exception e) {
        branch(); // put failed
        return;
      }
      dontBranch();
      return;
    default:
      branch(); // Unsupported pchannel
      return;
    }
  }

  private final void iPutorb() {
    switch (pchannel) {
    case STDBLK:
      twoPopDs(); // Address in now in rB
      putbBuf.clear(); // Reset the buf
      putbBuf.put(0, (byte) rA); // Value to be put is now in putBuf
      if (rB > STDBLK_SIZE) {
        branch(); // outside block device bounds, cannot get
        return;
      }
      try {
        stdblkChannel.position(rB);
      } catch (Exception e) {
        branch(); // seek failed
        return;
      }
      try {
        stdblkChannel.write(putbBuf);
      } catch (Exception e) {
        branch(); // put failed
        return;
      }
      dontBranch();
      return;
    default:
      branch(); // Unsupported pchannel
      return;
    }
  }

  private final void iReador() {
    readBuf.putInt(0, 0); // zero the buffer
    readBuf.clear(); // reset the buffer
    switch (rchannel) {
    case STDIN:
      try {
        if (stdinStream.read(readBuf.array()) == -1) {
          branch(); // end of stream
          return;
        }
        rA = readBuf.getInt(0);
        pushDs();
        dontBranch();
        return;
      } catch (Exception e) {
        branch(); // read failed
        return;
      }
    case STDIMP:
      try {
        if (stdimpChannel.read(readBuf) == -1) {
          branch(); // end of stream
          return;
        }
        rA = readBuf.getInt(0);
        pushDs();
        dontBranch();
        return;
      } catch (Exception e) {
        branch(); // read failed
        return;
      }
    default:
      branch(); // Unsupported rchannel
      return;
    }
  }

  private final void iReadorb() {
    readbBuf.put(0, (byte) 0); // zero the buffer
    readbBuf.clear(); // reset the buffer
    switch (rchannel) {
    case STDIN:
      try {
        rA = stdinStream.read(); // Always gives 0 to 255 (or -1 if EOS)
        if (rA == -1) {
          branch(); // end of stream
          return;
        }
        pushDs();
        dontBranch();
        return;
      } catch (Exception e) {
        branch(); // read failed
        return;
      }
    case STDIMP:
      try {
        if (stdimpChannel.read(readbBuf) == -1) {
          branch(); // end of stream
          return;
        }
        rA = (0x000000FF & (int) readbBuf.get(0));
        pushDs();
        dontBranch();
        return;
      } catch (Exception e) {
        branch(); // read failed
        return;
      }
    default:
      branch(); // Unsupported rchannel
      return;
    }
  }

  private final void iWritor() {
    switch (wchannel) {
    case STDOUT:
      popDs();
      writeBuf.clear(); // Reset the buf
      writeBuf.putInt(0, rA);
      try {
        stdoutStream.write(writeBuf.array());
      } catch (Exception e) {
        branch(); // write failed
        return;
      }
      dontBranch();
      return;
    case STDEXP:
      popDs();
      writeBuf.clear(); // Reset the buf
      writeBuf.putInt(0, rA);
      try {
        stdexpChannel.write(writeBuf);
      } catch (Exception e) {
        branch(); // write failed
        return;
      }
      dontBranch();
      return;
    default:
      branch(); // Unsupported wchannel
      return;
    }
  }

  private final void iWritorb() {
    switch (wchannel) {
    case STDOUT:
      popDs();
      writebBuf.clear(); // Reset the buf
      writebBuf.put(0, (byte) rA); // Value to be written is now in writeBuf
      try {
        stdoutStream.write(writebBuf.array());
      } catch (Exception e) {
        branch(); // write failed
        return;
      }
      dontBranch();
      return;
    case STDEXP:
      popDs();
      writebBuf.clear(); // Reset the buf
      writebBuf.put(0, (byte) rA); // Value to be written is now in writeBuf
      try {
        stdexpChannel.write(writebBuf);
      } catch (Exception e) {
        branch(); // write failed
        return;
      }
      dontBranch();
      return;
    default:
      branch(); // Unsupported wchannel
      return;
    }
  }

  private final void iTracor() {
    popDs();
    try {
      stdtrcChannel.write((byte) (0x000000FF & rA));
      stdtrcChannel.write((byte) ((0x0000FF00 & rA) >> 8));
      stdtrcChannel.write((byte) ((0x00FF0000 & rA) >> 16));
      stdtrcChannel.write((byte) ((0xFF000000 & rA) >> 24));
    } catch (Exception e) {
      branch(); // write failed
      return;
    }
    dontBranch();
  }

  private final void iTracorb() {
    popDs();
    try {
      stdtrcChannel.write((byte) rA);
    } catch (Exception e) {
      branch(); // write failed
      return;
    }
    dontBranch();
  }

  private final void iFind() {
    // ( numWords n src -- wordIndex ) find 1st instance of n from src
    // onwards but look at no more than numWords
    threePopDs(); // rA = numWords, rB = n, rC = src
    ensureWordAddressable(rC);
    if (rA >= 0) { // FIXME this overflow protection is untested
      if (((POS_INT_MAX-rC)/ WORD_SIZE)<rA) {
        trapMemBounds();
      } 
    } else {
      if (((NEG_INT_MAX+rC)/ WORD_SIZE)>rA) {
        trapMemBounds();
      } 
    }
    ensureWordAddressable(rC + (rA * WORD_SIZE));
    if (rA == 0) {
      // numWords is zero
      rA = -1; // -1 means not found
      pushDs();
      ;
      return;
    } else if (rA < 0) {
      // numWords is negative, do descending search // FIMXE negative
      // FINDs and MOVEs are untested, add to unit tests
      int addr;
      int result = -1; // -1 means not found
      for (int i = 0; i > rA; i--) {
        addr = rC + (i * WORD_SIZE);
        if (rB == memory.getInt(addr)) {
          result = (0 - i);
          break;
        }
      }
      rA = result;
      pushDs();
      return;
    } else {
      // numWords is positive, do ascending search
      int addr;
      int result = -1; // -1 means not found
      for (int i = 0; i < rA; i++) {
        addr = rC + (i * WORD_SIZE);
        if (rB == memory.getInt(addr)) {
          result = i;
          break;
        }
      }
      rA = result;
      pushDs();
    }
  }

  private final void iFindb() {
    // ( numBytes b src -- byteIndex ) find 1st instance of b from src
    // onwards but look at no more than numBytes
    threePopDs(); // rA = numBytes, rB = n, rC = src
    ensureByteAddressable(rC);
    if (rA >= 0) { // FIXME this overflow protection is untested
      if ((POS_INT_MAX-rC)<rA) {
      trapMemBounds();
      } 
    } else {
      if ((NEG_INT_MAX+rC)>rA) {
      trapMemBounds();
      } 
    }
    ensureByteAddressable(rC + rA);
    if (rA == 0) {
      // numBytes is zero
      rA = -1; // -1 means not found
      pushDs();
      ;
      return;
    } else if (rA < 0) {
      // numBytes is negative, do descending search
      int addr;
      int result = -1; // -1 means not found
      for (int i = 0; i > rA; i--) {
        addr = rC + i;
        if (memory.get(addr) == (byte) rB) {
          result = (0 - i);
          break;
        }
      }
      rA = result;
      pushDs();
      return;
    } else {
      // numBytes is positive, do ascending search
      int addr;
      int result = -1; // -1 means not found
      for (int i = 0; i < rA; i++) {
        addr = rC + i;
        if (memory.get(addr) == (byte) rB) {
          result = i;
          break;
        }
      }
      rA = result;
      pushDs();
      return;
    }
  }

  private final void iFill() {
    // ( numWords n dest -- ) fill numWords with n at dest onwards
    threePopDs(); // rA = numWords, rB = n, rC = dest
    ensureWordAddrWritable(rC);
    if (((POS_INT_MAX-rC)/ WORD_SIZE)<rA) { // FIXME this overflow protection is untested
      trapMemBounds();
    }
    ensureWordAddrWritable(rC + (rA * WORD_SIZE));
    if (rA <= 0) {
      // numWords is zero or less, so do nothing
      return;
    } else {
      int addr;
      for (int i = 0; i < rA; i++) {
        addr = rC + (i * WORD_SIZE);
        memory.putInt(addr, rB);
      }
      return;
    }
  }

  private final void iFillb() {
    // ( numBytes b dest -- ) fill numBytes with b at dest onwards
    threePopDs(); // rA = numBytes, rB = n, rC = dest
    ensureByteAddrWritable(rC);
    if ((POS_INT_MAX-rC)<rA) { // FIXME this overflow protection is untested
      trapMemBounds();
    }
    ensureByteAddrWritable(rC + rA);
    if (rA <= 0) {
      // numBytes is zero or less, so do nothing
      return;
    } else {
      int addr;
      for (int i = 0; i < rA; i++) {
        addr = rC + i;
        memory.put(addr, (byte) rB);
      }
      return;
    }
  }

  private final void iMatch() {
    // ( numWords src dest -- TRUE/FALSE ) see if strings match
    {
      threePopDs(); // rA = numWords, rB = src, rC = dest
      ensureWordAddressable(rB);
      if (rA >= 0) { // FIXME this overflow protection is untested
        if (((POS_INT_MAX-rB)/ WORD_SIZE)<rA) {
        trapMemBounds();
        } 
      } else {
        if (((NEG_INT_MAX+rB)/ WORD_SIZE)>rA) {
        trapMemBounds();
        } 
      }
      ensureWordAddressable(rB + (rA * WORD_SIZE));
      ensureWordAddressable(rC);
      if (rA >= 0) { // FIXME this overflow protection is untested
        if (((POS_INT_MAX-rC)/ WORD_SIZE)<rA) {
        trapMemBounds();
        } 
      } else {
        if (((NEG_INT_MAX+rC)/ WORD_SIZE)>rA) {
        trapMemBounds();
        } 
      }
      ensureWordAddressable(rC + (rA * WORD_SIZE));
      int w1addr;
      int w2addr;
      int result = TRUE;
      for (int i = 0; i < rA; i++) {
        w1addr = rB + (i * WORD_SIZE);
        w2addr = rC + (i * WORD_SIZE);
        if (memory.getInt(w1addr) != memory.getInt(w2addr)) {
          result = FALSE;
          break; // mismatch
        }
      }
      rA = result;
      pushDs();
      ;
    }
  }

  private final void iMatchb() {
    // ( numBytes src dest -- TRUE/FALSE ) see if strings match
    threePopDs(); // rA = numBytes, rB = src, rC = dest
    {
      ensureByteAddressable(rB);
      if (rA >= 0) { // FIXME this overflow protection is untested
        if ((POS_INT_MAX-rB)<rA) {
        trapMemBounds();
        } 
      } else {
        if ((NEG_INT_MAX+rB)>rA) {
        trapMemBounds();
        } 
      }
      ensureByteAddressable(rB + rA);
      ensureByteAddressable(rC);
      if (rA >= 0) { // FIXME this overflow protection is untested
        if ((POS_INT_MAX-rC)<rA) {
        trapMemBounds();
        } 
      } else {
        if ((NEG_INT_MAX+rC)>rA) {
        trapMemBounds();
        } 
      }
      ensureByteAddressable(rC + rA);
      int b1addr;
      int b2addr;
      int result = TRUE;
      for (int i = 0; i < rA; i++) {
        b1addr = rB + i;
        b2addr = rC + i;
        if (memory.get(b1addr) != memory.get(b2addr)) {
          result = FALSE;
          break; // mismatch
        }
      }
      rA = result;
      pushDs();
      ;
    }
  }

  private final void iMoveb() {
    // ( numBytes src dest -- ) copy byte at src addr to dest addr
    threePopDs(); // rA = numBytes, rB = src, rC = dest
    ensureByteAddressable(rB);
    if (rA >= 0) { // FIXME this overflow protection is untested
      if ((POS_INT_MAX-rB)<rA) {
      trapMemBounds();
      } 
    } else {
      if ((NEG_INT_MAX+rB)>rA) {
      trapMemBounds();
      } 
    }
    ensureByteAddressable(rB + rA);
    ensureByteAddrWritable(rC);
    if (rA >= 0) { // FIXME this overflow protection is untested
      if ((POS_INT_MAX-rC)<rA) {
      trapMemBounds();
      } 
    } else {
      if ((NEG_INT_MAX+rC)>rA) {
      trapMemBounds();
      } 
    }
    ensureByteAddrWritable(rC + rA);
    if (rA == 0) {
      // Nothing to do, numBytes is 0
      return;
    } else if (rB == rC) {
      // Nothing to do, src and dest are the same
      return;
    } else if (rA > 0) {
      // numBytes is positive, do ascending move
      int b1addr;
      int b2addr;
      for (int i = 0; i < rA; i++) {
        b1addr = rB + i;
        b2addr = rC + i;
        memory.put(b2addr, memory.get(b1addr));
      }
      return;
    } else {
      // numBytes is negative, do descending move
      int b1addr;
      int b2addr;
      for (int i = 0; i > rA; i--) {
        b1addr = rB + i;
        b2addr = rC + i;
        memory.put(b2addr, memory.get(b1addr));
      }
    }
  }

  private final void iMove() {
    // ( numWords src dest -- ) copy word at src addr to dest addr
    threePopDs(); // rA = numWords, rB = src, rC = dest
    ensureWordAddressable(rB);
    if (rA >= 0) { // FIXME this overflow protection is untested
      if (((POS_INT_MAX-rB)/ WORD_SIZE)<rA) {
      trapMemBounds();
      } 
    } else {
      if (((NEG_INT_MAX+rB)/ WORD_SIZE)>rA) {
      trapMemBounds();
      } 
    }
    ensureWordAddressable(rB + (rA * WORD_SIZE));
    ensureWordAddrWritable(rC);
    if (rA >= 0) { // FIXME this overflow protection is untested
      if (((POS_INT_MAX-rC)/ WORD_SIZE)<rA) {
      trapMemBounds();
      } 
    } else {
      if (((NEG_INT_MAX+rC)/ WORD_SIZE)>rA) {
      trapMemBounds();
      } 
    }
    ensureWordAddrWritable(rC + (rA * WORD_SIZE));
    if (rA == 0) {
      // Nothing to do, numWords is 0
      return;
    } else if (rB == rC) {
      // Nothing to do, src and dest are the same
      return;
    } else if (rA > 0) {
      // numWords is positive, do ascending move
      int w1addr;
      int w2addr;
      for (int i = 0; i < rA; i++) {
        w1addr = rB + (i * WORD_SIZE);
        w2addr = rC + (i * WORD_SIZE);
        memory.putInt(w2addr, memory.getInt(w1addr));
      }
      return;
    } else {
      // numWords is negative, do descending move
      int w1addr;
      int w2addr;
      for (int i = 0; i > rA; i--) {
        w1addr = rB + (i * WORD_SIZE);
        w2addr = rC + (i * WORD_SIZE);
        memory.putInt(w2addr, memory.getInt(w1addr));
      }
      return;
    }
  }

  private final void iLoadb() {
    // ( a -- byte )
    popDs();
    ensureByteAddressable(rA);
    rA = memory.get(rA); // Retrieve byte starting at specified byte
    pushDs();
  }

  private final void iRloadb() {
    peekDs();
    ensureByteAddressable(rA);
    rA = memory.get(rA); // Retrieve byte starting at specified byte
    pushDs();
  }

  private final void iLoad() {
    popDs();
    ensureWordAddressable(rA);
    rA = memory.getInt(rA); // Retrieve word starting at specified byte
    pushDs();
  }

  private final void iRload() {
    peekDs();
    ensureWordAddressable(rA);
    rA = memory.getInt(rA); // Retrieve word starting at specified byte
    pushDs();
  }

  private final void iPload() {
    // ( p -- word ) Like load but assumes address is a pointer
    // and therefore loads word from address stored at p
    popDs();
    ensureWordAddressable(rA);
    rA = memory.getInt(rA); // Retrieve addr starting at specified byte
    ensureWordAddressable(rA);
    rA = memory.getInt(rA);
    pushDs();
  }

  private final void iRpload() {
    // ( p -- p word ) Like rload but assumes address is a pointer
    peekDs();
    ensureWordAddressable(rA);
    rA = memory.getInt(rA); // Retrieve addr starting at specified byte
    ensureWordAddressable(rA);
    rA = memory.getInt(rA);
    pushDs();
  }

  private final void iPloadb() {
    ; // ( p -- p word ) Like loadb but assumes address is a pointer
      // and therefore loads byte from address stored at p
    popDs();
    ensureByteAddressable(rA);
    rA = memory.getInt(rA); // Retrieve addr starting at specified byte
    ensureByteAddressable(rA);
    rA = memory.get(rA);
    pushDs();
  }

  private final void iRploadb() {
    ; // ( p -- p word ) Like rloadb but assumes address is a pointer
      // and therefore loads byte from address stored at p
    peekDs();
    ensureByteAddressable(rA);
    rA = memory.getInt(rA); // Retrieve addr starting at specified byte
    ensureByteAddressable(rA);
    rA = memory.get(rA);
    pushDs();
  }

  private final void iStore() {
    // ( n a -- )
    popDs();
    rB = rA; // Address is in rB
    popDs(); // Value to store is in rA
    ensureWordAddrWritable(rB);
    memory.putInt(rB, rA);
  }

  private final void iStoreb() {
    // ( n a -- )
    popDs();
    rB = rA; // Address is in rB
    popDs(); // Value to store is in rA
    ensureByteAddrWritable(rB);
    memory.put(rB, (byte) rA);
  }

  private final void iPstore() {
    // ( n p -- ) Like STORE but assumes p is a pointer
    // and therefore stores word to address stored at p
    popDs();
    rB = rA; // Pointer is in rB
    popDs(); // Value to store is in rA
    ensureWordAddressable(rB);
    rB = memory.getInt(rB);
    ensureWordAddrWritable(rB);
    memory.putInt(rB, rA);
  }

  private final void iPstoreb() {
    // ( n p -- ) Like STOREB but assumes p is a pointer
    // and therefore stores word to address stored at p
    popDs();
    rB = rA; // Pointer is in rB
    popDs(); // Value to store is in rA
    ensureWordAddressable(rB);
    rB = memory.getInt(rB);
    ensureByteAddrWritable(rB);
    memory.put(rB, (byte) rA);
  }

  private final void iReboot() {
    lastRestartCode = LAST_RESTART_CODE_REBOOT;
    systemHardReset(); // Program requested hard reset
  }

  private final void iReset() {
    lastRestartCode = LAST_RESTART_CODE_RESET;
    systemReset(); // Program requested soft reset
    return;
  }

  private final void iCall() { // ( -- )
    rA = pc; // rA now contains return address
    rA = rA + WORD_SIZE;
    if (rA < 0) {
      trapPcOverflow(); // Protection against silent overflow of addition
    }
    pushRs(); // Return address is now on return stack
    branch(); // Additional check occurs here
  }

  private final void iDcall() {
    // ( a -- )
    popDs();
    rB = rA; // rB now contains call address
    // Ensure dynamic call address lies within program memory
    if ((rB < 0) || (rB > HIGHEST_WRITABLE_WORD)) {
      trapPcOverflow();
    }
    rA = pc; // rA now contains return address
    pushRs(); // Return address is now on return stack
    pc = rB; // pc now contains call address
  }

  private final void iRdcall() {
    // ( a -- a ) ONLY SAFE FOR NON-CONSUMING FUNCTIONS!
    peekDs();
    rB = rA; // rB now contains call address
    // Ensure dynamic call address lies within program memory
    if ((rB < 0) || (rB > HIGHEST_WRITABLE_WORD)) {
      trapPcOverflow();
    }
    rA = pc; // rA now contains return address
    pushRs(); // Return address is now on return stack
    pc = rB; // pc now contains call address
  }

  private final void iExit() {
    popRs();
    // Ensure return address lies within program memory
    if ((rA < 0) || (rA > HIGHEST_WRITABLE_WORD)) {
      trapPcOverflow();
    }
    pc = rA;
  }

  private final void iSpop() {
    // ( -- n ) [ n -- ]
    popSs();
    pushDs();
  }

  private final void iSpeek() {
    // ( -- n ) [ n -- n ]
    peekSs();
    pushDs();
  }

  private final void iSpeek2() {
    // ( -- n1 n2 ) [ n1 n2 -- n1 n2 ] Note: NOT same as spop spop
    twoPeekSs();
    rD = rA;
    rA = rB;
    rB = rD;
    twoPushDs();
  }

  private final void iSpeek3() {
    // ( -- n1 n2 n3 ) [ n1 n2 n3 -- n1 n2 n3 ] NOT same as spop spop spop
    threePeekSs();
    rD = rC;
    rC = rA;
    rA = rD;
    threePushDs();
  }

  private final void iSpeek4() {
    // ( -- n1 n2 n3 n4 ) [ n1 n2 n3 n4 -- n1 n2 n3 n4 ] NOT like spop x 4
    fourPeekSs();
    {
      int buf = rA;
      rA = rD;
      rD = buf;
      buf = rB;
      rB = rC;
      rC = buf;
    }
    fourPushDs();
  }

  private final void iSpop2() {
    // ( -- n1 n2 ) [ n1 n2 -- ] Note: NOT same as spop spop
    twoPopSs();
    rD = rA;
    rA = rB;
    rB = rD;
    twoPushDs();
  }

  private final void iSpop3() {
    // ( -- n1 n2 n3 ) [ n1 n2 n3 -- ] Note: NOT same as spop spop spop
    threePopSs();
    rD = rC;
    rC = rA;
    rA = rD;
    threePushDs();
  }

  private final void iSpop4() {
    // ( -- n1 n2 n3 n4 ) [ n1 n2 n3 n4 -- ] Note: NOT like spop x 4
    fourPopSs();
    {
      int buf = rA;
      rA = rD;
      rD = buf;
      buf = rB;
      rB = rC;
      rC = buf;
    }
    fourPushDs();
  }

  private final void iSpush() {
    // ( n -- ) [ -- n ]
    popDs();
    pushSs();
  }

  private final void iSpush2() {
    // ( n1 n2 -- ) [ -- n1 n2 ] Note: NOT same as spush spush
    twoPopDs();
    // Reverse rA, rB 'order'
    rD = rA;
    rA = rB;
    rB = rD;
    twoPushSs();
  }

  private final void iSpush3() {
    // ( n1 n2 n3 -- ) [ -- n1 n2 n3 ] Note: NOT same as spush spush spush
    threePopDs();
    // Reverse rA, rB, rC 'order'
    rD = rA;
    rA = rC;
    rC = rD;
    threePushSs();
  }

  private final void iSpush4() {
    // ( n1 n2 n3 n4 -- ) [ -- n1 n2 n3 n4 ] Note: NOT like spush x 4
    fourPopDs();
    {
      int buf = rA;
      rA = rD;
      rD = buf;
      buf = rB;
      rB = rC;
      rC = buf;
    }
    fourPushSs();
  }

  private final void iHold() {
    // ( n -- n ) [ -- n ]
    peekDs();
    pushSs();
  }

  private final void iHold2() {
    // ( n1 n2 -- n1 n2 ) [ -- n1 n2 ]
    twoPeekDs();
    twoPushSs();
  }

  private final void iHold3() {
    // ( n1 n2 n3 -- n1 n2 n3 ) [ -- n1 n2 n3 ]
    threePeekDs();
    threePushSs();
  }

  private final void iHold4() {
    // ( n1 n2 n3 n4 -- n1 n2 n3 n4 ) [ -- n1 n2 n3 n4 ]
    fourPeekDs();
    fourPushSs();
  }

  private final void iRpop() {
    // ( -- n ) { n -- }
    popRs();
    pushDs();
  }

  private final void iRpush() {
    // ( n -- ) { -- n }
    popDs();
    pushRs();
  }

  private final void iBrgez() {
    // ( n -- n )
    peekDs();
    if (rA < 0) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iBrlz() {
    // ( n -- n )
    peekDs();
    if (rA >= 0) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iBrlez() {
    // ( n -- n )
    peekDs();
    if (rA > 0) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iBrnz() {
    // ( n -- n )
    peekDs();
    if (rA == 0) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iBrz() {
    // ( n -- n )
    peekDs();
    if (rA != 0) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iBrg() {
    // ( n1 n2 -- n1 )
    peekSecondDs();
    popDs();
    if (rB <= rA) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iBrge() {
    // ( n1 n2 -- n1 )
    peekSecondDs();
    popDs();
    if (rB < rA) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iBrl() {
    // ( n1 n2 -- n1 )
    peekSecondDs();
    popDs();
    if (rB >= rA) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iBrle() {
    // ( n1 n2 -- n1 )
    peekSecondDs();
    popDs();
    if (rB > rA) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iBrne() {
    // ( n1 n2 -- n1 )
    peekSecondDs();
    popDs();
    if (rB == rA) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iBre() {
    // ( n1 n2 -- n1 )
    peekSecondDs();
    popDs();
    if (rB != rA) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iJgz() {
    // ( n -- )
    popDs();
    if (rA <= 0) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iJgez() {
    // ( n -- )
    popDs();
    if (rA < 0) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iJlz() {
    // ( n -- )
    popDs();
    if (rA >= 0) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iJlez() {
    // ( n -- )
    popDs();
    if (rA > 0) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iJz() {
    // ( n -- )
    popDs();
    if (rA != 0) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iJnz() {
    // ( n -- )
    popDs();
    if (rA == 0) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iJg() {
    // ( n1 n2 -- )
    twoPopDs();
    if (rA <= rB) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iJge() {
    // ( n1 n2 -- )
    twoPopDs();
    if (rA < rB) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iJl() {
    // ( n1 n2 -- )
    twoPopDs();
    if (rA >= rB) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iJle() {
    // ( n1 n2 -- )
    twoPopDs();
    if (rA > rB) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iJe() {
    // ( n1 n2 -- )
    twoPopDs();
    if (rA != rB) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iJne() {
    // ( n1 n2 -- )
    twoPopDs();
    if (rA == rB) {
      dontBranch();
    } else {
      branch();
    }
    ;
  }

  private final void iDjmp() {
    // ( a -- )
    popDs(); // rA now contains jump address
    // Ensure dynamic jump address lies within program memory
    if ((rA < 0) || (rA > HIGHEST_WRITABLE_WORD)) {
      trapPcOverflow();
    }
    pc = rA;
  }

  private final void iDecw() {
    // ( n -- n-WORD_SIZE )
    peekDs();
    if (rA < NEG_INT_MAX + WORD_SIZE) {
      trapMathOverflow();
    }
    rA = rA - WORD_SIZE;
    replaceDs();
  }

  private final void iDec2w() {
    // ( n -- n-TWO_WORDS_SIZE )
    peekDs();
    if (rA < NEG_INT_MAX + TWO_WORDS_SIZE) {
      trapMathOverflow();
    }
    rA = rA - TWO_WORDS_SIZE;
    replaceDs();
  }

  private final void iInc() {
    // ( n -- n+1 )
    peekDs();
    if (rA == POS_INT_MAX) {
      trapMathOverflow();
    }
    rA++;
    replaceDs();
  }

  private final void iInc2() {
    // ( n -- n+WORD_SIZE )
    peekDs();
    if (rA > POS_INT_MAX - WORD_SIZE) {
      trapMathOverflow();
    }
    rA = rA + WORD_SIZE;
    replaceDs();
  }

  private final void iInc2w() {
    // ( n -- n+TWO_WORDS_SIZE )
    peekDs();
    if (rA > POS_INT_MAX - TWO_WORDS_SIZE) {
      trapMathOverflow();
    }
    rA = rA + TWO_WORDS_SIZE;
    replaceDs();
  }

  private final void iDup() {
    // ( n -- n n )
    peekDs();
    pushDs();
  }

  private final void iDup2() {
    // ( n1 n2 -- n1 n2 n1 n2 )
    twoPeekDs();
    twoPushDs();
  }

  private final void iDup3() {
    threePeekDs();
    threePushDs();
  }

  private final void iDup4() {
    fourPeekDs();
    fourPushDs();
  }

  private final void iSwap() {
    twoPeekDs();
    rC = rA;
    rA = rB;
    rB = rC;
    twoReplaceDs();
  }

  private final void iOver() {
    peekSecondDs();
    rA = rB;
    pushDs();
  }

  private final void iNip() {
    if (dsp > DS_HAS_TWO) {
      trapDsUnderflow();
    }
    peekDs();
    dsp++;
    ds[dsp] = rA;
  }

  private final void iTuck() {
    twoPeekDs();
    rC = rA;
    rA = rB;
    rB = rC;
    twoReplaceDs();
    peekSecondDs();
    rA = rB;
    pushDs();
  }

  private final void iRot() {
    threePeekDs();
    threeReplaceDs();
  }

  private final void iLeap() {
    peekThirdDs();
    rA = rC;
    pushDs();
  }

  private final void iTor() {
    threePeekDs();
    threeRReplaceDs();
  }

  private final void iAnd() {
    twoPopDs();
    rA = rA & rB;
    pushDs();
  }

  private final void iRand() {
    twoPeekDs();
    rA = rA & rB;
    pushDs();
  }

  private final void iOr() {
    twoPopDs();
    rA = rA | rB;
    pushDs();
  }

  private final void iRor() {
    // ( n1 n2 -- n1 n2 n2|n1 )
    twoPeekDs();
    rA = rA | rB;
    pushDs();
  }

  private final void iXor() {
    // ( n1 n2 -- n1^n2 )
    twoPopDs();
    rA = rA ^ rB;
    pushDs();
  }

  private final void iRxor() {
    twoPeekDs();
    rA = rA ^ rB;
    pushDs();
  }

  private final void iAdd() {
    // ( n1 n2 -- n1+n2 )
    //   FIXME Needs further testing and further optimization.
    // WARNING: This arithmetic operation has not been comprehensively
    // tested and therefore might possibly be incorrect for some
    // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
    // NOTE: This is a direct translation of the logic used in the
    // C FVM implementation. Since Java supports 64-bit longs
    // it might be better to refactor this to use widening
    // to detect overflow in a simpler manner.
    twoPopDs();
    if ((rA > 0) == (rB > 0)) {
      // Overflow is possible (both same sign)
      if (rA > 0) {
        // Both positive and > 0
        if ((POS_INT_MAX - rA) < rB) {
          trapMathOverflow();
        }
      } else {
        // Both negative or 0
        if ((NEG_INT_MAX - rA) > rB) {
          trapMathOverflow();
        }
      }
    }
    rA = rA + rB;
    pushDs();
  }

  private final void iRadd() {
    // ( n1 n2 -- n1 n2+n1 )
    //   FIXME Needs further testing and further optimization.
    // WARNING: This arithmetic operation has not been comprehensively
    // tested and therefore might possibly be incorrect for some
    // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
    // NOTE: This is a direct translation of the logic used in the
    // C FVM implementation. Since Java supports 64-bit longs
    // it might be better to refactor this to use widening
    // to detect overflow in a simpler manner.
    twoPeekDs();
    if ((rA > 0) == (rB > 0)) {
      // Overflow is possible (both same sign)
      if (rA > 0) {
        // Both positive and > 0
        if ((POS_INT_MAX - rA) < rB) {
          trapMathOverflow();
        }
      } else {
        // Both negative or 0
        if ((NEG_INT_MAX - rA) > rB) {
          trapMathOverflow();
        }
      }
    }
    rA = rA + rB;
    twoReplaceDs();
  }

  private final void iDiv() {
    // ( n1 n2 -- n1/n2 )
    twoPopDs();
    // Do not allow division by zero
    if (rB == 0) {
      trapDivideByZero();
    }
    // Do not allow division of NEG_INT_MAX by -1
    if ((rA == NEG_INT_MAX) && (rB == -1)) {
      trapMathOverflow();
    }
    rA = rA / rB;
    pushDs();
  }

  private final void iRdiv() {
    // ( n1 n2 -- n1 n2/n1 )
    twoPeekDs();
    // Do not allow division by zero
    if (rB == 0) {
      trapDivideByZero();
    }
    // Do not allow division of NEG_INT_MAX by -1
    if ((rA == NEG_INT_MAX) && (rB == -1)) {
      trapMathOverflow();
    }
    rA = rA / rB;
    twoReplaceDs();
  }

  private final void iMod() {
    // ( n1 n2 -- n1%n2 )
    twoPopDs();
    // Do not allow division by zero
    if (rB == 0) {
      trapDivideByZero();
    }
    // Do not allow division of NEG_INT_MAX by -1
    if ((rA == NEG_INT_MAX) && (rB == -1)) {
      trapMathOverflow();
    }
    rA = rA % rB;
    pushDs();
  }

  private final void iRmod() {
    // ( n1 n2 -- n1 n2%n1 )
    twoPeekDs();
    // Do not allow division by zero
    if (rB == 0) {
      trapDivideByZero();
    }
    // Do not allow division of NEG_INT_MAX by -1
    if ((rA == NEG_INT_MAX) && (rB == -1)) {
      trapMathOverflow();
    }
    rA = rA % rB;
    twoReplaceDs();
  }

  private final void iDivmod() {
    // ( n1 n2 -- n1/n2 n1%n2 )
    //   FIXME Needs further testing and further optimization.
    // WARNING: This arithmetic operation has not been comprehensively
    // tested and therefore might possibly be incorrect for some
    // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
    // NOTE: This is a direct translation of the logic used in the
    // C FVM implementation. Since Java supports 64-bit longs
    // it might be better to refactor this to use widening
    // to detect overflow in a simpler manner.
    twoPopDs();
    // Do not allow division by zero
    if (rB == 0) {
      trapDivideByZero();
    }
    // Do not allow division of NEG_INT_MAX by -1
    if ((rA == NEG_INT_MAX) && (rB == -1)) {
      trapMathOverflow();
    }
    rC = rA; // Preserve original rA
    rA = rA / rB;
    pushDs();
    rA = rC; // Restore original rA
    rA = rA % rB;
    pushDs();
  }

  private final void iMul() {
    // ( n1 n2 -- n1*n2 ) ---------------------------------
    //   FIXME Needs further testing and further optimization.
    // WARNING: This arithmetic operation has not been comprehensively
    // tested and therefore might possibly be incorrect for some
    // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
    // NOTE: This is a direct translation of the logic used in the
    // C FVM implementation. Since Java supports 64-bit longs
    // it might be better to refactor this to use widening
    // to detect overflow in a simpler manner.
    twoPopDs();
    if (rA == 0) {
      pushDs();
      return;
    } else if (rB == 0) {
      rA = 0;
      pushDs();
      return;
    } else if (rA == 1) {
      rA = rB;
      pushDs();
      return;
    } else if (rB == 1) {
      pushDs();
      return;
    } else if (rA > 0) { // At this point neither rA nor rB are 0 or 1
      if (rB > 0) {
        // Both >= 2
        if ((POS_INT_MAX / rB) < rA) {
          trapMathOverflow();
        } else {
          rA = rA * rB;
          pushDs();
          return;
        }
      } else {
        // rA >= 2, rB negative
        if ((0 - (POS_INT_MAX / rB)) < rA) {
          trapMathOverflow();
        } else {
          rA = rA * rB;
          pushDs();
          return;
        }
      }
    } else if (rB < 0) {
      // Both negative
      if ((NEG_INT_MAX == rA) || (NEG_INT_MAX == rB)) {
        trapMathOverflow();
      } else if ((POS_INT_MAX / rB) > rA) {
        trapMathOverflow();
      } else {
        rA = rA * rB;
        pushDs();
        return;
      }
    } else {
      // rB >=2, rA negative
      if ((0 - (POS_INT_MAX / rA)) < rB) {
        trapMathOverflow();
      } else {
        rA = rA * rB;
        pushDs();
        return;
      }
    } // -----------------------------------------------------------------
  }

  private final void iRmul() {
    // ( n1 n2 -- n1 n2*n1 )
    //   FIXME Needs further testing and further optimization.
    // WARNING: This arithmetic operation has not been comprehensively
    // tested and therefore might possibly be incorrect for some
    // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
    // NOTE: This is a direct translation of the logic used in the
    // C FVM implementation. Since Java supports 64-bit longs
    // it might be better to refactor this to use widening
    // to detect overflow in a simpler manner.
    twoPeekDs();
    if (rA == 0) {
      twoReplaceDs();
      return;
    } else if (rB == 0) {
      rA = 0;
      twoReplaceDs();
      return;
    } else if (rA == 1) {
      rA = rB;
      twoReplaceDs();
      return;
    } else if (rB == 1) {
      twoReplaceDs();
      return;
    } else if (rA > 0) { // At this point neither rA nor rB are 0 or 1
      if (rB > 0) {
        // Both >= 2
        if ((POS_INT_MAX / rB) < rA) {
          trapMathOverflow();
        } else {
          rA = rA * rB;
          twoReplaceDs();
          return;
        }
      } else {
        // rA >= 2, rB negative
        if ((0 - (POS_INT_MAX / rB)) < rA) {
          trapMathOverflow();
        } else {
          rA = rA * rB;
          twoReplaceDs();
          return;
        }
      }
    } else if (rB < 0) {
      // Both negative
      if ((NEG_INT_MAX == rA) || (NEG_INT_MAX == rB)) {
        trapMathOverflow();
      } else if ((POS_INT_MAX / rB) > rA) {
        trapMathOverflow();
      } else {
        rA = rA * rB;
        twoReplaceDs();
        return;
      }
    } else {
      // rB >=2, rA negative
      if ((0 - (POS_INT_MAX / rA)) < rB) {
        trapMathOverflow();
      } else {
        rA = rA * rB;
        twoReplaceDs();
        return;
      }
    } // -----------------------------------------------------------------
  }

  private final void iSub() {
    // ( n1 n2 -- n1-n2 )
    //   FIXME Needs further testing and further optimization.
    // WARNING: This arithmetic operation has not been comprehensively
    // tested and therefore might possibly be incorrect for some
    // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
    // NOTE: This is a direct translation of the logic used in the
    // C FVM implementation. Since Java supports 64-bit longs
    // it might be better to refactor this to use widening
    // to detect overflow in a simpler manner.
    twoPopDs();
    if ((rA > 0) != (rB > 0)) {
      // Overflow is possible (signs are opposite)
      if (rA > 0) {
        // rA positive, rB <=0
        if (((POS_INT_MAX - rA) + rB) < 0) {
          trapMathOverflow();
        }
      } else {
        // rB positive, rA <=0
        if ((rA - NEG_INT_MAX) < rB) {
          trapMathOverflow();
        }
      }
    }
    rA = rA - rB;
    pushDs();
  }

  private final void iRsub() {
    // ( n1 n2 -- n1 n2-n1 )
    //   FIXME Needs further testing and further optimization.
    // WARNING: This arithmetic operation has not been comprehensively
    // tested and therefore might possibly be incorrect for some
    // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
    // NOTE: This is a direct translation of the logic used in the
    // C FVM implementation. Since Java supports 64-bit longs
    // it might be better to refactor this to use widening
    // to detect overflow in a simpler manner.
    twoPeekDs();
    if ((rA > 0) != (rB > 0)) {
      // Overflow is possible (signs are opposite)
      if (rA > 0) {
        // rA positive, rB <=0
        if (((POS_INT_MAX - rA) + rB) < 0) {
          trapMathOverflow();
        }
      } else {
        // rB positive, rA <=0
        if ((rA - NEG_INT_MAX) < rB) {
          trapMathOverflow();
        }
      }
    }
    rA = rA - rB;
    twoReplaceDs();
  }

  private final void iShl() {
    // ( n1 n2 -- n1<<n2 )
    twoPopDs();
    if (rB > 0x1f) {
      trapXsBitshift();
    }
    rA = rA << rB;
    pushDs();
  }

  private final void iRshl() {
    // ( n1 n2 -- n1 n2 n1<<n2 )
    twoPeekDs();
    if (rA > 0x1f) {
      trapXsBitshift();
    }
    rB = rB << rA;
    rA = rB;
    pushDs();
  }

  private final void iShr() {
    // ( n1 n2 -- n1>>n2 )
    {
      twoPopDs();
      if (rB > 0x1f) {
        trapXsBitshift();
      }
      rA = rA >>> rB;
      pushDs();
    }
  }

  private final void iRshr() {
    // ( n1 n2 -- n1 n2 n1>>n2 )
    {
      twoPeekDs();
      if (rA > 0x1f) {
        trapXsBitshift();
      }
      rB = rB >>> rA;
      rA = rB;
      pushDs();
    }
  }

  private final void iNeg() {
    // ( n1 -- n1*-1 )
    popDs();
    // Do not allow negation of NEG_INT_MAX
    if (rA == NEG_INT_MAX) {
      trapMathOverflow();
    }
    rA = rA * -1;
    pushDs();
  }

  private final void iAbs() {
    // ( n1 -- |n1| )
    peekDs();
    if (rA < 0) {
      // Do not allow negation of NEG_INT_MAX
      if (rA == NEG_INT_MAX) {
        trapMathOverflow();
      }
      rA = rA * -1;
      replaceDs();
    }
  }

  private final void iRdivmod() {
    // ( n1 n2 -- n1 n2/n1 n2%n1 )
    //   FIXME Needs further testing and further optimization.
    // WARNING: This arithmetic operation has not been comprehensively
    // tested and therefore might possibly be incorrect for some
    // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
    // NOTE: This is a direct translation of the logic used in the
    // C FVM implementation. Since Java supports 64-bit longs
    // it might be better to refactor this to use widening
    // to detect overflow in a simpler manner.
    twoPeekDs();
    // Do not allow division by zero
    if (rB == 0) {
      trapDivideByZero();
    }
    // Do not allow division of NEG_INT_MAX by -1
    if ((rA == NEG_INT_MAX) && (rB == -1)) {
      trapMathOverflow();
    }
    rC = rA; // Preserve original rA
    rA = rA / rB;
    replaceDs();
    rA = rC; // Restore original rA
    rA = rA % rB;
    pushDs();
  }

  // ===========================================================================
  // ENTRY POINT
  // ===========================================================================
  private final void systemInitDevices() {
    openStdin();
    openStdout();
    openStdtrc();
    openStdblk();
    openStdexp();
    openStdimp();
    setIOdefaults();
  }

  private final void nextInstruction() {

    // nextInstruction: // Begin Freeputer program execution
    while (keepRunning) {
      if (TRON_ENABLED) {
        optTrace();
      }

      rA = memory.getInt(pc);
      incPc();

      switch (rA) {

      case iLIT: // ( -- n )
        iLit();
        break;
      case iCALL: // ( -- )
        iCall();
        break;
      case iDEC:
        ; // ( n -- n-1 )
        iDec();
        break;
      case iBRGZ: // ( n -- n )
        iBrgz();
        break;
      case iDROP: // ( n -- )
        dropDs();
        break;
      case iHALT: // ( -- )
        exitSuccess();
        break;
      case iWALL: // ( -- )
        trapWall();
        break;
      case iDATA: // ( -- )
        trapData();
        break;
      case iTROFF:
        iTroff();
        break;
      case iTRON:
        iTron();
        break;
      case iWCHAN:
        iWchan();
        break;
      case iRCHAN:
        iRchan();
        break;
      case iGCHAN:
        iGchan();
        break;
      case iPCHAN:
        iPchan();
        break;
      case iHOMIO:
        setIOdefaults();
        break;
      case iECODE:
        iEcode();
        break;
      case iRCODE:
        iRcode();
        break;
      case iROM:
        iRom();
        break;
      case iRAM:
        iRam();
        break;
      case iMAP:
        iMap();
        break;
      case iSTDBLK:
        iStdblk();
        break;
      case iDS:
        iDs();
        break;
      case iSS:
        iSs();
        break;
      case iRS:
        iRs();
        break;
      case iDSN:
        iDsn();
        break;
      case iSSN:
        iSsn();
        break;
      case iRSN:
        iRsn();
        break;
      case iREV: // ( n1 n2 n3 -- n3 n2 n1 )
        iRev();
        break;
      case iGETOR:
        iGetor();
        break;
      case iGETORB:
        iGetorb();
        break;
      case iPUTOR:
        iPutor();
        break;
      case iPUTORB:
        iPutorb();
        break;
      case iREADOR:
        ;
        iReador();
        break;
      case iREADORB:
        ;
        iReadorb();
        break;
      case iWRITOR:
        iWritor();
        break;
      case iWRITORB:
        iWritorb();
        break;
      case iTRACOR:
        iTracor();
        break;
      case iTRACORB:
        iTracorb();
        break;
      case iFIND:
        iFind();
        break;
      case iFINDB:
        iFindb();
        break;
      case iFILL:
        iFill();
        break;
      case iFILLB:
        iFillb();
        break;
      case iMATCH:
        iMatch();
        break;
      case iMATCHB:
        iMatchb();
        break;
      case iMOVEB:
        iMoveb();
        break;
      case iMOVE:
        iMove();
        break;
      case iLOADB:
        iLoadb();
        break;
      case iRLOADB: // ( a -- a byte )
        iRloadb();
        break;
      case iLOAD: // ( a -- word )
        iLoad();
        break;
      case iRLOAD: // ( a -- a word )
        iRload();
        break;
      case iPLOAD:
        iPload();
        break;
      case iRPLOAD:
        iRpload();
        break;
      case iPLOADB:
        iPloadb();
        break;
      case iRPLOADB:
        iRploadb();
        break;
      case iSTORE:
        iStore();
        break;
      case iSTOREB:
        iStoreb();
        break;
      case iPSTORE:
        iPstore();
        break;
      case iPSTOREB:
        iPstoreb();
        break;
      case iREBOOT:
        iReboot();
        break;
      case iRESET:
        iReset();
        break;
      case iDCALL:
        iDcall();
        break;
      case iRDCALL:
        iRdcall();
        break;
      case iEXIT:
        iExit();
        break;
      case iSPOP:
        iSpop();
        break;
      case iSPEEK:
        iSpeek();
        break;
      case iSPEEK2:
        iSpeek2();
        break;
      case iSPEEK3:
        iSpeek3();
        break;
      case iSPEEK4:
        iSpeek4();
        break;
      case iSPOP2:
        iSpop2();
        break;
      case iSPOP3:
        iSpop3();
        break;
      case iSPOP4:
        iSpop4();
        break;
      case iSPUSH:
        iSpush();
        break;
      case iSPUSH2:
        iSpush2();
        break;
      case iSPUSH3:
        iSpush3();
        break;
      case iSPUSH4:
        iSpush4();
        break;
      case iHOLD:
        iHold();
        break;
      case iHOLD2:
        iHold2();
        break;
      case iHOLD3:
        iHold3();
        break;
      case iHOLD4:
        iHold4();
        break;
      case iRPOP:
        iRpop();
        break;
      case iRPUSH:
        iRpush();
        break;
      case iBRGEZ:
        iBrgez();
        break;
      case iBRLZ:
        iBrlz();
        break;
      case iBRLEZ:
        iBrlez();
        break;
      case iBRNZ:
        iBrnz();
        break;
      case iBRZ:
        iBrz();
        break;
      case iBRG:
        iBrg();
        break;
      case iBRGE:
        iBrge();
        break;
      case iBRL:
        iBrl();
        break;
      case iBRLE:
        iBrle();
        break;
      case iBRNE:
        iBrne();
        break;
      case iBRE:
        iBre();
        break;
      case iJGZ:
        iJgz();
        break;
      case iJGEZ:
        iJgez();
        break;
      case iJLZ:
        iJlz();
        break;
      case iJLEZ:
        iJlez();
        break;
      case iJZ:
        iJz();
        break;
      case iJNZ:
        iJnz();
        break;
      case iJG:
        iJg();
        break;
      case iJGE:
        iJge();
        break;
      case iJL:
        iJl();
        break;
      case iJLE:
        iJle();
        break;
      case iJE:
        iJe();
        break;
      case iJNE:
        iJne();
        break;
      case iJMP: // ( -- )
        branch();
        break;
      case iDJMP:
        iDjmp();
        break;
      case iDECW:
        iDecw();
        break;
      case iDEC2W:
        iDec2w();
        break;
      case iINC:
        iInc();
        break;
      case iINCW:
        iInc2();
        break;
      case iINC2W:
        iInc2w();
        break;
      case iDROP2: // ( n1 n2 -- )
        twoDropDs();
        break;
      case iDROP3: // ( n1 n2 n3 -- )
        threeDropDs();
        break;
      case iDROP4: // ( n1 n2 n3 n4 -- )
        fourDropDs();
        break;
      case iDUP:
        iDup();
        break;
      case iDUP2: // ( n1 n2 -- n1 n2 n1 n2 )
        iDup2();
        break;
      case iDUP3: // ( n1 n2 n3 -- n1 n2 n3 n1 n2 n3 )
        iDup3();
        break;
      case iDUP4: // ( n1 n2 n3 n4 -- n1 n2 n3 n4 n1 n2 n3 n4 )
        iDup4();
        break;
      case iSWAP: // ( n1 n2 -- n2 n1 )
        iSwap();
        break;
      case iOVER: // ( n1 n2 -- n1 n2 n1 )
        iOver();
        break;
      case iNIP: // ( n1 n2 -- n2 )
        iNip();
        break;
      case iTUCK: // ( n1 n2 -- n2 n1 n2 )
        iTuck();
        break;
      case iROT: // ( n1 n2 n3 -- n2 n3 n1 )
        iRot();
        break;
      case iLEAP: // ( n1 n2 n3 -- n1 n2 n3 n1 )
        iLeap();
        break;
      case iTOR: // ( n1 n2 n3 -- n3 n1 n2 )
        iTor();
        break;
      case iAND: // ( n1 n2 -- n1&n2 )
        iAnd();
        break;
      case iRAND: // ( n1 n2 -- n1 n2 n2&n1 )
        iRand();
        break;
      case iOR: // ( n1 n2 -- n1|n2 )
        iOr();
        break;
      case iROR:
        iRor();
        break;
      case iXOR:
        iXor();
        break;
      case iRXOR: // ( n1 n2 -- n1 n2 n2^n1 )
        iRxor();
        break;
      case iADD:
        iAdd();
        break;
      case iRADD:
        iRadd();
        break;
      case iDIV:
        iDiv();
        break;
      case iRDIV:
        iRdiv();
        break;
      case iMOD:
        iMod();
        break;
      case iRMOD:
        iRmod();
        break;
      case iDIVMOD:
        iDivmod();
        break;
      case iMUL:
        iMul();
        break;
      case iRMUL:
        iRmul();
        break;
      case iSUB:
        iSub();
        break;
      case iRSUB:
        iRsub();
        break;
      case iSHL:
        iShl();
        break;
      case iRSHL:
        iRshl();
        break;
      case iSHR:
        iShr();
        break;
      case iRSHR:
        iRshr();
        break;
      case iNEG:
        iNeg();
        break;
      case iABS:
        iAbs();
        break;
      case iRDIVMOD:
        iRdivmod();
        break;
      default:
        trapIllegalOpcode();
        break;
      }
    } // end of while (equivalent to goto nextInstruction)

  }

/*
  private final void systemCopyProgram() { 
   // Uncomment systemCopyProgram logic to run the example program hardcoded above
   //  (see "EXAMPLE OF INDIRECT THREADED PROGRAM" section above).
   // This is an (unusual) alternative to systemLoadProgram below.
   // -----------------------------------------------------------------------
	  // Copy program into system memory
	  for( int i=0 ; i < PROGRAM_SIZE; i++) {
		  memory.putInt((i*WORD_SIZE), itProg[i]);
	  };
   // ----------------------------------------------------------------------- 
  }
*/

  private final void systemLoadProgram() {
    // Uncomment systemLoadProgram logic to load program from ROM file (as usual).
    // This is an alternative to (the much more unusual) systemCopyProgram above.
	// -----------------------------------------------------------------------
    openRom();
    try {
      int nRead = 0;
      int inByte = 0;
      while (inByte != -1) {
        inByte = romStream.read();
        memory.put((byte) inByte);
        nRead++;
        if (nRead > ROM_SIZE) {
          trapCantReadRom(); // ROM file too big for FVM instance ROM
        }
      }
    } catch (IOException e1) {
      trapCantReadRom(); // Reading ROM file contents failed
    }
    closeRom();
    // -----------------------------------------------------------------------
  }

  private final void systemInitCore() {
    // systemInitCore:
    dsp = DS_EMPTY; // Initialize data stack pointer
    rsp = RS_EMPTY; // Initialize return stack pointer
    ssp = SS_EMPTY; // Initialize software stack pointer
    pc = 0; // Set program counter to start of program
  }

  /**
   * Gets the <code>systemExitCode</code> for this FVM instance.
   * 
   * A null <code>systemExitCode</code> indicates that the FVM instance has not yet
   * completed its run. 0 indicates success. A non-zero value indicates failure.
   * See the "TRAPS" section of this source code for those failure codes.
   * @return an Integer systemExitCode or null
   */
  public Integer getSystemExitCode() {
    return this.systemExitCode;
  }

  /**
   * <p>Run this FVM instance.</p>
   * <p>To run synchronously use:</p>
   * <pre><code>
   * FVM fvm = new FVM();
   * fvm.run();
   * </code></pre>
   * 
   * <p>To run asynchronously use:</p>
   * <pre><code>
   * FVM fvm = new FVM();
   * new Thread(fvm).start();
   * </code></pre>
   */
  @Override
  public final void run() {
    
    try {
        // Attempt (once only) to clearStateAndInitialize
        clearStateAndInitialize();

        // Attempt (possibly repeatedly) to load and run the program
        while (keepRunning) {
          try {
         // systemCopyProgram(); // Uncomment either this line or next line
            systemLoadProgram();
            systemInitCore();
            nextInstruction();
          } catch (IllegalStateException e) {
              // We got here from a soft reset, a hard reset or a system exit.
              // If the FVM is configured to exit upon trap then a
              // system exit will have occurred and keepRunning will be false.
              // Otherwise the FVM has reset, keepRunning is true,
              // and we now restart so as to run again.
        	  
        	  // Only uncomment the next line when debugging the FVM
        	  //e.printStackTrace();
          }
        }
    } catch (IllegalStateException e) {
        // Our attempt to clearStateAndInitialize failed.
    }

  } // end of runfvm()

  // =========================================================================
  // SYSTEM RESET
  // =========================================================================
  private final void systemReset() {
    lastExitCode = rB; // Save lastExitCode (passed in here in rB)
    if (stdblkChannel != null) {
      closeStdblk(); // Close the standard block device
    }
    if (stdexpChannel != null) {
      closeStdexp(); // Close stdexp
    }
    if (stdimpChannel != null) {
      closeStdimp(); // Close stdimp
    }
    if (stdtrcChannel != null) {
      closeStdtrc(); // Close stdtrc
    }
    if (stdoutStream != null) {
      closeStdout(); // Close stdin, Java version is explicit here
    }
    if (stdinStream != null) {
      closeStdin(); // Close stdout, Java version is explicit here
    }

    // The next line should normally be the uncommented one for most FVMs
    //   exitFail();        // Uncomment for exit with specific failure code
    //   exitFailGeneric(); // Uncomment for exit with generic failure code
    // The next line should be uncommented for fvm16-16MB-sr-append for fvmtest
    //   systemSoftReset(); // Uncomment for soft reset
    //   systemHardReset(); // Uncomment for hard reset
    // Or uncomment this if-else block for automatic configuration:
    if (softResetOnTrap) {
      systemSoftReset();
    } else {
      exitFail();
    }
  }

  // =========================================================================
  // EXIT POINTS
  // =========================================================================
  private final void exitSuccess() {
    rB = 0; // exitCode for success (Linux standard)
    systemExit();
  }

  private final void exitFail() {
    if (rB != 0) {
      systemExit(); // use exit code for specific failure (if any)
    }
  }

  @SuppressWarnings(value = { "unused" })
  private final void exitFailGeneric() {
    rB = 1; // exitCode for generic failure (Linux standard)
    systemExit();
  }

  private final void systemExit() { // Exit using exitCode in rB
    systemExitCode = rB;
    keepRunning = false;
    throw new IllegalStateException("systemExit with exit code:" + rB);
  }

  // =========================================================================
  // TRAPS
  // =========================================================================
  // -------------------------------------------------------------------------
  // TRAPS: ILLEGAL PROGRAM FLOW
  //
  // -------------------------------------------------------------------------
  private final void trapWall() {
    rB = 1;
    traceExit(msgTrapWall);
    systemReset();
  }

  private final void trapData() {
    rB = 2;
    traceExit(msgTrapData);
    systemReset();
  }

  private final void trapPcOverflow() {
    rB = 3;
    traceExitMsg(msgTrapPcOverflow);
    systemReset();
  }

  //
  // ------------------------------------------------------------------------
  // TRAPS: ILLEGAL OPCODES
  //
  // -------------------------------------------------------------------------
  private final void trapIllegalOpcode() {
    rB = 11;
    traceExit(msgTrapIllegalOpcode);
    systemReset();
  }

  //
  // -------------------------------------------------------------------------
  // TRAPS: ILLEGAL MATHEMATICAL OPERATIONS
  //
  // -------------------------------------------------------------------------
  private final void trapMathOverflow() {
    rB = 21;
    traceExit(msgTrapMathOverflow);
    systemReset();
  }

  private final void trapDivideByZero() {
    rB = 22;
    traceExit(msgTrapDivideByZero);
    systemReset();
  }

  private final void trapXsBitshift() {
    rB = 23;
    traceExit(msgTrapXsBitshift);
    systemReset();
  }

  //
  // -------------------------------------------------------------------------
  // TRAPS: ILLEGAL STACK OPERATIONS
  //
  // -------------------------------------------------------------------------
  private final void trapDsUnderflow() {
    rB = 31;
    traceExit(msgTrapDsUnderflow);
    systemReset();
  }

  private final void trapDsOverflow() {
    rB = 32;
    traceExit(msgTrapDsOverflow);
    systemReset();
  }

  private final void trapRsUnderflow() {
    rB = 33;
    traceExit(msgTrapRsUnderflow);
    systemReset();
  }

  private final void trapRsOverflow() {
    rB = 34;
    traceExit(msgTrapRsOverflow);
    systemReset();
  }

  private final void trapSsUnderflow() {
    rB = 35;
    traceExit(msgTrapSsUnderflow);
    systemReset();
  }

  private final void trapSsOverflow() {
    rB = 36;
    traceExit(msgTrapSsOverflow);
    systemReset();
  }

  // -------------------------------------------------------------------------
  // TRAPS: ILLEGAL MEMORY ACCESS
  // -------------------------------------------------------------------------
  private final void trapMemBounds() {
    rB = 41;
    traceExitMsg(msgTrapMemBounds);
    systemReset();
  }

  private final void trapRAMBounds() {
    rB = 42;
    traceExitMsg(msgTrapRAMBounds);
    systemReset();
  }

  // -------------------------------------------------------------------------
  // TRAPS: ROM
  // -------------------------------------------------------------------------
  // Note: a ROM file ('rom.fp') can be created using a Freelang compiler
  private final void trapCantOpenRom() {
    rB = 51;
    traceExitMsg(msgTrapCantOpenRom);
    exitFail();
  }

  private final void trapCantCloseRom() {
    rB = 52;
    traceExitMsg(msgTrapCantCloseRom);
    exitFail();
  }

  private final void trapCantReadRom() {
    rB = 53;
    traceExitMsg(msgTrapCantReadRom);
    exitFail();
  }

  // -------------------------------------------------------------------------
  // TRAPS: STDBLK
  // -------------------------------------------------------------------------
  // Note: a suitable zero-filled stdblk file ('std.blk') can be created on
  // Linux
  // by the following command (assuming STDBLK_SIZE is 16777216 bytes):
  // head -c 16777216 /dev/zero > std.blk
  // Note: to create a 'std.blk' file of 0 size on Linux simply use:
  // touch std.blk

  private final void trapCantOpenStdblk() {
  // to open and close
    rB = 61;
    traceExitMsg(msgTrapCantOpenStdblk);
    exitFail();
  }

  private final void trapCantCloseStdblk() {
    rB = 62;
    traceExitMsg(msgTrapCantCloseStdblk);
    exitFail();
  }

  // -------------------------------------------------------------------------
  // TRAPS: STREAMS
  // -------------------------------------------------------------------------
  // Note: this FVM will automatically create (or recreate) a 'std.trc' file
  // as it starts up; any previous data in that file will be lost
  private final void trapCantOpenStdtrc() {
    rB = 71;
    exitFail();
  }

  private final void trapCantCloseStdtrc() {
    rB = 72;
    exitFail();
  }

  private final void trapCantWriteToStdtrc() {
    rB = 73;
    exitFail();
  }

  // Note: this FVM will automatically create (or recreate) a 'std.exp' file
  // as it starts up; any previous data in that file will be lost
  private final void trapCantOpenStdexp() {
    rB = 74;
    traceExitMsg(msgTrapCantOpenStdexp);
    exitFail();
  }

  private final void trapCantCloseStdexp() {
    rB = 75;
    traceExitMsg(msgTrapCantCloseStdexp);
    exitFail();
  }

  // Note: to create a 'std.imp' file of 0 size on Linux simply use:
  // touch std.imp
  private final void trapCantOpenStdimp() {
    rB = 77;
    traceExitMsg(msgTrapCantOpenStdimp);
    exitFail();
  }

  private final void trapCantCloseStdimp() {
    rB = 78;
    traceExitMsg(msgTrapCantCloseStdimp);
    exitFail();
  }
  // =========================================================================

  /*
   * Zero-fill all variables holding FVM state except system memory and
   * lastExitCode
   */
  void clearState() {

    rchannel = 0;
    wchannel = 0;
    gchannel = 0;
    pchannel = 0;
    readBuf.putInt(0, 0); // zero the buffer
    readBuf.clear(); // reset the buffer
    writeBuf.putInt(0, 0); // zero the buffer
    writeBuf.clear(); // reset the buffer
    getBuf.putInt(0, 0); // zero the buffer
    getBuf.clear(); // reset the buffer
    putBuf.putInt(0, 0); // zero the buffer
    putBuf.clear(); // reset the buffer
    readbBuf.put(0, (byte) 0); // zero the buffer
    readbBuf.clear(); // reset the buffer
    writebBuf.put(0, (byte) 0); // zero the buffer
    writebBuf.clear(); // reset the buffer
    getbBuf.put(0, (byte) 0); // zero the buffer
    getbBuf.clear(); // reset the buffer
    putbBuf.put(0, (byte) 0); // zero the buffer
    putbBuf.clear(); // reset the buffer

    stdblkChannel = null;
    romStream = null;
    stdtrcChannel = null;
    stdexpChannel = null;
    stdimpChannel = null;

    // Java version makes these explicit
    stdinStream = null;
    stdoutStream = null;

    // pcTmp = 0;

    rsp = RS_EMPTY;
    for (int i = 0; i < (rsStop + 1); i++) {
      rs[i] = 0;
    }

    ssp = SS_EMPTY;
    for (int i = 0; i < (ssStop + 1); i++) {
      ss[i] = 0;
    }

    dsp = DS_EMPTY;
    for (int i = 0; i < (dsStop + 1); i++) {
      ds[i] = 0;
    }
  }

  // Zero-fill all FVM system memory
  private final void clearMem() {
    for (int i = 0; i < ((ROM_SIZE + RAM_SIZE + MAP_SIZE) / WORD_SIZE); i++) {
      memory.putInt(i, 0); // Slow looping but uses less JVM memory
    }
  }

  /**
   * <p>
   * Creates and runs a small, simple, default FVM instance.
   * </p>
   * <p>
   * For that instance to run successfully there
   * must locally exist 'rom.fp' (containing your compiled Freelang program),
   * 'std.blk' (an empty file) and 'std.imp' (an empty file)
   * in the directory from which you invoke java.
   * </p>
   * <p>
   * Modify this method to experiment with creating and running
   * more sophisticated FVM instances synchronoulsy or asynchronously.
   * </p>
   * 
   * @param args (not used)
   */
  public static void main(String[] args) {

  // Uncomment only 1 of the next 4 lines
  // FVM fvm = FVM.getFvmtestInstance();           // For running fvmtest suite
  // FVM fvm = new FVM(0x01000000,0);              // For fvm16_0MB
  // FVM fvm = new FVM(0x01000000,0x01000000);     // For fvm16_16MB
     FVM fvm = new FVM();                          // For fvm16_0kB
	 fvm.run();
	 Integer outcome = fvm.getSystemExitCode();
	 System.exit(outcome);
	  
	  
	// Note: when running any FVM instance there must be locally present
	// at least an appropriate 'rom.fp' (containing the Freelang program to
	// be run by your FVM instance) and a suitable 'std.blk' and 'std.imp'
	 
	// Note: when running the fvmtest suite there must be locally present
	// a 'rom.fp' (containing the compiled fvmtest program itself) and
	// the exact versions of 'std.blk', 'std.imp' and 'std.in.tst' expected
	// by fvmtest. For details see: 'dev/freelang/fvmtest/src/fvmtest.fl'
  }

  // =========================================================================
  // EXIT TRACING
  // =========================================================================
  // Send an error message to stdtrc
  // along with information regarding current program state.
  private final void traceExit(String msg) {
    try {
      if (TRON_ENABLED) {
        stdtrcChannel.printf("%s\n", msg);
        stdtrcChannel.printf("%s\n", msgBefore);
        traceInfo();
        traceStacks();
      } else {
        stdtrcChannel.printf("%s\n", msg);
      }
    } catch (Exception e) {
      trapCantWriteToStdtrc();
    }
  }

  private final void traceExitMsg(String msg) {
    try {
      if (TRON_ENABLED) {
        stdtrcChannel.printf("%s\n", msg);
        stdtrcChannel.printf("PC %08x\n", pc);
      } else {
        stdtrcChannel.printf("%s\n", msg);
      }
    } catch (Exception e) {
      trapCantWriteToStdtrc();
    }
  }
}
