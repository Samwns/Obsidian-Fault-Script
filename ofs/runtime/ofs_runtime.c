#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "ws2_32.lib")
#elif defined(__linux__)
#include <arpa/inet.h>
#include <dlfcn.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

void echo_stone(int64_t v) { printf("%lld\n", (long long)v); }

void echo_crystal(double v) {
    if (v == (int64_t)v) {
        printf("%.1f\n", v);
    } else {
        printf("%g\n", v);
    }
}

void echo_obsidian(const char* s) { puts(s ? s : "(null)"); }
void echo_bool(int v) { puts(v ? "true" : "false"); }
void echo_newline(void) { putchar('\n'); }

void ofs_echo_color(const char* ansi_color, const char* text) {
    const char* color = ansi_color ? ansi_color : "\x1b[0m";
    printf("%s%s\x1b[0m\n", color, text ? text : "");
}

void* ofs_alloc(int64_t size) {
    if (size <= 0) size = 1;
    void* ptr = malloc((size_t)size);
    if (!ptr) {
        fputs("OFS: out of memory\n", stderr);
        exit(1);
    }
    return ptr;
}

void ofs_free(void* ptr) { free(ptr); }

typedef struct {
    void* data;
    int64_t len;
    int64_t cap;
    int64_t elem_size;
} OfsArray;

OfsArray* ofs_array_new(int64_t initial_cap, int64_t elem_size) {
    if (initial_cap <= 0) initial_cap = 4;
    if (elem_size <= 0) elem_size = 8;
    OfsArray* arr = (OfsArray*)ofs_alloc((int64_t)sizeof(OfsArray));
    arr->data = ofs_alloc(initial_cap * elem_size);
    arr->len = 0;
    arr->cap = initial_cap;
    arr->elem_size = elem_size;
    return arr;
}

void ofs_array_push(OfsArray* arr, void* element) {
    if (arr->len >= arr->cap) {
        arr->cap = arr->cap ? arr->cap * 2 : 4;
        arr->data = realloc(arr->data, (size_t)(arr->cap * arr->elem_size));
        if (!arr->data) {
            fputs("OFS: array grow failed\n", stderr);
            exit(1);
        }
    }
    memcpy((char*)arr->data + arr->len * arr->elem_size, element, (size_t)arr->elem_size);
    arr->len++;
}

void* ofs_array_get(OfsArray* arr, int64_t idx) {
    if (!arr || idx < 0 || idx >= arr->len) {
        fprintf(stderr, "OFS: index %lld out of bounds\n", (long long)idx);
        exit(1);
    }
    return (char*)arr->data + idx * arr->elem_size;
}

int64_t ofs_array_len(OfsArray* arr) { return arr ? arr->len : 0; }

void ofs_array_set(OfsArray* arr, int64_t idx, void* element) {
    if (!arr || idx < 0 || idx >= arr->len) {
        fprintf(stderr, "OFS: set index %lld out of bounds\n", (long long)idx);
        exit(1);
    }
    memcpy((char*)arr->data + idx * arr->elem_size, element, (size_t)arr->elem_size);
}

void* ofs_array_pop(OfsArray* arr) {
    if (!arr || arr->len <= 0) {
        fputs("OFS: pop from empty array\n", stderr);
        exit(1);
    }
    arr->len--;
    return (char*)arr->data + arr->len * arr->elem_size;
}

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

int ofs_str_contains(const char* haystack, const char* needle) {
    if (!haystack || !needle) return 0;
    return strstr(haystack, needle) != NULL;
}

int64_t ofs_str_len(const char* s) { return s ? (int64_t)strlen(s) : 0; }

int64_t ofs_str_char_at(const char* s, int64_t idx) {
    if (!s || idx < 0 || idx >= (int64_t)strlen(s)) return -1;
    return (int64_t)(unsigned char)s[idx];
}

