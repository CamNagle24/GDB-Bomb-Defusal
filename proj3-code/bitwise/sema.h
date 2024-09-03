#ifndef SEMA_H
#define SEMA_H

#include <stdint.h>

int sema_wait(uint32_t *val);

int sema_post(uint32_t *val);

#endif // SEMA_H
