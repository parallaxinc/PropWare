#pragma once

#include <stdlib.h>
#include <new>

std::new_handler __new_handler;

void *
operator new (std::size_t sz) {
    void *p;

/* malloc (0) is unpredictable; avoid it.  */
    if (sz == 0)
        sz = 1;
    p      = (void *) malloc(sz);
    while (p == 0) {
        std::new_handler handler = __new_handler;
        if (!handler)
            ::abort();
        handler();
        // FIXME: Replace std namespace when GCCv5+ C++ headers are installed
        p                        = (void *) ::malloc(sz);
    }

    return p;
}

void *
operator new[] (std::size_t sz) {
    return ::operator new(sz);
}

void
operator delete (void *ptr) {
    if (ptr)
        // FIXME: Replace std namespace when GCCv5+ C++ headers are installed
        ::free(ptr);
}

#if __GNUC__ >= 5
void
operator delete (void *ptr, std::size_t size) {
    operator delete(ptr);
}
#endif

std::new_handler std::set_new_handler (std::new_handler handler) {
    std::new_handler prev_handler = __new_handler;
    __new_handler = handler;
    return prev_handler;
}
