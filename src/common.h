#ifndef Z_COMMON_H
#define Z_COMMON_H

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float	 r32;
typedef double	 r64;

typedef u8  b8;
typedef u32 b16;
typedef u32 b32;
typedef u64 b64;
enum {false, true};

#define KILOBYTES(n) (1024LL*n)
#define MEGABYTES(n) (1024LL*KILOBYTES(n))
#define GIGABYTES(n) (1024LL*MEGABYTES(n))
#define TERABYTES(n) (1024LL*GIGABYTES(n))

#define assert(a) if(!a){*((u8*)0) = 5;}

#endif
