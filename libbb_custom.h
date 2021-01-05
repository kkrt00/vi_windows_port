//libs
#include "vi_config.h"

#include <ctype.h>
#include <sys/types.h>
#include <windows.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <ws2tcpip.h>
#include <termios.h>

#include "platform.h"
#include <mingw.h>
#include <xatonum.h>

extern const char bb_skip_ansi_emulation[] ALIGN1;

/* Some useful definitions */
#undef FALSE
#define FALSE   ((int) 0)
#undef TRUE
#define TRUE    ((int) 1)
#undef SKIP
#define SKIP	((int) 2)

/* Macros for min/max.  */
#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

enum {
	KEYCODE_UP        =  -2,
	KEYCODE_DOWN      =  -3,
	KEYCODE_RIGHT     =  -4,
	KEYCODE_LEFT      =  -5,
	KEYCODE_HOME      =  -6,
	KEYCODE_END       =  -7,
	KEYCODE_INSERT    =  -8,
	KEYCODE_DELETE    =  -9,
	KEYCODE_PAGEUP    = -10,
	KEYCODE_PAGEDOWN  = -11,
	KEYCODE_BACKSPACE = -12, /* Used only if Alt/Ctrl/Shifted */
	KEYCODE_D         = -13, /* Used only if Alted */

	/* ^^^^^ Be sure that last defined value is small enough.
	 * Current read_key() code allows going up to -32 (0xfff..fffe0).
	 * This gives three upper bits in LSB to play with:
	 * KEYCODE_foo values are 0xfff..fffXX, lowest XX bits are: scavvvvv,
	 * s=0 if SHIFT, c=0 if CTRL, a=0 if ALT,
	 * vvvvv bits are the same for same key regardless of "shift bits".
	 */
	//KEYCODE_SHIFT_...   = KEYCODE_...   & ~0x80,
	KEYCODE_CTRL_RIGHT    = KEYCODE_RIGHT & ~0x40,
	KEYCODE_CTRL_LEFT     = KEYCODE_LEFT  & ~0x40,
	KEYCODE_ALT_RIGHT     = KEYCODE_RIGHT & ~0x20,
	KEYCODE_ALT_LEFT      = KEYCODE_LEFT  & ~0x20,
	KEYCODE_ALT_BACKSPACE = KEYCODE_BACKSPACE & ~0x20,
	KEYCODE_ALT_D         = KEYCODE_D     & ~0x20,

	KEYCODE_CURSOR_POS = -0x100, /* 0xfff..fff00 */
	/* How long is the longest ESC sequence we know?
	 * We want it big enough to be able to contain
	 * cursor position sequence "ESC [ 9999 ; 9999 R"
	 */
	KEYCODE_BUFFER_SIZE = 16
};
int64_t read_key(int fd, char *buffer, int timeout) FAST_FUNC;
void read_key_ungets(char *buffer, const char *str, unsigned len) FAST_FUNC;

struct globals;
/* '*const' ptr makes gcc optimize code much better.
 * Magic prevents ptr_to_globals from going into rodata.
 * If you want to assign a value, use SET_PTR_TO_GLOBALS(x) */
extern struct globals *const ptr_to_globals;

#if defined(__clang_major__) && __clang_major__ >= 9
/* Clang/llvm drops assignment to "constant" storage. Silently.
 * Needs serious convincing to not eliminate the store.
 */
static ALWAYS_INLINE void* not_const_pp(const void *p)
{
	void *pp;
	__asm__ __volatile__(
		"# forget that p points to const"
		: /*outputs*/ "=r" (pp)
		: /*inputs*/ "0" (p)
	);
	return pp;
}
#else
static ALWAYS_INLINE void* not_const_pp(const void *p) { return (void*)p; }
#endif

/* At least gcc 3.4.6 on mipsel system needs optimization barrier */
#define barrier() __asm__ __volatile__("":::"memory")
#define SET_PTR_TO_GLOBALS(x) do { \
	(*(struct globals**)not_const_pp(&ptr_to_globals)) = (void*)(x); \
	barrier(); \
} while (0)

#define FREE_PTR_TO_GLOBALS() do { \
	if (ENABLE_FEATURE_CLEAN_UP) { \
		free(ptr_to_globals); \
	} \
} while (0)


# define STRERROR_FMT    "%s"
# define STRERROR_ERRNO  ,strerror(errno)

#if defined(errno)
/* If errno is a define, assume it's "define errno (*__errno_location())"
 * and we will cache it's result in this variable */
extern int *const bb_errno;
#undef errno
#define errno (*bb_errno)
#define bb_cached_errno_ptr 1
#endif



