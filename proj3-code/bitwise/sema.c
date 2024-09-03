#include <errno.h>
#include <linux/futex.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "sema.h"

// Note that this implementation is a binary semaphore
// Internal value is always either 0 or 1

int sema_wait(uint32_t *val) {
    while (__atomic_exchange_n(val, 0, __ATOMIC_SEQ_CST) == 0) {
        if (syscall(SYS_futex, val, FUTEX_WAIT, 0, NULL) == -1) {
            if (errno != EAGAIN) {
                return -1;
            } else {
                // Value is no longer 0, so caller should bring it back to 0
                // and proceed
                __atomic_store_n(val, 0, __ATOMIC_SEQ_CST);
                return 0;
            }
        }
    }
    return 0;
}

int sema_post(uint32_t *val) {
    __atomic_store_n(val, 1, __ATOMIC_SEQ_CST);
    if (syscall(SYS_futex, val, FUTEX_WAKE, 1) == -1) {
        return -1;
    }
    return 0;
}
