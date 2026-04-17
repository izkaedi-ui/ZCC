#ifndef FAKE_CSMITH_H
#define FAKE_CSMITH_H
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long int64_t;
typedef unsigned long uint64_t;
typedef long intptr_t;
typedef unsigned long uintptr_t;
typedef unsigned long size_t;
#define INT8_MAX 127
#define INT8_MIN (-128)
#define INT16_MAX 32767
#define INT16_MIN (-32768)
#define INT32_MAX 2147483647
#define INT32_MIN (-2147483647-1)
#define INT64_MAX 9223372036854775807L
#define INT64_MIN (-9223372036854775807L-1)
#define UINT8_MAX 255
#define UINT16_MAX 65535
#define UINT32_MAX 4294967295U
#define UINT64_MAX 18446744073709551615UL
#define NULL ((void*)0)
static int printf(const char *fmt, ...);
static int strcmp(const char *s1, const char *s2);
#define platform_main_begin()
#define platform_main_end(crc, flag)
#define crc32_gentab()
#define transparent_crc(val, varname, flag)
#endif
