# zpacker - very simple LZ77-based compression

This is LZ77-based compression code with a focus on small files, and simplicity
of decompression. Packing (and depacking) code is done in C language, and
depacking in 68000 assembly language is also provided.

## Features

As it is focused on small files, no fancy features like 32-bit offsets or chunk
sizes. The packer will behave slightly better than lz4 on small files, but
significantly worse on larger files.

## File format

The compressed file format is a succession of two types of chunks:

 - _Literal chunks_ : Chunks containing not packed data. They consist in one header byte containing the `number of bytes - 1 + 0xc0`, then follows the corresponding number of bytes to be copied directly to the output byte stream. Such chunks contain between 1 and 64 uncompressed bytes.
 - _Offset chunks_ : Chunks describing a repetition of previous data. They consist in one header byte containing the `number of bytes - 4`, then an offset byte. Repetition size may vary between 4 and 195 bytes, and with offset values from -256 to -1.
