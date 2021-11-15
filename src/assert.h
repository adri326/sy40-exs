#include <stdio.h>
#include <errno.h>
#include <assert.h>

#ifndef ASSERT_H
#define ASSERT_H

#define passert_op(type, print, left, right, op, op_neg) { int line = __LINE__; \
    errno = 0; \
    type l = left; \
    type r = right; \
    if (l op_neg r) { \
        if (errno != 0) { \
            char buffer[1024]; \
            sprintf(buffer, __FILE__ ":%d errno is non-zero", line); \
            perror(buffer); \
        } \
        fprintf(stderr, __FILE__ ":%d Assertion failed: " print " " #op_neg " " print "\n", line, l, r); \
        fprintf(stderr, __FILE__ ":%d Expected " #left " " #op " " #right "\n", line); \
        exit(1); \
    } \
}

#define passert_eq(type, print, left, right) passert_op(type, print, left, right, ==, !=)

#define passert_neq(type, print, left, right) passert_op(type, print, left, right, !=, ==)

#define passert_gt(type, print, left, right) passert_op(type, print, left, right, >, <=)

#define passert_lt(type, print, left, right) passert_op(type, print, left, right, <, >=)

#define passert_gte(type, print, left, right) passert_op(type, print, left, right, >=, <)

#define passert_lte(type, print, left, right) passert_op(type, print, left, right, <=, >)

#endif // ASSERT_H
