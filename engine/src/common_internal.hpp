#pragma once

// Yes, this is horrible, yes I'm ashamed, but this is the most foolproof way of
// not letting a user mess with a handle index/generation fields withou
// resorting to void* or std::any or variant and shit. A handle is and will
// always be just an u64, it's gonna be ok
#define MAKE_HANDLE_SETTERS(handle_type, handle_name)                                                                                      \
    inline void set_##handle_name##_index(handle_type* handle, u32 value) {                                                                \
        auto* p = reinterpret_cast<uint64_t*>(handle);                                                                                     \
        *p      = (*p & 0xFFFFFFFF00000000ull) | static_cast<uint64_t>(value);                                                             \
    }                                                                                                                                      \
    inline void set_##handle_name##_generation(handle_type* handle, u32 value) {                                                           \
        auto* p = reinterpret_cast<uint64_t*>(handle);                                                                                     \
        *p      = (static_cast<uint64_t>(value) << 32) | (*p & 0xFFFFFFFFull);                                                             \
    }
