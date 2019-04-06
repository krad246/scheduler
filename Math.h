/*
 * Math.h
 *
 *  Created on: Apr 5, 2019
 *      Author: krad2
 */

#ifndef MATH_H_
#define MATH_H_

#include <cstdint>
#include <tuple>
#include <type_traits>

template <class T> struct NextSize;
template <class T> struct tag { using type = T; };

template <> struct NextSize<std::uint8_t>  : tag<std::uint16_t> { };
template <> struct NextSize<std::uint16_t> : tag<std::uint32_t> { };
template <> struct NextSize<std::uint32_t> : tag<std::uint64_t> { };

template <> struct NextSize<std::int8_t>  : tag<std::int16_t> { };
template <> struct NextSize<std::int16_t> : tag<std::int32_t> { };
template <> struct NextSize<std::int32_t> : tag<std::int64_t> { };

template <std::size_t bits, bool sign> struct MinSizeType;
template <> struct MinSizeType<8, false> : tag<std::uint8_t> { };
template <> struct MinSizeType<16, false> : tag<std::uint16_t> { };
template <> struct MinSizeType<32, false> : tag<std::uint32_t> { };
template <> struct MinSizeType<64, false> : tag<std::uint64_t> { };

template <> struct MinSizeType<8, true> : tag<std::int8_t> { };
template <> struct MinSizeType<16, true> : tag<std::int16_t> { };
template <> struct MinSizeType<32, true> : tag<std::int32_t> { };
template <> struct MinSizeType<64, true> : tag<std::int64_t> { };

template <class T1, class T2>
inline auto multiply(T1 x, T2 y) {
	constexpr bool bigger = sizeof(T1) > sizeof(T2);
	using BiggerType = typename std::conditional<bigger, T1, T2>::type;
	using SignedType = typename std::conditional<std::is_signed<T1>::value, T1, T2>::type;

	using ProductRetType = typename std::conditional <
		std::is_signed<T1>::value != std::is_signed<T2>::value,
		typename std::conditional <
			!std::is_same<SignedType, BiggerType>::value,
			typename std::make_signed <
				typename NextSize<BiggerType>::type
			>::type,
			typename NextSize<BiggerType>::type
		>::type,
		typename NextSize<BiggerType>::type
	>::type;

	register ProductRetType ans = 0;
	register ProductRetType smallerNum = x > y ? y : x;
	const register ProductRetType largerNum = x > y ? x : y;

	std::uint8_t count = 0;
	while (smallerNum) {
		if (smallerNum & 1) {
			ans += largerNum << count;
		}

		count++;
		smallerNum >>= 1;
	}

	return ans;
}

template <class T1, class T2>
auto modulus(T1 x, T2 y);

template <std::size_t bits>
inline typename MinSizeType<bits, false>::type rand(void);

template <> inline std::uint16_t rand<16>(void) {
	static std::uint16_t state = 1;
    state ^= state << 7;
    state ^= state >> 9;
    state ^= state << 8;
    return state;
}

template <> inline std::uint32_t rand<32>(void) {
	static std::uint32_t state = 2463534242;
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

template <> inline std::uint64_t rand<64>(void) {
	static std::uint64_t state = 8817264546332525;
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}

#endif /* MATH_H_ */
