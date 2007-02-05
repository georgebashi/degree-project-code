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
// FIXME: 
#define TWOPI (2 * M_PI)

#define TRUE 1
#define FALSE 0

// argument options
#define OPT_PRINT_AUDIO 1
#define OPT_PRINT_WINDOW 2
#define OPT_PRINT_FFT 3

#endif
