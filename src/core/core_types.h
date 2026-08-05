#pragma once
#include <cstdint>
#include <cstddef>
#include <climits>
#include <type_traits>
#include <algorithm>
#include <string_view>
#include <initializer_list>

// NOTE: These are basically just for convenience and to avoid including <cstdint> everywhere
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;

using b8 = bool;
using b32 = int32_t;

using cstr = const char*;

constexpr i32 BitsPerByte = CHAR_BIT;

#ifdef _MSC_VER
#define __forceinline __forceinline
#else
#define __forceinline inline __attribute__((always_inline))
#endif

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

template <typename T>
constexpr T Min(T a, T b) { return (a < b) ? a : b; }
template <typename T>
constexpr T Max(T a, T b) { return (a > b) ? a : b; }
template <typename T>
constexpr T Clamp(T value, T min, T max) { return Min(Max(value, min), max); }

template <typename T>
constexpr void Swap(T& a, T& b)
{
	T temp = a;
	a = b;
	b = temp;
}

// NOTE: Basically a "safe" version of static_cast for enums, which will also work for bitflags
template <typename T>
constexpr b8 HasFlag(T value, T flag)
{
	static_assert(std::is_enum_v<T>);
	return (static_cast<std::underlying_type_t<T>>(value) & static_cast<std::underlying_type_t<T>>(flag)) != 0;
}

template <typename T>
constexpr void SetFlag(T& value, T flag)
{
	static_assert(std::is_enum_v<T>);
	value = static_cast<T>(static_cast<std::underlying_type_t<T>>(value) | static_cast<std::underlying_type_t<T>>(flag));
}

template <typename T>
constexpr void ClearFlag(T& value, T flag)
{
	static_assert(std::is_enum_v<T>);
	value = static_cast<T>(static_cast<std::underlying_type_t<T>>(value) & ~static_cast<std::underlying_type_t<T>>(flag));
}

template <typename T>
constexpr void ToggleFlag(T& value, T flag)
{
	static_assert(std::is_enum_v<T>);
	value = static_cast<T>(static_cast<std::underlying_type_t<T>>(value) ^ static_cast<std::underlying_type_t<T>>(flag));
}

// NOTE: Basically a "safe" version of static_cast for enums, which will also work for bitflags
template <typename T>
constexpr T Bit(T index)
{
	return static_cast<T>(1ull << static_cast<u64>(index));
}

// NOTE: Basically a "safe" version of static_cast for enums, which will also work for bitflags
template <typename T>
constexpr b8 IsPow2(T value)
{
	return value != 0 && (value & (value - 1)) == 0;
}

// NOTE: Basically a "safe" version of static_cast for enums, which will also work for bitflags
template <typename T>
constexpr T AlignUp(T value, T alignment)
{
	return (value + alignment - 1) & ~(alignment - 1);
}

// NOTE: Basically a "safe" version of static_cast for enums, which will also work for bitflags
template <typename T>
constexpr T AlignDown(T value, T alignment)
{
	return value & ~(alignment - 1);
}

// NOTE: Basically a "safe" version of static_cast for enums, which will also work for bitflags
template <typename T>
constexpr b8 IsAligned(T value, T alignment)
{
	return (value & (alignment - 1)) == 0;
}

// NOTE: Basically a "safe" version of static_cast for enums, which will also work for bitflags
template <typename T>
constexpr T Kilobytes(T value) { return value * static_cast<T>(1024); }
template <typename T>
constexpr T Megabytes(T value) { return Kilobytes(value) * static_cast<T>(1024); }
template <typename T>
constexpr T Gigabytes(T value) { return Megabytes(value) * static_cast<T>(1024); }
template <typename T>
constexpr T Terabytes(T value) { return Gigabytes(value) * static_cast<T>(1024); }

// NOTE: Basically a "safe" version of static_cast for enums, which will also work for bitflags
#define KiloBytes(Value) Kilobytes(Value)
#define MegaBytes(Value) Megabytes(Value)
#define GigaBytes(Value) Gigabytes(Value)
#define TeraBytes(Value) Terabytes(Value)

// NOTE: Basically a "safe" version of static_cast for enums, which will also work for bitflags
template <typename T>
constexpr T ToKilobytes(T value) { return value / static_cast<T>(1024); }
template <typename T>
constexpr T ToMegabytes(T value) { return ToKilobytes(value) / static_cast<T>(1024); }
template <typename T>
constexpr T ToGigabytes(T value) { return ToMegabytes(value) / static_cast<T>(1024); }
template <typename T>
constexpr T ToTerabytes(T value) { return ToGigabytes(value) / static_cast<T>(1024); }

// NOTE: Basically a "safe" version of static_cast for enums, which will also work for bitflags
template <typename T>
constexpr T ToPercent(T value, T max) { return (value * static_cast<T>(100)) / max; }
template <typename T>
constexpr T FromPercent(T percent, T max) { return (percent * max) / static_cast<T>(100); }

