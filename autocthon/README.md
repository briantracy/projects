


# Technical Overview

## Address Space Layout

Do we separate code and data and symbols? Do symbols even need to be
mapped into the address space?

Code does not need to be mapped into the address space. I think the stack,
heap, and data all need to be in the address space. This way, all can be
readable and writeable and contiguous.

This means an "address" will either point to a data location or a code location. Thus, there are two address spaces, one for data and one for code. This could be a cool distinction. So `read 0x400` and `jump 0x400` will both be meaningful instructions.


## Control Transfer

Options for the `call` instruction:

1. `call <address>`: an absolute address, put in by the linker
2. `call <offset>`: for position-independent code. Offsets are computed by assembler for intra-object calls, and by the linker for inter-object calls.
3. `call <symbol-idx>`: symbol lookups are available at run time.