int64_t ofs_str_char_at_known(const char* s, int64_t idx) {
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

char* ofs_str_upper(const char* s) {
    if (!s) return ofs_str_concat("", "");
    int64_t len = (int64_t)strlen(s);
    char* result = (char*)ofs_alloc(len + 1);
    for (int64_t i = 0; i < len; i++) result[i] = (char)toupper((unsigned char)s[i]);
    result[len] = '\0';
    return result;
}

char* ofs_str_lower(const char* s) {
    if (!s) return ofs_str_concat("", "");
    int64_t len = (int64_t)strlen(s);
    char* result = (char*)ofs_alloc(len + 1);
    for (int64_t i = 0; i < len; i++) result[i] = (char)tolower((unsigned char)s[i]);
    result[len] = '\0';
    return result;
}

char* ofs_stone_to_obsidian(int64_t v) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%lld", (long long)v);
    return ofs_str_concat(buf, "");
}

char* ofs_crystal_to_obsidian(double v) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%g", v);
    return ofs_str_concat(buf, "");
}

int64_t ofs_obsidian_to_stone(const char* s) { return s ? (int64_t)strtoll(s, NULL, 10) : 0; }
double ofs_obsidian_to_crystal(const char* s) { return s ? strtod(s, NULL) : 0.0; }

double ofs_pow(double base, double exp) { return pow(base, exp); }
double ofs_sqrt(double v) { return sqrt(v); }

int64_t ofs_mod(int64_t a, int64_t b) {
    if (b == 0) {
        fputs("OFS: modulo by zero\n", stderr);
        exit(1);
    }
    int64_t r = a % b;
    return r < 0 ? r + (b < 0 ? -b : b) : r;
}

char* ofs_read_line(void) {
    char buf[4096];
    if (!fgets(buf, sizeof(buf), stdin)) return ofs_str_concat("", "");
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
    return ofs_str_concat(buf, "");
}

void echo_stone_nn(int64_t v) { printf("%lld", (long long)v); }
void echo_crystal_nn(double v) { printf("%g", v); }
void echo_obsidian_nn(const char* s) { fputs(s ? s : "", stdout); }

#if defined(_WIN32)
typedef SOCKET ofs_socket_t;
#define OFS_INVALID_SOCKET INVALID_SOCKET
static void ofs_socket_close(ofs_socket_t s) { closesocket(s); }
#else
typedef int ofs_socket_t;
#define OFS_INVALID_SOCKET (-1)
static void ofs_socket_close(ofs_socket_t s) { close(s); }
#endif

static int ofs_socket_init(void) {
#if defined(_WIN32)
    static int started = 0;
    if (!started) {
        WSADATA data;
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return -1;
        started = 1;
    }
#endif
    return 0;
}

static int ofs_send_all(ofs_socket_t client, const char* data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
#if defined(_WIN32)
        int n = send(client, data + sent, (int)(len - sent), 0);
#else
        ssize_t n = send(client, data + sent, len - sent, 0);
#endif
        if (n <= 0) return -1;
        sent += (size_t)n;
    }
    return 0;
}

static char* ofs_build_http_response(const char* content_type, const char* body) {
    const char* type = content_type ? content_type : "text/plain; charset=utf-8";
    const char* payload = body ? body : "";
    size_t body_len = strlen(payload);
    char header[512];
    int header_len = snprintf(
        header,
        sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "Cache-Control: no-store\r\n"
        "\r\n",
        type,
        body_len
    );
    if (header_len < 0) return NULL;
    if ((size_t)header_len >= sizeof(header)) header_len = (int)sizeof(header) - 1;

    char* response = (char*)ofs_alloc((int64_t)header_len + (int64_t)body_len + 1);
    memcpy(response, header, (size_t)header_len);
    memcpy(response + header_len, payload, body_len);
    response[header_len + body_len] = '\0';
    return response;
}