// NOTE: Basically a "safe" version of static_cast for enums, which will also work for bitflags
template <typename T>
constexpr T ToDegrees(T radians) { return radians * static_cast<T>(180.0 / 3.14159265358979323846); }
template <typename T>
constexpr T ToRadians(T degrees) { return degrees * static_cast<T>(3.14159265358979323846 / 180.0); }

// NOTE: Basically a "safe" version of static_cast for enums, which will also work for bitflags
template <typename T>
constexpr T Lerp(T a, T b, T t) { return a + (b - a) * t; }
template <typename T>
constexpr T Unlerp(T a, T b, T value) { return (value - a) / (b - a); }
template <typename T>
constexpr T Remap(T inMin, T inMax, T outMin, T outMax, T value) { return Lerp(outMin, outMax, Unlerp(inMin, inMax, value)); }

// NOTE: Example: if constexpr (expect_type_v<TTested, TExpected>); to be used where TTested is a template type declaring forwarding-reference parameters
template <typename TTested, typename... TExpected>
constexpr b8 expect_type_v = (std::is_same_v<TExpected, std::remove_cv_t<std::remove_reference_t<TTested>>> || ...);

template <typename TTested, typename... TExpected>
using expect_type_t = std::enable_if_t<expect_type_v<TTested, TExpected...>, bool>;

// Fallback for types that have COUNT but not Count, using a more robust pattern
template <typename T>
struct has_COUNT {
    template <typename U> static auto test(int) -> decltype(U::COUNT, std::true_type{});
    template <typename U> static auto test(...) -> std::false_type;
    static constexpr bool value = decltype(test<T>(0))::value;
};

template <typename T>
struct has_Count {
    template <typename U> static auto test(int) -> decltype(U::Count, std::true_type{});
    template <typename U> static auto test(...) -> std::false_type;
    static constexpr bool value = decltype(test<T>(0))::value;
};

template <typename T, typename = void>
struct EnumCountMemberHelper {};

template <typename EnumType, typename Enable = void>
struct EnumCountResolver;

template <typename EnumType>
struct EnumCountResolver<EnumType, std::enable_if_t<has_Count<EnumType>::value>> {
    static constexpr EnumType value = EnumType::Count;
};

template <typename EnumType>
struct EnumCountResolver<EnumType, std::enable_if_t<!has_Count<EnumType>::value && has_COUNT<EnumType>::value>> {
    static constexpr EnumType value = EnumType::COUNT;
};

template <typename EnumType>
struct EnumCountResolver<EnumType, std::enable_if_t<!has_Count<EnumType>::value && !has_COUNT<EnumType>::value>> {
    static constexpr EnumType value = EnumCountMemberHelper<EnumType>::value;
};

template <typename EnumType>
constexpr EnumType EnumCountMember = EnumCountResolver<EnumType>::value;

// NOTE: Assumes the enum class EnumType { ..., Count }; convention to be used everywhere
template <typename EnumType>
constexpr size_t EnumCount = static_cast<size_t>(EnumCountMember<EnumType>);

template <typename EnumType>
constexpr i32 EnumCountI32 = static_cast<i32>(EnumCountMember<EnumType>);

template <typename... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template <typename EnumType>
constexpr __forceinline size_t EnumToIndex(EnumType enumValue)
{
	static_assert(std::is_enum_v<EnumType>);
	return static_cast<size_t>(enumValue);
}

// NOTE: Basically "++enum", specifically to be used inside for loops. Return void to not add any confusion with the inOut& param
//		 Example: for (CoolEnum enum = {}; enum < CoolEnum::Count; IncrementEnum(enum)) { ... }
template <typename EnumType>
constexpr __forceinline void IncrementEnum(EnumType& inOutValue)
{
	static_assert(std::is_enum_v<EnumType>);
	inOutValue = static_cast<EnumType>(static_cast<size_t>(inOutValue) + 1);
}

template <typename EnumType, EnumType... Values>
constexpr __forceinline EnumType MinEnum(EnumType first, EnumType second)
{
	return static_cast<EnumType>(std::min(static_cast<size_t>(first), static_cast<size_t>(second)));
}

template <typename EnumType, EnumType... Values>
constexpr __forceinline EnumType MaxEnum(EnumType first, EnumType second)
{
	return static_cast<EnumType>(std::max(static_cast<size_t>(first), static_cast<size_t>(second)));
}

template <typename EnumType, typename T, EnumType... Values>
constexpr __forceinline EnumType ValueToEnum(T value)
{
	static_assert(std::is_enum_v<EnumType>);
	// NOTE: This is a bit of a hack, but it works for now
	// return static_cast<EnumType>(std::min({ (expect_type_v<T, EnumToType<Values>> ? static_cast<size_t>(Values) : static_cast<size_t>(EnumType::Count))... }));
	return static_cast<EnumType>(0);
}