struct suffix_mult {
	char suffix[4];
	unsigned mult;
};
extern const struct suffix_mult bkm_suffixes[];
#define km_suffixes (bkm_suffixes + 1)
extern const struct suffix_mult cwbkMG_suffixes[];
#define kMG_suffixes (cwbkMG_suffixes + 3)
extern const struct suffix_mult kmg_i_suffixes[];

typedef struct md5_ctx_t {
	uint8_t wbuffer[64]; /* always correctly aligned for uint64_t */
	void (*process_block)(struct md5_ctx_t*) FAST_FUNC;
	uint64_t total64;    /* must be directly before hash[] */
	uint32_t hash[8];    /* 4 elements for md5, 5 for sha1, 8 for sha256 */
} md5_ctx_t;
typedef struct md5_ctx_t sha1_ctx_t;
typedef struct md5_ctx_t sha256_ctx_t;
typedef struct sha512_ctx_t {
	uint64_t total64[2];  /* must be directly before hash[] */
	uint64_t hash[8];
	uint8_t wbuffer[128]; /* always correctly aligned for uint64_t */
} sha512_ctx_t;
typedef struct sha3_ctx_t {
	uint64_t state[25];
	unsigned bytes_queued;
	unsigned input_block_bytes;
} sha3_ctx_t;
void md5_begin(md5_ctx_t *ctx) FAST_FUNC;
void md5_hash(md5_ctx_t *ctx, const void *buffer, size_t len) FAST_FUNC;
unsigned md5_end(md5_ctx_t *ctx, void *resbuf) FAST_FUNC;
void sha1_begin(sha1_ctx_t *ctx) FAST_FUNC;
#define sha1_hash md5_hash
unsigned sha1_end(sha1_ctx_t *ctx, void *resbuf) FAST_FUNC;
void sha256_begin(sha256_ctx_t *ctx) FAST_FUNC;
#define sha256_hash md5_hash
#define sha256_end  sha1_end
void sha512_begin(sha512_ctx_t *ctx) FAST_FUNC;
void sha512_hash(sha512_ctx_t *ctx, const void *buffer, size_t len) FAST_FUNC;
unsigned sha512_end(sha512_ctx_t *ctx, void *resbuf) FAST_FUNC;
void sha3_begin(sha3_ctx_t *ctx) FAST_FUNC;
void sha3_hash(sha3_ctx_t *ctx, const void *buffer, size_t len) FAST_FUNC;
unsigned sha3_end(sha3_ctx_t *ctx, void *resbuf) FAST_FUNC;
/* TLS benefits from knowing that sha1 and sha256 share these. Give them "agnostic" names too */
typedef struct md5_ctx_t md5sha_ctx_t;
#define md5sha_hash md5_hash
#define sha_end sha1_end
enum {
	MD5_OUTSIZE    = 16,
	SHA1_OUTSIZE   = 20,
	SHA256_OUTSIZE = 32,
	SHA512_OUTSIZE = 64,
	SHA3_OUTSIZE   = 28,
};

#if ENABLE_PLATFORM_MINGW32 && \
	(!defined(__USE_MINGW_ANSI_STDIO) || !__USE_MINGW_ANSI_STDIO)
#define LL_FMT "I64"
#else
#define LL_FMT "ll"
#endif

void *malloc_or_warn(size_t size) FAST_FUNC RETURNS_MALLOC;
void *xmalloc(size_t size) FAST_FUNC RETURNS_MALLOC;
void *xzalloc(size_t size) FAST_FUNC RETURNS_MALLOC;
void *xrealloc(void *old, size_t size) FAST_FUNC;
/* After v = xrealloc_vector(v, SHIFT, idx) it's ok to use
 * at least v[idx] and v[idx+1], for all idx values.
 * SHIFT specifies how many new elements are added (1:2, 2:4, ..., 8:256...)
 * when all elements are used up. New elements are zeroed out.
 * xrealloc_vector(v, SHIFT, idx) *MUST* be called with consecutive IDXs -
 * skipping an index is a bad bug - it may miss a realloc!
 */
#define xrealloc_vector(vector, shift, idx) \
	xrealloc_vector_helper((vector), (sizeof((vector)[0]) << 8) + (shift), (idx))
void* xrealloc_vector_helper(void *vector, unsigned sizeof_and_shift, int idx) FAST_FUNC;
char *xstrdup(const char *s) FAST_FUNC RETURNS_MALLOC;
char *xstrndup(const char *s, int n) FAST_FUNC RETURNS_MALLOC;
void *xmemdup(const void *s, int n) FAST_FUNC RETURNS_MALLOC;

#define ARRAY_SIZE(x) ((unsigned)(sizeof(x) / sizeof((x)[0])))
char *last_char_is(const char *s, int c) FAST_FUNC;

/* Function stubs */

void bb_die_memory_exhausted(void) NORETURN FAST_FUNC;
#define bb_simple_error_msg_and_die bb_error_msg_and_die
