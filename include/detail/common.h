#pragma once
#include "config.h"
#include <type_traits>

#if defined(__clang__)
#define INLINE [[clang::always_inline]] inline
#elif defined(__GNUC__) || defined(__GNUG__)
#define INLINE [[gnu::always_inline]] inline
#elif defined(_MSC_VER)
#define INLINE __forceinline
#else
#define INLINE inline
#endif

#include <cstddef>
#include <cstdint>

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
namespace detail
{
    struct _castable_to_any_pointer
    {
        void* ptr;

        constexpr _castable_to_any_pointer(void* ptr) : ptr(ptr) {}

        template <typename T>
        operator T*()
        {
            return reinterpret_cast<T*>(ptr);
        }
    };
} // namespace detail

template <typename T>
inline auto as_ptr(std::uintptr_t ptr) -> T*
{
    return reinterpret_cast<T*>(ptr);
}

inline auto as_ptr(std::uintptr_t ptr) -> detail::_castable_to_any_pointer { return {reinterpret_cast<void*>(ptr)}; }

inline auto as_pv(std::uintptr_t ptr) -> void* { return as_ptr<void>(ptr); }

template <typename T>
inline auto as_uptr(T* ptr) -> uintptr_t
{
    return reinterpret_cast<std::uintptr_t>(ptr);
}

template <typename T>
inline auto as_ptr(void* ptr) -> T*
{
    return reinterpret_cast<T*>(ptr);
}
inline auto as_ptr(void* ptr) -> detail::_castable_to_any_pointer { return {ptr}; }

template <typename T>
inline auto as_vptr(T* ptr) -> void*
{
    return reinterpret_cast<void*>(ptr);
}

template <typename T>
inline auto as_vptr(const T* ptr) -> const void*
{
    return reinterpret_cast<const void*>(ptr);
}

inline auto as_vptr(std::uintptr_t ptr) -> void* { return reinterpret_cast<void*>(ptr); }

template <typename T, typename U>
inline auto cast_ptr(U* ptr) -> T*
{
    return reinterpret_cast<T*>(ptr);
}

template <typename T>
inline auto cast_ptr(T* ptr) -> detail::_castable_to_any_pointer
{
    return detail::_castable_to_any_pointer(as_vptr(ptr));
}

template <typename T, std::size_t N>
inline auto decay_arr(T (&arr)[N]) -> T*
{
    return (T*)arr;
}

template <typename T>
inline auto decay_arr(T (&arr)[]) -> T*
{
    return (T*)arr;
}

template <typename T>
inline auto as_signed(T val) -> typename std::make_signed<T>::type
{
    return typename std::make_signed<T>::type(val);
}

template <typename T>
inline auto as_unsigned(T val) -> typename std::make_unsigned<T>::type
{
    return typename std::make_unsigned<T>::type(val);
}

// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
