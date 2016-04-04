#if defined(__APPLE__)

#include <libkern/OSByteOrder.h>

#define htob16(x) OSSwapHostToBigInt16(x)
#define htob32(x) OSSwapHostToBigInt32(x)
#define htob64(x) OSSwapHostToBigInt64(x)
#define b16toh(x) OSSwapBigToHostInt16(x)
#define b32toh(x) OSSwapBigToHostInt32(x)
#define b64toh(x) OSSwapBigToHostInt64(x)

#else

#include <endian.h>
#include <byteswap.h>

#ifndef htob16
#define htob16(x) bswap_16(x)
#endif

#ifndef htob32
#define htob32(x) bswap_32(x)
#endif

#ifndef htob64
#define htob64(x) bswap_64(x)
#endif

#ifndef b16toh
#define b16toh(x) bswap_16(x)
#endif

#ifndef b32toh
#define b32toh(x) bswap_32(x)
#endif

#ifndef b64toh
#define b64toh(x) bswap_64(x)
#endif

#endif
