#pragma once
#include <string>
#include <cstdlib>

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

// ── Locale detection ──────────────────────────────────────────────────────
// Returns true when the system language is Portuguese (pt / pt_BR / pt_PT).
// Checked once at first call and cached.

inline bool ofs_locale_is_pt() {
    static int cached = -1;
    if (cached != -1) return cached == 1;

    // Env vars take priority on all platforms (easy override / CI)
    for (const char* var : {"LANGUAGE", "LC_ALL", "LC_MESSAGES", "LANG"}) {
        const char* val = std::getenv(var);
        if (val && val[0] != '\0') {
            std::string s(val);
            cached = (s.rfind("pt", 0) == 0) ? 1 : 0;
            return cached == 1;
        }
    }

#if defined(_WIN32)
    // Windows fallback: query the Windows UI language
    LANGID lid = GetUserDefaultUILanguage();
    // LANG_PORTUGUESE == 0x16
    cached = (PRIMARYLANGID(lid) == 0x16) ? 1 : 0;
#else
    cached = 0;
#endif
    return cached == 1;
}

// ── Message selection macro ───────────────────────────────────────────────
// Usage: OFS_MSG("English text", "Texto em português")
// Returns std::string with the text matching the system language.

#define OFS_MSG(en, pt) (ofs_locale_is_pt() ? std::string(pt) : std::string(en))
