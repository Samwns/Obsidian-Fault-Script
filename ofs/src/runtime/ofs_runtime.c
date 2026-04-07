#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>

// ── Echo functions (called by echo() in OFS) ─────────────────────────────

void echo_stone(int64_t v) {
    printf("%lld\n", (long long)v);
}

void echo_crystal(double v) {
    // Print without trailing zeros when possible
    if (v == (int64_t)v) printf("%.1f\n", v);
    else                  printf("%g\n", v);
}

void echo_obsidian(const char* s) {
    puts(s ? s : "(null)");
}

void echo_bool(int v) {
    puts(v ? "true" : "false");
}

void ofs_echo_color(const char* ansi_color, const char* text) {
    const char* raw_color = ansi_color ? ansi_color : "\x1b[0m";
    const size_t raw_len = strlen(raw_color);
    char* normalized = (char*)malloc(raw_len + 1);
    int allocated = 1;
    if (!normalized) {
        normalized = (char*)"\x1b[0m";
        allocated = 0;
    } else {
        size_t i = 0;
        size_t w = 0;
        while (i < raw_len) {
            if (i + 7 <= raw_len && strncmp(raw_color + i, "\\u001b[", 7) == 0) {
                normalized[w++] = '\x1b';
                normalized[w++] = '[';
                i += 7;
                continue;
            }
            if (i + 5 <= raw_len && strncmp(raw_color + i, "\\x1b[", 5) == 0) {
                normalized[w++] = '\x1b';
                normalized[w++] = '[';
                i += 5;
                continue;
            }
            normalized[w++] = raw_color[i++];
        }
        normalized[w] = '\0';
    }

    const char* value = text ? text : "";
    printf("%s%s\x1b[0m\n", normalized, value);

    if (allocated) {
        free(normalized);
    }
}

void echo_newline(void) {
    putchar('\n');
}

// ── Memory allocation ─────────────────────────────────────────────────────

void* ofs_alloc(int64_t size) {
    if (size <= 0) size = 1;
    void* ptr = malloc((size_t)size);
    if (!ptr) { fputs("OFS: out of memory\n", stderr); exit(1); }
    return ptr;
}

void ofs_free(void* ptr) {
    free(ptr);
}

// ── Dynamic array ─────────────────────────────────────────────────────────
// Layout: { data: void*, len: i64, cap: i64, elem_size: i64 }

typedef struct {
    void*   data;
    int64_t len;
    int64_t cap;
    int64_t elem_size;
} OfsArray;

OfsArray* ofs_array_new(int64_t initial_cap, int64_t elem_size) {
    if (initial_cap <= 0) initial_cap = 4;
    if (elem_size <= 0) elem_size = 8;
    OfsArray* arr   = (OfsArray*)ofs_alloc(sizeof(OfsArray));
    arr->data       = ofs_alloc(initial_cap * elem_size);
    arr->len        = 0;
    arr->cap        = initial_cap;
    arr->elem_size  = elem_size;
    return arr;
}

void ofs_array_push(OfsArray* arr, void* element) {
    if (arr->len >= arr->cap) {
        arr->cap  = arr->cap ? arr->cap * 2 : 4;
        arr->data = realloc(arr->data, (size_t)(arr->cap * arr->elem_size));
        if (!arr->data) { fputs("OFS: array grow failed\n", stderr); exit(1); }
    }
    memcpy((char*)arr->data + arr->len * arr->elem_size, element, (size_t)arr->elem_size);
    arr->len++;
}

void* ofs_array_get(OfsArray* arr, int64_t idx) {
    if (idx < 0 || idx >= arr->len) {
        fprintf(stderr, "OFS: index %lld out of bounds (len=%lld)\n",
                (long long)idx, (long long)arr->len);
        exit(1);
    }
    return (char*)arr->data + idx * arr->elem_size;
}

int64_t ofs_array_len(OfsArray* arr) {
    return arr->len;
}

