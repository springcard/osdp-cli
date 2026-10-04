#include "random.h"
#include "../util/utils.h"
#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#include <limits.h>
#else
#include <errno.h>
#include <sys/random.h>
#endif

bool osdp_random_bytes(uint8_t *buffer, size_t size)
{
    if (!buffer && size) return false;
#ifdef _WIN32
    if (size <= ULONG_MAX && BCryptGenRandom(NULL, buffer, (ULONG)size,
                                            BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0)
        return true;
#else
    size_t offset = 0;
    while (offset < size) {
        ssize_t count = getrandom(buffer + offset, size - offset, 0);
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) break;
        offset += (size_t)count;
    }
    if (offset == size) return true;
#endif
    secure_zero(buffer, size);
    return false;
}
