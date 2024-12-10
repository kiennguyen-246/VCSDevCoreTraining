#pragma once
#ifdef _WIN32
#define strcpy(x, ...) strcpy_s(x, sizeof(x), __VA_ARGS__)
#define memcpy(x, ...) memcpy_s(x, sizeof(x), __VA_ARGS__)
#endif