static ofs_socket_t ofs_webserver_listen(int64_t port) {
    if (port <= 0 || port > 65535) port = 8080;
    if (ofs_socket_init() != 0) return OFS_INVALID_SOCKET;

    ofs_socket_t server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == OFS_INVALID_SOCKET) return OFS_INVALID_SOCKET;

    int yes = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)port);

    if (bind(server, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        ofs_socket_close(server);
        return OFS_INVALID_SOCKET;
    }
    if (listen(server, 16) != 0) {
        ofs_socket_close(server);
        return OFS_INVALID_SOCKET;
    }
    return server;
}

static int64_t ofs_webserver_accept_one(ofs_socket_t server, const char* response, size_t response_len) {
    char request_buf[2048];
    struct sockaddr_in client_addr;
#if defined(_WIN32)
    int client_len = sizeof(client_addr);
#else
    socklen_t client_len = sizeof(client_addr);
#endif
    ofs_socket_t client = accept(server, (struct sockaddr*)&client_addr, &client_len);
    if (client == OFS_INVALID_SOCKET) return -1;

    recv(client, request_buf, sizeof(request_buf) - 1, 0);
    int ok = ofs_send_all(client, response, response_len);
    ofs_socket_close(client);
    return ok == 0 ? 0 : -1;
}

int64_t ofs_webserver_serve_once(int64_t port, const char* content_type, const char* body) {
    char* response = ofs_build_http_response(content_type, body);
    if (!response) return 1;
    size_t response_len = strlen(response);
    ofs_socket_t server = ofs_webserver_listen(port);
    if (server == OFS_INVALID_SOCKET) {
        ofs_free(response);
        return 2;
    }

    int64_t result = ofs_webserver_accept_one(server, response, response_len);
    ofs_socket_close(server);
    ofs_free(response);
    return result == 0 ? 0 : 3;
}

int64_t ofs_webserver_serve_forever(int64_t port, const char* content_type, const char* body) {
    char* response = ofs_build_http_response(content_type, body);
    if (!response) return 1;
    size_t response_len = strlen(response);
    ofs_socket_t server = ofs_webserver_listen(port);
    if (server == OFS_INVALID_SOCKET) {
        ofs_free(response);
        return 2;
    }

    printf("OFS web server listening on http://127.0.0.1:%lld\n", (long long)(port > 0 ? port : 8080));
    fflush(stdout);
    for (;;) {
        ofs_webserver_accept_one(server, response, response_len);
    }

    ofs_socket_close(server);
    ofs_free(response);
    return 0;
}

typedef struct {
    int64_t width;
    int64_t height;
    int64_t open;
    int64_t headless;
    int64_t mouse_x;
    int64_t mouse_y;
    int64_t mouse_buttons[8];
} OfsWindowState;

static OfsWindowState ofs_window_state = {0};

static int ofs_window_clamp_size(int64_t v) {
    if (v < 1) return 1;
    if (v > 8192) return 8192;
    return (int)v;
}

#if defined(_WIN32)

static HWND ofs_win32_hwnd = NULL;
static HDC ofs_win32_mem_dc = NULL;
static HBITMAP ofs_win32_bitmap = NULL;
static void* ofs_win32_pixels = NULL;
static BITMAPINFO ofs_win32_bmi;

static void ofs_win32_create_buffer(int width, int height) {
    if (ofs_win32_bitmap) {
        DeleteObject(ofs_win32_bitmap);
        ofs_win32_bitmap = NULL;
        ofs_win32_pixels = NULL;
    }
    if (!ofs_win32_mem_dc) {
        HDC screen = GetDC(NULL);
        ofs_win32_mem_dc = CreateCompatibleDC(screen);
        ReleaseDC(NULL, screen);
    }
    memset(&ofs_win32_bmi, 0, sizeof(ofs_win32_bmi));
    ofs_win32_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    ofs_win32_bmi.bmiHeader.biWidth = width;
    ofs_win32_bmi.bmiHeader.biHeight = -height;
    ofs_win32_bmi.bmiHeader.biPlanes = 1;
    ofs_win32_bmi.bmiHeader.biBitCount = 32;
    ofs_win32_bmi.bmiHeader.biCompression = BI_RGB;
    ofs_win32_bitmap = CreateDIBSection(ofs_win32_mem_dc, &ofs_win32_bmi, DIB_RGB_COLORS, &ofs_win32_pixels, NULL, 0);
    if (ofs_win32_bitmap) {
        SelectObject(ofs_win32_mem_dc, ofs_win32_bitmap);
    }
}