// ── String operations ─────────────────────────────────────────────────────

char* ofs_str_concat(const char* a, const char* b) {
    size_t la = a ? strlen(a) : 0;
    size_t lb = b ? strlen(b) : 0;
    char* result = (char*)ofs_alloc((int64_t)(la + lb + 1));
    if (a) memcpy(result, a, la);
    if (b) memcpy(result + la, b, lb);
    result[la + lb] = '\0';
    return result;
}

int ofs_str_eq(const char* a, const char* b) {
    if (!a && !b) return 1;
    if (!a || !b) return 0;
    return strcmp(a, b) == 0;
}

int64_t ofs_str_len(const char* s) {
    return s ? (int64_t)strlen(s) : 0;
}

int64_t ofs_str_char_at(const char* s, int64_t idx) {
    if (!s || idx < 0 || idx >= (int64_t)strlen(s)) return -1;
    return (int64_t)(unsigned char)s[idx];
}

char* ofs_str_substr(const char* s, int64_t start, int64_t len) {
    if (!s) return ofs_str_concat("", "");
    int64_t slen = (int64_t)strlen(s);
    if (start < 0) start = 0;
    if (start >= slen) return ofs_str_concat("", "");
    if (len < 0 || start + len > slen) len = slen - start;
    char* result = (char*)ofs_alloc(len + 1);
    memcpy(result, s + start, (size_t)len);
    result[len] = '\0';
    return result;
}

int ofs_str_contains(const char* haystack, const char* needle) {
    if (!haystack || !needle) return 0;
    return strstr(haystack, needle) != NULL;
}

char* ofs_str_upper(const char* s) {
    if (!s) return ofs_str_concat("", "");
    int64_t len = (int64_t)strlen(s);
    char* result = (char*)ofs_alloc(len + 1);
    for (int64_t i = 0; i < len; i++) {
        result[i] = (char)toupper((unsigned char)s[i]);
    }
    result[len] = '\0';
    return result;
}

char* ofs_str_lower(const char* s) {
    if (!s) return ofs_str_concat("", "");
    int64_t len = (int64_t)strlen(s);
    char* result = (char*)ofs_alloc(len + 1);
    for (int64_t i = 0; i < len; i++) {
        result[i] = (char)tolower((unsigned char)s[i]);
    }
    result[len] = '\0';
    return result;
}

// ── Type conversions ──────────────────────────────────────────────────────

char* ofs_stone_to_obsidian(int64_t v) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%lld", (long long)v);
    size_t len = strlen(buf);
    char* result = (char*)ofs_alloc((int64_t)(len + 1));
    memcpy(result, buf, len + 1);
    return result;
}

char* ofs_crystal_to_obsidian(double v) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%g", v);
    size_t len = strlen(buf);
    char* result = (char*)ofs_alloc((int64_t)(len + 1));
    memcpy(result, buf, len + 1);
    return result;
}

int64_t ofs_obsidian_to_stone(const char* s) {
    if (!s) return 0;
    return (int64_t)strtoll(s, NULL, 10);
}

double ofs_obsidian_to_crystal(const char* s) {
    if (!s) return 0.0;
    return strtod(s, NULL);
}

// ── Array operations (extended) ───────────────────────────────────────────

void ofs_array_set(OfsArray* arr, int64_t idx, void* element) {
    if (idx < 0 || idx >= arr->len) {
        fprintf(stderr, "OFS: set index %lld out of bounds (len=%lld)\n",
                (long long)idx, (long long)arr->len);
        exit(1);
    }
    memcpy((char*)arr->data + idx * arr->elem_size, element, (size_t)arr->elem_size);
}

void* ofs_array_pop(OfsArray* arr) {
    if (arr->len <= 0) {
        fputs("OFS: pop from empty array\n", stderr);
        exit(1);
    }
    arr->len--;
    return (char*)arr->data + arr->len * arr->elem_size;
}

// ── Math operations ───────────────────────────────────────────────────────

