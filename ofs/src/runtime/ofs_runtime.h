#ifndef OFS_RUNTIME_H
#define OFS_RUNTIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Echo functions (called by echo() in OFS)
void echo_stone(int64_t v);
void echo_crystal(double v);
void echo_obsidian(const char* s);
void echo_bool(int v);
void echo_newline(void);

// Memory allocation
void* ofs_alloc(int64_t size);
void  ofs_free(void* ptr);

// Dynamic array
typedef struct {
    void*   data;
    int64_t len;
    int64_t cap;
    int64_t elem_size;
} OfsArray;

OfsArray* ofs_array_new(int64_t initial_cap, int64_t elem_size);
void      ofs_array_push(OfsArray* arr, void* element);
void*     ofs_array_get(OfsArray* arr, int64_t idx);
int64_t   ofs_array_len(OfsArray* arr);

// String operations
char* ofs_str_concat(const char* a, const char* b);
int   ofs_str_eq(const char* a, const char* b);
int64_t ofs_str_len(const char* s);
int64_t ofs_str_char_at(const char* s, int64_t idx);
char*   ofs_str_substr(const char* s, int64_t start, int64_t len);
int     ofs_str_contains(const char* haystack, const char* needle);

// Type conversions
char*   ofs_stone_to_obsidian(int64_t v);
char*   ofs_crystal_to_obsidian(double v);
int64_t ofs_obsidian_to_stone(const char* s);
double  ofs_obsidian_to_crystal(const char* s);

// Array operations (extended)
void  ofs_array_set(OfsArray* arr, int64_t idx, void* element);
void* ofs_array_pop(OfsArray* arr);

// Math operations
double  ofs_pow(double base, double exp);
double  ofs_sqrt(double v);
int64_t ofs_mod(int64_t a, int64_t b);

// I/O operations
char* ofs_read_line(void);
void  echo_stone_nn(int64_t v);
void  echo_crystal_nn(double v);
void  echo_obsidian_nn(const char* s);

#ifdef __cplusplus
}
#endif

#endif // OFS_RUNTIME_H
