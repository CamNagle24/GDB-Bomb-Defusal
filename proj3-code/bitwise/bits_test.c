/* Testing Code */

#include <limits.h>
#include <math.h>

/* Routines used by floating point test code */

/* Convert from bit level representation to floating point number */
float u2f(unsigned u) {
    union {
        unsigned u;
        float f;
    } a;
    a.u = u;
    return a.f;
}

/* Convert from floating point number to bit-level representation */
unsigned f2u(float f) {
    union {
        unsigned u;
        float f;
    } a;
    a.f = f;
    return a.u;
}

int test_bitMatch(int x, int y) {
    int result = 0;
    for (int i = 0; i < 32; i++) {
        int mask = 1 << i;
        int bit = (x & mask) == (y & mask);
        result |= bit << i;
    }
    return result;
}

int test_evenBits(void) {
    int result = 0;
    for (int i = 0; i < 32; i+=2) {
        result |= 1 << i;
    }
    return result;
}

int test_allOddBits(int x) {
    for (int i = 1; i < 32; i += 2) {
        if ((x & (1<<i)) == 0) {
            return 0;
        }
    }
    return 1;
}

unsigned test_floatAbsVal(unsigned uf) {
    float f = u2f(uf);
    unsigned unf = f2u(-f);
    if (isnan(f)) {
        return uf;
    }
    /* An unfortunate hack to get around a limitation of the BDD Checker */
    if ((int) uf < 0) {
        return unf;
    }
    return uf;
}

int test_implication(int x, int y) {
    return !(x & (!y));
}

int test_isNegative(int x) {
    return x < 0;
}

int test_sign(int x) {
    if (!x) {
        return 0;
    }
    return (x < 0) ? -1 : 1;
}

int test_isGreater(int x, int y) {
    return x > y;
}

int test_logicalShift(int x, int n) {
    unsigned u = (unsigned) x;
    unsigned shifted = u >> n;
    return (int) shifted;
}

int test_rotateRight(int x, int n) {
    unsigned u = (unsigned) x;
    for (int i = 0; i < n; i++) {
        unsigned lsb = (u & 1) << 31;
        unsigned rest = u >> 1;
        u = lsb | rest;
    }
    return (int) u;
}

unsigned test_floatScale4(unsigned uf) {
    float f = u2f(uf);
    float tf = 4*f;
    if (isnan(f)) {
        return uf;
    } else {
        return f2u(tf);
    }
}

int test_greatestBitPos(int x) {
    unsigned mask = 1<<31;
    if (x == 0) {
        return 0;
    }
    while (!(mask & x)) {
        mask = mask >> 1;
    }
    return mask;
}