static LRESULT CALLBACK ofs_win32_wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CLOSE:
        case WM_DESTROY:
            ofs_window_state.open = 0;
            PostQuitMessage(0);
            return 0;
        case WM_MOUSEMOVE:
            ofs_window_state.mouse_x = (int16_t)(lp & 0xffff);
            ofs_window_state.mouse_y = (int16_t)((lp >> 16) & 0xffff);
            return 0;
        case WM_LBUTTONDOWN: ofs_window_state.mouse_buttons[0] = 1; return 0;
        case WM_LBUTTONUP: ofs_window_state.mouse_buttons[0] = 0; return 0;
        case WM_RBUTTONDOWN: ofs_window_state.mouse_buttons[1] = 1; return 0;
        case WM_RBUTTONUP: ofs_window_state.mouse_buttons[1] = 0; return 0;
        case WM_MBUTTONDOWN: ofs_window_state.mouse_buttons[2] = 1; return 0;
        case WM_MBUTTONUP: ofs_window_state.mouse_buttons[2] = 0; return 0;
        default:
            return DefWindowProcA(hwnd, msg, wp, lp);
    }
}

void ofs_window_create(const char* title, int64_t w, int64_t h) {
    int width = ofs_window_clamp_size(w);
    int height = ofs_window_clamp_size(h);
    HINSTANCE inst = GetModuleHandleA(NULL);
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = ofs_win32_wndproc;
    wc.hInstance = inst;
    wc.lpszClassName = "OFSWindow";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc);

    DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, style, FALSE);
    ofs_win32_hwnd = CreateWindowExA(0, wc.lpszClassName, title ? title : "OFS",
                                     style, CW_USEDEFAULT, CW_USEDEFAULT,
                                     rect.right - rect.left, rect.bottom - rect.top,
                                     NULL, NULL, inst, NULL);
    if (!ofs_win32_hwnd) {
        ofs_window_state.headless = 1;
        ofs_window_state.open = 0;
        ofs_window_state.width = width;
        ofs_window_state.height = height;
        return;
    }
    ofs_window_state.headless = 0;
    ofs_window_state.open = 1;
    ofs_window_state.width = width;
    ofs_window_state.height = height;
    ofs_win32_create_buffer(width, height);
}

void ofs_window_destroy(void) {
    if (ofs_win32_bitmap) DeleteObject(ofs_win32_bitmap);
    if (ofs_win32_mem_dc) DeleteDC(ofs_win32_mem_dc);
    if (ofs_win32_hwnd) DestroyWindow(ofs_win32_hwnd);
    ofs_win32_bitmap = NULL;
    ofs_win32_mem_dc = NULL;
    ofs_win32_hwnd = NULL;
    ofs_win32_pixels = NULL;
    ofs_window_state.open = 0;
}