double ofs_pow(double base, double exp) {
    return pow(base, exp);
}

double ofs_sqrt(double v) {
    return sqrt(v);
}

int64_t ofs_mod(int64_t a, int64_t b) {
    if (b == 0) { fputs("OFS: modulo by zero\n", stderr); exit(1); }
    int64_t r = a % b;
    return r < 0 ? r + (b < 0 ? -b : b) : r;
}

// ── I/O operations ────────────────────────────────────────────────────────

char* ofs_read_line(void) {
    char buf[4096];
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        char* empty = (char*)ofs_alloc(1);
        empty[0] = '\0';
        return empty;
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[--len] = '\0';
    char* result = (char*)ofs_alloc((int64_t)(len + 1));
    memcpy(result, buf, len + 1);
    return result;
}

void echo_stone_nn(int64_t v) {
    printf("%lld", (long long)v);
}

void echo_crystal_nn(double v) {
    if (v == (int64_t)v) printf("%.1f", v);
    else printf("%g", v);
}

void echo_obsidian_nn(const char* s) {
    fputs(s ? s : "(null)", stdout);
}

// ── HTTP webserver helpers ────────────────────────────────────────────────
// Minimal POSIX-socket-based HTTP server (Linux / macOS).
// Windows uses Winsock; for now they return -1 on that platform.

#if defined(_WIN32)
#  include <winsock2.h>
#  pragma comment(lib, "ws2_32.lib")
#  define OFS_CLOSE_SOCKET(s) closesocket(s)
typedef SOCKET ofs_socket_t;
static int ofs_ws_init(void) {
    WSADATA wd;
    return WSAStartup(MAKEWORD(2,2), &wd);
}
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <unistd.h>
#  define OFS_CLOSE_SOCKET(s) close(s)
typedef int ofs_socket_t;
static int ofs_ws_init(void) { return 0; }
#endif

static ofs_socket_t ofs_make_server(int64_t port) {
    if (ofs_ws_init() != 0) return (ofs_socket_t)-1;

    ofs_socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if ((int)fd < 0) return (ofs_socket_t)-1;

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((unsigned short)port);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        OFS_CLOSE_SOCKET(fd);
        return (ofs_socket_t)-1;
    }
    listen(fd, 10);
    return fd;
}

static void ofs_serve_one(ofs_socket_t server_fd,
                          const char* content_type, const char* body) {
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
#if defined(_WIN32)
    int client_len = sizeof(client_addr);
#else
    socklen_t client_len = sizeof(client_addr);
#endif
    ofs_socket_t client_fd = accept(server_fd,
                                     (struct sockaddr*)&client_addr,
                                     &client_len);
    if ((int)client_fd < 0) return;

    /* Discard the request */
    char req_buf[4096];
    recv(client_fd, req_buf, sizeof(req_buf) - 1, 0);

    const char* ct = content_type ? content_type : "text/plain";
    const char* bd = body       ? body        : "";
    size_t body_len = strlen(bd);

    char header[640];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
        ct, body_len);

    send(client_fd, header, (int)strlen(header), 0);
    send(client_fd, bd,     (int)body_len,       0);
    OFS_CLOSE_SOCKET(client_fd);
}

int64_t ofs_webserver_serve_once(int64_t port,
                                  const char* content_type,
                                  const char* body) {
    ofs_socket_t fd = ofs_make_server(port);
    if ((int)fd < 0) return -1;
    ofs_serve_one(fd, content_type, body);
    OFS_CLOSE_SOCKET(fd);
    return 200;
}

int64_t ofs_webserver_serve_forever(int64_t port,
                                     const char* content_type,
                                     const char* body) {
    ofs_socket_t fd = ofs_make_server(port);
    if ((int)fd < 0) return -1;
    for (;;) {
        ofs_serve_one(fd, content_type, body);
    }
    /* unreachable */
    OFS_CLOSE_SOCKET(fd);
    return 0;
}
