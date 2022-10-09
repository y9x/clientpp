/*
 * Location utilities
 */
#pragma once
#include "../utils/Uri.h"
#include <string>

namespace krunker {
extern const wchar_t *game;
extern const wchar_t *games;
extern const wchar_t *editor;
extern const wchar_t *social;
extern const wchar_t *viewer;
extern const wchar_t *scripting;
extern const wchar_t *docs;
extern const wchar_t *tos;

bool host_is_krunker(const std::wstring &host);
}; // namespace krunker
