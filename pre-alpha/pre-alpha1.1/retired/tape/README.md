<meta http-equiv="content-type" content="text/html;charset=utf-8">

# The Tape

A user interface for the future

## What is the tape?

The tape is a textual user interface (TUI). It has only one logical dimension: length (measured in cells). It is logically divided into consecutive cells (each of which can display one character) and the actual physical dimensions of a cell may vary from cell to cell. It has a logical cursor that points at the current cell. Although it has only one logical dimension it might physically be flexible and elastic and might be folded or curved into two or three dimensions in any configuration the user prefers. However, the tape is not aware of that.

The tape has no concept of rows, columns, lines, line width, font size, screen size, page size, screen resolution, pixels, pixel size, physical dimensions or multi-dimensional layout. It is just a sequence of cells. Although a tape might use colour, highlighting and different font sizes and styles as an aid to comprehension, the convention is that such things must not add information and that the application sending information to be displayed on the tape cannot control such things (other than perhaps to turn their use on or off globally). Thus a simple monochrome tape using one simple font would convey (almost if not entirely) the same information.

Applications must not make assumptions about tape length but may poll the tape at runtime to ascertain its length. By convention the entire length of a tape is assumed to be visible at once. Since techniques equivalent to scrolling or paging could be employed, a short tape is fully capable of conveying the same information as a long tape. This is because the tape has only one logical dimension. Applications must never make any assumptions regarding the actual physical geometry of the tape. For example, it is common to represent the tape as folded onto the screen of a two-dimensional display such that the tape wraps from one line to the next of that display but applications must not assume, require or utilize such geometry.

The tape is extremely portable (easily implemented on a wide variety of hardware) and is therefore recommended as the *default* user interface for all Freeputer applications. Importantly for freedom, it is very reasonable to assume that a minimal tape will easily be implemented on future hardware. Freeputer applications may optionally support sophisticated secondary display devices while using the tape as the default primary display.

By convention, a tape *must* as a minimum support 7-bit ASCII or similar; *optionally* it may also support the display of UTF-8 characters. At all times designers should take into account that for reasons of freedom it should be easy to implement a minimal tape on bare metal (such as on a microcontroller driving a small alphanumeric character LCD display) without requiring the support of any operating system. The tape is however also very well suited to large monitors and sophisticated displays running on powerful operating systems.

---

Copyright © Robert Gollagher 2016  

This document was written by Robert Gollagher.  
This document was first published on 12 March 2016.  
This document was last updated on 12 March 2016 at 22:39.  
This document is licensed under a [Creative Commons Attribution-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-sa/4.0/).


The official Freeputer website is [freeputer.net](http://www.freeputer.net).

---
