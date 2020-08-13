# `grit` - GBA Raster Image Transmogrifier

This program reads in a bitmap image and converts it to a format suitable for
GBA/NDS video games.
The bitmap parser has a buffer overflow bug.

Homepage: https://www.coranac.com/projects/grit/

This version has been modified to re-enable the custom image parsing code in
the `cldib` subdirectory, which is still present in the source distribution but
has since been superseded by the `FreeImage` library.

## Running

```
make
./grit fromager.bmp
./grit fromager-exploit.bmp
```

`fromager.bmp` is a simple test image.
`fromager-exploit.bmp` is a variant of the image that triggers the buffer
overflow bug.

## The bug

The BMP parser makes assumptions about the palette size based on the color
depth of the image.
For 24bpp images, it assumes the palette size is zero, and allocates the image
buffer accordingly.
However, it populates the palette by reading the number of palette entries
declared in the image header (`biClrUsed`), without checking that this matches
the assumed palette size that was used when allocating.
A malformed image that declares 24bpp color depth but has a non-zero
`biClrUsed` can read an arbitrary amount of data (up to the length of the file)
into the image buffer.

The image buffer is used to store both palette data and pixel data.
In our example image, `fromager.bmp`, the size of the pixel data is 0x3000
bytes.
To trigger the overflow, (1) we set `biClrUsed = 0x1004`, so it will attempt to
fill the palette with 0x300c bytes (each palette entry is 3 bytes), and (2) we
append 12 additional bytes to the file so it can satisfy a read of length
0x300c (normally the file ends immediately after the pixel data, which is only
0x3000 bytes in this case).
On Linux, the 12 byte overflow overwrites heap metadata and triggers an
assertion failure in the memory allocator.

Hex dumps of the original and modified images are available in `fromager.hex`
and `fromager-exploit.hex`.  Run `diff -u fromager.hex fromager-exploit.hex` to
see the changes.
