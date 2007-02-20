#ifndef COMMON_H
#define COMMON_H

#ifdef __GCC__
#define LIKELY(x)   (__builtin_expect(x, 1))
#define UNLIKELY(x) (__builtin_expect(x, 0))
#else /* __GCC__ */
#define LIKELY(x)   ((x) == 1)
#define UNLIKELY(x) ((x) == 0)
#endif /* __GCC__ */

// constants
#define WINDOW_SIZE 1024
#define STEP_SIZE 512
#define BINS 512
#define WINDOWS_PER_BLOCK 150

// FIXME:
#define TWOPI (2 * M_PI)

#endif
