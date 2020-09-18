#include <stdlib.h>

void* operator new(std::size_t sz) {
    return malloc(sz);
}

void operator delete(void* ptr) noexcept {
    free(ptr);
}
