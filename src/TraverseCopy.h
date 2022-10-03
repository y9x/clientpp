#pragma once
#include <rapidjson/fwd.h>

rapidjson::Value TraverseCopy(rapidjson::Value &value, rapidjson::Value &match,
                              rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator, bool allow_new_props = false,
                              bool *changed = nullptr);