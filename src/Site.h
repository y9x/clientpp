/*
 * Location utilities
 */
#pragma once
#include "../utils/Uri.h"
#include <string>

namespace krunker {
enum type {
  Game,
  Social,
  Editor,
  Viewer,
  Documents,
  Scripting,
  Invalid,
};

extern const wchar_t *game;
extern const wchar_t *games;
extern const wchar_t *editor;
extern const wchar_t *social;
extern const wchar_t *viewer;
extern const wchar_t *scripting;
extern const wchar_t *tos;

extern type identifyType(UriW uri);
}; // namespace krunker
