#define ENABLE_PLATFORM_MINGW32 1

typedef signed char smallint;
typedef unsigned char smalluint;
# include <stdbool.h>

# define ALWAYS_INLINE inline
# define NOINLINE
# define DEPRECATED
# define UNUSED_PARAM_RESULT

# define ALIGN1 __attribute__((aligned(1)))
# define ALIGN2 __attribute__((aligned(2)))
# define ALIGN4 __attribute__((aligned(4)))

#ifndef __const
# define __const const
#endif

#define UNUSED_PARAM __attribute__ ((__unused__))
#define NORETURN __attribute__ ((__noreturn__))

#  define FAST_FUNC
# define FIX_ALIASING

# define PUSH_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN
# define POP_SAVED_FUNCTION_VISIBILITY

#if defined(i386) || defined(__x86_64__) || defined(__powerpc__)
# define BB_UNALIGNED_MEMACCESS_OK 1
# define move_from_unaligned_int(v, intp)  ((v) = *(bb__aliased_int*)(intp))
# define move_from_unaligned_long(v, longp) ((v) = *(bb__aliased_long*)(longp))
# define move_from_unaligned16(v, u16p) ((v) = *(bb__aliased_uint16_t*)(u16p))
# define move_from_unaligned32(v, u32p) ((v) = *(bb__aliased_uint32_t*)(u32p))
# define move_to_unaligned16(u16p, v)   (*(bb__aliased_uint16_t*)(u16p) = (v))
# define move_to_unaligned32(u32p, v)   (*(bb__aliased_uint32_t*)(u32p) = (v))
# define move_to_unaligned64(u64p, v)   (*(bb__aliased_uint64_t*)(u64p) = (v))
/* #elif ... - add your favorite arch today! */
#else
# define BB_UNALIGNED_MEMACCESS_OK 0
/* performs reasonably well (gcc usually inlines memcpy here) */
# define move_from_unaligned_int(v, intp) (memcpy(&(v), (intp), sizeof(int)))
# define move_from_unaligned_long(v, longp) (memcpy(&(v), (longp), sizeof(long)))
# define move_from_unaligned16(v, u16p) (memcpy(&(v), (u16p), 2))
# define move_from_unaligned32(v, u32p) (memcpy(&(v), (u32p), 4))
# define move_to_unaligned16(u16p, v) do { \
	uint16_t __t = (v); \
	memcpy((u16p), &__t, 2); \
} while (0)
# define move_to_unaligned32(u32p, v) do { \
	uint32_t __t = (v); \
	memcpy((u32p), &__t, 4); \
} while (0)
# define move_to_unaligned64(u64p, v) do { \
	uint64_t __t = (v); \
	memcpy((u64p), &__t, 8); \
} while (0)
#endif

/* ---- Endian Detection ------------------------------------ */
#include <limits.h>
#if defined(__digital__) && defined(__unix__)
# include <sex.h>
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
   || defined(__APPLE__)
# include <sys/resource.h>  /* rlimit */
# include <machine/endian.h>
# define bswap_64 __bswap64
# define bswap_32 __bswap32
# define bswap_16 __bswap16
#elif ENABLE_PLATFORM_MINGW32
# define __BIG_ENDIAN 0
# define __LITTLE_ENDIAN 1
# define __BYTE_ORDER __LITTLE_ENDIAN
# define bswap_16(x) ((((x) & 0xFF00) >> 8) | (((x) & 0xFF) << 8))
# define bswap_32(x) ((bswap_16(((x) & 0xFFFF0000L) >> 16)) | (bswap_16((x) & 0xFFFFL) << 16))
# define bswap_64(x) ((bswap_32(((x) & 0xFFFFFFFF00000000LL) >> 32)) | (bswap_32((x) & 0xFFFFFFFFLL) << 32))
#else
# include <byteswap.h>
# include <endian.h>
#endif

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN
# define BB_BIG_ENDIAN 1
# define BB_LITTLE_ENDIAN 0
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN
# define BB_BIG_ENDIAN 0
# define BB_LITTLE_ENDIAN 1
#elif defined(_BYTE_ORDER) && _BYTE_ORDER == _BIG_ENDIAN
# define BB_BIG_ENDIAN 1
# define BB_LITTLE_ENDIAN 0
#elif defined(_BYTE_ORDER) && _BYTE_ORDER == _LITTLE_ENDIAN
# define BB_BIG_ENDIAN 0
# define BB_LITTLE_ENDIAN 1
#elif defined(BYTE_ORDER) && BYTE_ORDER == BIG_ENDIAN
# define BB_BIG_ENDIAN 1
# define BB_LITTLE_ENDIAN 0
#elif defined(BYTE_ORDER) && BYTE_ORDER == LITTLE_ENDIAN
# define BB_BIG_ENDIAN 0
# define BB_LITTLE_ENDIAN 1
#elif defined(__386__)
# define BB_BIG_ENDIAN 0
# define BB_LITTLE_ENDIAN 1
#else
# error "Can't determine endianness"
#endif

#if ULONG_MAX > 0xffffffff
# define bb_bswap_64(x) bswap_64(x)
#endif

/* SWAP_LEnn means "convert CPU<->little_endian by swapping bytes" */
#if BB_BIG_ENDIAN
# define SWAP_BE16(x) (x)
# define SWAP_BE32(x) (x)
# define SWAP_BE64(x) (x)
# define SWAP_LE16(x) bswap_16(x)
# define SWAP_LE32(x) bswap_32(x)
# define SWAP_LE64(x) bb_bswap_64(x)
# define IF_BIG_ENDIAN(...) __VA_ARGS__
# define IF_LITTLE_ENDIAN(...)
#else
# define SWAP_BE16(x) bswap_16(x)
# define SWAP_BE32(x) bswap_32(x)
# define SWAP_BE64(x) bb_bswap_64(x)
# define SWAP_LE16(x) (x)
# define SWAP_LE32(x) (x)
# define SWAP_LE64(x) (x)
# define IF_BIG_ENDIAN(...)
# define IF_LITTLE_ENDIAN(...) __VA_ARGS__
#endif

#define RETURNS_MALLOC __attribute__ ((malloc))
#define PACKED __attribute__ ((__packed__))
#define ALIGNED(m) __attribute__ ((__aligned__(m)))

typedef int      bb__aliased_int      FIX_ALIASING;
typedef long     bb__aliased_long     FIX_ALIASING;
typedef uint16_t bb__aliased_uint16_t FIX_ALIASING;
typedef uint32_t bb__aliased_uint32_t FIX_ALIASING;
typedef uint64_t bb__aliased_uint64_t FIX_ALIASING;


   /* Convenience macros to test the version of gcc. */
#undef __GNUC_PREREQ
#if defined __GNUC__ && defined __GNUC_MINOR__
# define __GNUC_PREREQ(maj, min) \
		((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
# define __GNUC_PREREQ(maj, min) 0
#endif

#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))

 /* required by mingw */
#include <stddef.h>
extern void *memrchr(const void *s, int c, size_t n) FAST_FUNC;