void ofs_window_present_stone_rgba(void* buf) {
    if (!ofs_win32_hwnd || !ofs_win32_pixels || !buf) return;
    int width = (int)ofs_window_state.width;
    int height = (int)ofs_window_state.height;
    int64_t* src = (int64_t*)buf;
    uint32_t* dst = (uint32_t*)ofs_win32_pixels;
    int64_t total = (int64_t)width * height;
    for (int64_t i = 0; i < total; i++) {
        uint32_t rgb = (uint32_t)(src[i] & 0x00ffffffu);
        dst[i] = rgb;
    }
    HDC dc = GetDC(ofs_win32_hwnd);
    StretchDIBits(dc, 0, 0, width, height, 0, 0, width, height, ofs_win32_pixels, &ofs_win32_bmi, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(ofs_win32_hwnd, dc);
}

int64_t ofs_window_poll(void) {
    if (!ofs_window_state.open || ofs_window_state.headless) return 0;
    MSG msg;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return ofs_window_state.open;
}

void ofs_window_set_title(const char* title) {
    if (ofs_win32_hwnd) SetWindowTextA(ofs_win32_hwnd, title ? title : "OFS");
}

int64_t ofs_key_down(int64_t scancode) {
    if (scancode < 0 || scancode > 255) return 0;
    return (GetAsyncKeyState((int)scancode) & 0x8000) != 0;
}

#elif defined(__linux__)

typedef struct _XDisplay Display;
typedef struct _XGC* GC;
typedef struct Visual Visual;
typedef struct _XImage XImage;
typedef unsigned long Window;
typedef unsigned long Atom;
typedef unsigned long Time;
typedef int Bool;

typedef struct {
    int type;
    unsigned long serial;
    Bool send_event;
    Display* display;
    Window window;
    char rest[192];
} OfsXEvent;

typedef struct {
    void* lib;
    Display* display;
    Window window;
    GC gc;
    Visual* visual;
    XImage* image;
    uint32_t* pixels;
    int screen;
    int depth;
    Atom wm_delete;
    int (*XPending)(Display*);
    int (*XNextEvent)(Display*, OfsXEvent*);
    Display* (*XOpenDisplay)(const char*);
    int (*XCloseDisplay)(Display*);
    int (*XDefaultScreen)(Display*);
    Window (*XRootWindow)(Display*, int);
    unsigned long (*XBlackPixel)(Display*, int);
    unsigned long (*XWhitePixel)(Display*, int);
    Window (*XCreateSimpleWindow)(Display*, Window, int, int, unsigned int, unsigned int, unsigned int, unsigned long, unsigned long);
    int (*XMapWindow)(Display*, Window);
    int (*XDestroyWindow)(Display*, Window);
    int (*XStoreName)(Display*, Window, const char*);
    int (*XSelectInput)(Display*, Window, long);
    GC (*XCreateGC)(Display*, Window, unsigned long, void*);
    int (*XFreeGC)(Display*, GC);
    int (*XFlush)(Display*);
    Visual* (*XDefaultVisual)(Display*, int);
    int (*XDefaultDepth)(Display*, int);
    XImage* (*XCreateImage)(Display*, Visual*, unsigned int, int, int, char*, unsigned int, unsigned int, int, int);
    int (*XPutImage)(Display*, Window, GC, XImage*, int, int, int, int, unsigned int, unsigned int);
    Atom (*XInternAtom)(Display*, const char*, Bool);
    int (*XSetWMProtocols)(Display*, Window, Atom*, int);
    Bool (*XQueryPointer)(Display*, Window, Window*, Window*, int*, int*, int*, int*, unsigned int*);
} OfsX11;

static OfsX11 ofs_x11 = {0};

static void* ofs_x11_sym(const char* name) {
    void* p = dlsym(ofs_x11.lib, name);
    if (!p) ofs_window_state.headless = 1;
    return p;
}

static int ofs_x11_load(void) {
    if (ofs_x11.lib) return !ofs_window_state.headless;
    ofs_x11.lib = dlopen("libX11.so.6", RTLD_LAZY | RTLD_LOCAL);
    if (!ofs_x11.lib) {
        ofs_window_state.headless = 1;
        return 0;
    }
#define OFS_X11_LOAD(name) ofs_x11.name = (void*)ofs_x11_sym(#name)
    OFS_X11_LOAD(XPending);
    OFS_X11_LOAD(XNextEvent);
    OFS_X11_LOAD(XOpenDisplay);
    OFS_X11_LOAD(XCloseDisplay);
    OFS_X11_LOAD(XDefaultScreen);
    OFS_X11_LOAD(XRootWindow);
    OFS_X11_LOAD(XBlackPixel);
    OFS_X11_LOAD(XWhitePixel);
    OFS_X11_LOAD(XCreateSimpleWindow);
    OFS_X11_LOAD(XMapWindow);
    OFS_X11_LOAD(XDestroyWindow);
    OFS_X11_LOAD(XStoreName);
    OFS_X11_LOAD(XSelectInput);
    OFS_X11_LOAD(XCreateGC);
    OFS_X11_LOAD(XFreeGC);
    OFS_X11_LOAD(XFlush);
    OFS_X11_LOAD(XDefaultVisual);
    OFS_X11_LOAD(XDefaultDepth);
    OFS_X11_LOAD(XCreateImage);
    OFS_X11_LOAD(XPutImage);
    OFS_X11_LOAD(XInternAtom);
    OFS_X11_LOAD(XSetWMProtocols);
    OFS_X11_LOAD(XQueryPointer);
#undef OFS_X11_LOAD
    return !ofs_window_state.headless;
}

void ofs_window_create(const char* title, int64_t w, int64_t h) {
    int width = ofs_window_clamp_size(w);
    int height = ofs_window_clamp_size(h);
    ofs_window_state.width = width;
    ofs_window_state.height = height;
    ofs_window_state.open = 0;
    ofs_window_state.headless = 0;
    if (!ofs_x11_load()) return;

    ofs_x11.display = ofs_x11.XOpenDisplay(NULL);
    if (!ofs_x11.display) {
        ofs_window_state.headless = 1;
        return;
    }
    ofs_x11.screen = ofs_x11.XDefaultScreen(ofs_x11.display);
    Window root = ofs_x11.XRootWindow(ofs_x11.display, ofs_x11.screen);
    ofs_x11.window = ofs_x11.XCreateSimpleWindow(
        ofs_x11.display, root, 0, 0, (unsigned int)width, (unsigned int)height, 1,
        ofs_x11.XBlackPixel(ofs_x11.display, ofs_x11.screen),
        ofs_x11.XWhitePixel(ofs_x11.display, ofs_x11.screen));
    long mask = (1L << 0) | (1L << 1) | (1L << 2) | (1L << 3) | (1L << 6) | (1L << 15) | (1L << 17);
    ofs_x11.XSelectInput(ofs_x11.display, ofs_x11.window, mask);
    ofs_x11.XStoreName(ofs_x11.display, ofs_x11.window, title ? title : "OFS");
    ofs_x11.wm_delete = ofs_x11.XInternAtom(ofs_x11.display, "WM_DELETE_WINDOW", 0);
    ofs_x11.XSetWMProtocols(ofs_x11.display, ofs_x11.window, &ofs_x11.wm_delete, 1);
    ofs_x11.gc = ofs_x11.XCreateGC(ofs_x11.display, ofs_x11.window, 0, NULL);
    ofs_x11.visual = ofs_x11.XDefaultVisual(ofs_x11.display, ofs_x11.screen);
    ofs_x11.depth = ofs_x11.XDefaultDepth(ofs_x11.display, ofs_x11.screen);
    ofs_x11.pixels = (uint32_t*)calloc((size_t)width * (size_t)height, sizeof(uint32_t));
    if (ofs_x11.pixels) {
        ofs_x11.image = ofs_x11.XCreateImage(ofs_x11.display, ofs_x11.visual, (unsigned int)ofs_x11.depth, 2, 0,
                                             (char*)ofs_x11.pixels, (unsigned int)width, (unsigned int)height, 32, 0);
    }
    ofs_x11.XMapWindow(ofs_x11.display, ofs_x11.window);
    ofs_x11.XFlush(ofs_x11.display);
    ofs_window_state.open = 1;
}

void ofs_window_destroy(void) {
    if (ofs_x11.display) {
        if (ofs_x11.gc) ofs_x11.XFreeGC(ofs_x11.display, ofs_x11.gc);
        if (ofs_x11.window) ofs_x11.XDestroyWindow(ofs_x11.display, ofs_x11.window);
        ofs_x11.XCloseDisplay(ofs_x11.display);
    }
    free(ofs_x11.pixels);
    ofs_x11.display = NULL;
    ofs_x11.window = 0;
    ofs_x11.gc = NULL;
    ofs_x11.image = NULL;
    ofs_x11.pixels = NULL;
    ofs_window_state.open = 0;
}

void ofs_window_present_stone_rgba(void* buf) {
    if (!ofs_x11.display || !ofs_x11.image || !ofs_x11.pixels || !buf) return;
    int64_t total = ofs_window_state.width * ofs_window_state.height;
    int64_t* src = (int64_t*)buf;
    for (int64_t i = 0; i < total; i++) {
        ofs_x11.pixels[i] = (uint32_t)(src[i] & 0x00ffffffu);
    }
    ofs_x11.XPutImage(ofs_x11.display, ofs_x11.window, ofs_x11.gc, ofs_x11.image, 0, 0, 0, 0,
                      (unsigned int)ofs_window_state.width, (unsigned int)ofs_window_state.height);
    ofs_x11.XFlush(ofs_x11.display);
}

int64_t ofs_window_poll(void) {
    if (!ofs_window_state.open || ofs_window_state.headless || !ofs_x11.display) return 0;
    OfsXEvent ev;
    while (ofs_x11.XPending(ofs_x11.display) > 0) {
        memset(&ev, 0, sizeof(ev));
        ofs_x11.XNextEvent(ofs_x11.display, &ev);
        if (ev.type == 17 || ev.type == 33) {
            ofs_window_state.open = 0;
        }
    }
    Window root;
    Window child;
    int root_x;
    int root_y;
    int win_x;
    int win_y;
    unsigned int mask;
    if (ofs_x11.XQueryPointer(ofs_x11.display, ofs_x11.window, &root, &child, &root_x, &root_y, &win_x, &win_y, &mask)) {
        ofs_window_state.mouse_x = win_x;
        ofs_window_state.mouse_y = win_y;
        ofs_window_state.mouse_buttons[0] = (mask & (1u << 8)) != 0;
        ofs_window_state.mouse_buttons[1] = (mask & (1u << 10)) != 0;
        ofs_window_state.mouse_buttons[2] = (mask & (1u << 9)) != 0;
    }
    return ofs_window_state.open;
}

void ofs_window_set_title(const char* title) {
    if (ofs_x11.display && ofs_x11.window) {
        ofs_x11.XStoreName(ofs_x11.display, ofs_x11.window, title ? title : "OFS");
        ofs_x11.XFlush(ofs_x11.display);
    }
}

int64_t ofs_key_down(int64_t scancode) {
    (void)scancode;
    return 0;
}

#else

void ofs_window_create(const char* title, int64_t w, int64_t h) {
    (void)title;
    ofs_window_state.width = ofs_window_clamp_size(w);
    ofs_window_state.height = ofs_window_clamp_size(h);
    ofs_window_state.open = 0;
    ofs_window_state.headless = 1;
}

void ofs_window_destroy(void) { ofs_window_state.open = 0; }
void ofs_window_present_stone_rgba(void* buf) { (void)buf; }
int64_t ofs_window_poll(void) { return 0; }
void ofs_window_set_title(const char* title) { (void)title; }
int64_t ofs_key_down(int64_t scancode) { (void)scancode; return 0; }

#endif

void ofs_window_present(void* buf) { ofs_window_present_stone_rgba(buf); }
void ofs_window_present_pixels(void* buf) { ofs_window_present_stone_rgba(buf); }
int64_t ofs_window_is_open(void) { return ofs_window_state.open; }
int64_t ofs_window_width(void) { return ofs_window_state.width; }
int64_t ofs_window_height(void) { return ofs_window_state.height; }
int64_t ofs_mouse_x(void) { return ofs_window_state.mouse_x; }
int64_t ofs_mouse_y(void) { return ofs_window_state.mouse_y; }
int64_t ofs_mouse_btn(int64_t btn) {
    if (btn < 0 || btn >= 8) return 0;
    return ofs_window_state.mouse_buttons[btn];
}
