#pragma once
#include <array>
#include <cstdint>
#include <type_traits>
#include <utility>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

union DeviceLUID {
    struct
    {
        u32 low;
        i32 high;
    };

    u8 vk_uuid[8];
};

#define NullDeviceLUID                                                                                                                     \
    DeviceLUID { }

#if defined(_MSC_VER)
#define CORE_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define CORE_FORCE_INLINE inline __attribute__((always_inline))
#else
#define CORE_FORCE_INLINE inline
#endif

#define CORE_MAKE_INTERFACE(name)                                                                                                          \
protected:                                                                                                                                 \
    name() = default;                                                                                                                      \
                                                                                                                                           \
public:                                                                                                                                    \
    virtual ~name() {};                                                                                                                    \
                                                                                                                                           \
private:

#define CORE_MAKE_INTERFACE_PROTECTED(name)                                                                                                \
protected:                                                                                                                                 \
    name() = default;                                                                                                                      \
    virtual ~name() {};                                                                                                                    \
                                                                                                                                           \
private:

#define CORE_MAKE_HANDLE(name)                                                                                                             \
    typedef struct name                                                                                                                    \
    {                                                                                                                                      \
    private:                                                                                                                               \
        u64 compound_index = 0;                                                                                                            \
                                                                                                                                           \
    public:                                                                                                                                \
        u32 get_index() const { return static_cast<u32>(compound_index & 0xFFFFFFFFull); }                                                 \
        u32 get_generation() const { return static_cast<u32>(compound_index >> 32); }                                                      \
    } name;

#define CORE_MAKE_SCOPED_ENUM_BITOPS(T)                                                                                                    \
    inline constexpr T operator|(T lhs, T rhs) {                                                                                           \
        return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) | static_cast<std::underlying_type_t<T>>(rhs));                  \
    }                                                                                                                                      \
    inline constexpr T operator&(T lhs, T rhs) {                                                                                           \
        return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) & static_cast<std::underlying_type_t<T>>(rhs));                  \
    }                                                                                                                                      \
    inline constexpr T operator^(T lhs, T rhs) {                                                                                           \
        return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) ^ static_cast<std::underlying_type_t<T>>(rhs));                  \
    }                                                                                                                                      \
    inline constexpr T operator~(T rhs) { return static_cast<T>(~static_cast<std::underlying_type_t<T>>(rhs)); }                           \
    inline T& operator|=(T& lhs, T rhs) {                                                                                                  \
        return lhs = static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) | static_cast<std::underlying_type_t<T>>(rhs));            \
    }                                                                                                                                      \
    inline T& operator&=(T& lhs, T rhs) {                                                                                                  \
        return lhs = static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) & static_cast<std::underlying_type_t<T>>(rhs));            \
    }                                                                                                                                      \
    inline T& operator^=(T& lhs, T rhs) {                                                                                                  \
        return lhs = static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) ^ static_cast<std::underlying_type_t<T>>(rhs));            \
    }

#define CORE_NESTED_SCOPED_ENUM_BITOPS(T)                                                                                                  \
    friend inline constexpr T operator|(T lhs, T rhs) {                                                                                    \
        return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) | static_cast<std::underlying_type_t<T>>(rhs));                  \
    }                                                                                                                                      \
    friend inline constexpr T operator&(T lhs, T rhs) {                                                                                    \
        return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) & static_cast<std::underlying_type_t<T>>(rhs));                  \
    }                                                                                                                                      \
    friend inline constexpr T operator^(T lhs, T rhs) {                                                                                    \
        return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) ^ static_cast<std::underlying_type_t<T>>(rhs));                  \
    }                                                                                                                                      \
    friend inline constexpr T operator~(T rhs) { return static_cast<T>(~static_cast<std::underlying_type_t<T>>(rhs)); }                    \
    friend inline T& operator|=(T& lhs, T rhs) {                                                                                           \
        return lhs = static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) | static_cast<std::underlying_type_t<T>>(rhs));            \
    }                                                                                                                                      \
    friend inline T& operator&=(T& lhs, T rhs) {                                                                                           \
        return lhs = static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) & static_cast<std::underlying_type_t<T>>(rhs));            \
    }                                                                                                                                      \
    friend inline T& operator^=(T& lhs, T rhs) {                                                                                           \
        return lhs = static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) ^ static_cast<std::underlying_type_t<T>>(rhs));            \
    }
