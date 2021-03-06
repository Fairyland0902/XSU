#ifndef _ASSERT_H
#define _ASSERT_H

#undef assert
#define assert(expr, msg) \
    ((expr) ? (void)0 : badassert(#expr, msg))

void badassert(const char* expr, const char* msg);

#endif
