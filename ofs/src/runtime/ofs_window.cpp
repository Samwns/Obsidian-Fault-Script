#include <stdint.h>
#include <vector>

#if defined(OFS_HAS_SDL2) && __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#define OFS_WINDOW_BACKEND_SDL 1
#elif defined(OFS_HAS_SDL2) && __has_include(<SDL.h>)
#include <SDL.h>
#define OFS_WINDOW_BACKEND_SDL 1
#else
#define OFS_WINDOW_BACKEND_SDL 0
#endif

#if OFS_WINDOW_BACKEND_SDL

static SDL_Window* g_window = nullptr;
static SDL_Renderer* g_renderer = nullptr;
static SDL_Texture* g_texture = nullptr;
static std::vector<uint32_t> g_present_buffer;
static int g_width = 0;
static int g_height = 0;
static int g_running = 0;

extern "C" {

void ofs_window_create(const char* title, int w, int h) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        g_running = 0;
        return;
    }

    g_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        w,
        h,
        SDL_WINDOW_SHOWN
    );

    if (!g_window) {
        SDL_Quit();
        g_running = 0;
        return;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_renderer) {
        SDL_DestroyWindow(g_window);
        g_window = nullptr;
        SDL_Quit();
        g_running = 0;
        return;
    }

    g_texture = SDL_CreateTexture(
        g_renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        w,
        h
    );

    if (!g_texture) {
        SDL_DestroyRenderer(g_renderer);
        SDL_DestroyWindow(g_window);
        g_renderer = nullptr;
        g_window = nullptr;
        SDL_Quit();
        g_running = 0;
        return;
    }

    g_width = w;
    g_height = h;
    g_present_buffer.assign(static_cast<size_t>(w) * static_cast<size_t>(h), 0u);
    g_running = 1;
}

void ofs_window_destroy() {
    if (g_texture) {
        SDL_DestroyTexture(g_texture);
        g_texture = nullptr;
    }
    if (g_renderer) {
        SDL_DestroyRenderer(g_renderer);
        g_renderer = nullptr;
    }
    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = nullptr;
    }
    SDL_Quit();
    g_present_buffer.clear();
    g_running = 0;
    g_width = 0;
    g_height = 0;
}

void ofs_window_present(void* pixel_buffer) {
    if (!g_renderer || !g_texture || !pixel_buffer) return;

    SDL_UpdateTexture(g_texture, nullptr, pixel_buffer, g_width * 4);
    SDL_RenderClear(g_renderer);
    SDL_RenderCopy(g_renderer, g_texture, nullptr, nullptr);
    SDL_RenderPresent(g_renderer);
}

void ofs_window_present_stone_rgba(const int64_t* pixel_buffer) {
    if (!g_renderer || !g_texture || !pixel_buffer) return;

    const size_t total = static_cast<size_t>(g_width) * static_cast<size_t>(g_height);
    if (g_present_buffer.size() != total) {
        g_present_buffer.assign(total, 0u);
    }

    for (size_t i = 0; i < total; ++i) {
        g_present_buffer[i] = static_cast<uint32_t>(pixel_buffer[i]);
    }

    SDL_UpdateTexture(g_texture, nullptr, g_present_buffer.data(), g_width * 4);
    SDL_RenderClear(g_renderer);
    SDL_RenderCopy(g_renderer, g_texture, nullptr, nullptr);
    SDL_RenderPresent(g_renderer);
}

int ofs_window_poll() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            g_running = 0;
        }
    }
    return g_running;
}

int ofs_window_is_open() {
    return g_running;
}

int ofs_window_width() {
    return g_width;
}

int ofs_window_height() {
    return g_height;
}

void ofs_window_set_title(const char* title) {
    if (g_window && title) {
        SDL_SetWindowTitle(g_window, title);
    }
}

int ofs_mouse_x() {
    int x = 0;
    int y = 0;
    SDL_GetMouseState(&x, &y);
    return x;
}

int ofs_mouse_y() {
    int x = 0;
    int y = 0;
    SDL_GetMouseState(&x, &y);
    return y;
}

int ofs_mouse_btn(int b) {
    return (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(b)) != 0;
}

int ofs_key_down(int scancode) {
    const uint8_t* keys = SDL_GetKeyboardState(nullptr);
    if (!keys || scancode < 0) return 0;
    return keys[scancode] ? 1 : 0;
}

}

#else

extern "C" {

void ofs_window_create(const char* title, int w, int h) { (void)title; (void)w; (void)h; }
void ofs_window_destroy() {}
void ofs_window_present(void* pixel_buffer) { (void)pixel_buffer; }
void ofs_window_present_stone_rgba(const int64_t* pixel_buffer) { (void)pixel_buffer; }
int ofs_window_poll() { return 0; }
int ofs_window_is_open() { return 0; }
int ofs_window_width() { return 0; }
int ofs_window_height() { return 0; }
void ofs_window_set_title(const char* title) { (void)title; }
int ofs_mouse_x() { return 0; }
int ofs_mouse_y() { return 0; }
int ofs_mouse_btn(int b) { (void)b; return 0; }
int ofs_key_down(int scancode) { (void)scancode; return 0; }

}

#endif
