#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Assert that the trace is valid only if `cond` is non-zero.
void __cc_flag_invalid(void);
// Indicate that the program has exhibited a bug if `cond` is non-zero.
void __cc_flag_bug(void);

// Assert that the trace is valid only if `cond` is non-zero.
void __cc_valid_if(int cond) {
    if (!cond) {
        __cc_flag_invalid();
    }
}

// Indicate that the program has exhibited a bug if `cond` is non-zero.
void __cc_bug_if(int cond) {
    if (cond) {
        __cc_flag_bug();
    }
}

// Allocate `size` words of memory.
uintptr_t* __cc_malloc(size_t size);
// Free the allocation starting at `ptr`.
void __cc_free(uintptr_t* ptr);

// Let the prover arbitrarily choose an address to poison in the range `start
// <= ptr < end`.  The prover returns `NULL` to indicate that nothing should be
// poisoned.
uintptr_t* __cc_advise_poison(uintptr_t* start, uintptr_t* end);

// Write `val` to `*ptr` and poison `*ptr`.  If `*ptr` is already poisoned, the
// trace is invalid.
void __cc_write_and_poison(uintptr_t* ptr, uintptr_t val);

// Allocate a block of `size` words.  (Actual `libc` malloc works in bytes.)
uintptr_t* malloc_words(size_t size) {
    uintptr_t* ptr = __cc_malloc(size);

    // Compute and validate the size of the allocation provided by the prover.
    uintptr_t addr = (uintptr_t)ptr;
    size_t region_size = 1ull << ((addr >> 58) & 63);
    // The allocated region must have space for `size` words, plus an
    // additional word for metadata.
    __cc_valid_if(region_size >= size + 1 && addr % region_size == 0);

    // Write 1 (allocated) to the metadata field, and poison it to prevent
    // tampering.  This will make the trace invalid if the metadata word is
    // already poisoned (this happens if the prover tries to return the same
    // region for two separate allocations).
    // region twice).
    uintptr_t* metadata = ptr + region_size - 1;
    __cc_write_and_poison(metadata, 1);

    // Choose a word to poison in the range `ptr .. metadata`.
    uintptr_t* poison = __cc_advise_poison(ptr + size, metadata);
    if (poison != NULL) {
        // The poisoned address must be in the unused space at the end of the
        // region.
        __cc_valid_if(ptr + size <= poison && poison < metadata);
        __cc_write_and_poison(poison, 0);
    }

    return ptr;
}

void free_words(uintptr_t* ptr) {
    if (ptr == NULL) {
        return;
    }

    // Get the allocation size.
    uintptr_t log_region_size = (uintptr_t)ptr >> 58;
    uintptr_t region_size = 1ull << log_region_size;

    // Ensure `ptr` points to the start of a region.
    __cc_bug_if((uintptr_t)ptr % region_size != 0);

    // Write to `*ptr`.  This memory access lets us catch double-free and
    // free-before-alloc by turning them into use-after-free and
    // use-before-alloc bugs, which we catch by other means.
    (*ptr) = 0;

    // We free only after the write, so the interpreter's fine-grained
    // allocation tracking doesn't flag it as a use-after-free.
    __cc_free(ptr);

    // Choose an address to poison.
    uintptr_t* metadata = ptr + region_size - 1;
    uintptr_t* poison = __cc_advise_poison(ptr, metadata);
    if (poison != NULL) {
        __cc_valid_if((uintptr_t)poison % sizeof(uintptr_t) == 0);
        // The pointer must be somewhere within the freed region.
        __cc_valid_if(ptr <= poison && poison < metadata);
        __cc_write_and_poison(poison, 0);
    }
}

void __llvm__memcpy__p0i8__p0i8__i64(uint8_t *dest, const uint8_t *src, uint64_t len) {
    for (uint64_t i = 0; i < len; ++i) {
      dest[i] = src[i];
    }
}

void __llvm__memset__p0i8__i64(uint8_t *dest, uint8_t val, uint64_t len) {
    for (uint64_t i = 0; i < len; ++i) {
        dest[i] = val;
    }
}

void* malloc(size_t size) {
    return (void*)malloc_words(size);
}

void free(void* ptr) {
    free_words((uintptr_t*)ptr);
}

int strcmp(const char *s1, const char *s2) {
    for (;;) {
        int a = *s1;
        int b = *s2;
        int diff = a - b;
        if (diff == 0) {
            if (a == 0) {
                return 0;
            } else {
                ++s1;
                ++s2;
            }
        } else {
            return diff;
        }
    }
}

size_t strlen(const char* s) {
    const char* t = s;
    while (*t) {
        ++t;
    }
    return t - s;
}

char *strdup(const char *s) {
    char* t = malloc(strlen(s) + 1);
    strcpy(t, s);
    return t;
}

char *strcpy(char *dest, const char *src) {
    char* orig_dest = dest;
    while (*src) {
        *dest = *src;
        ++dest;
        ++src;
    }
    return orig_dest;
}
