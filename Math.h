/*
 * Math.h
 *
 *  Created on: Apr 5, 2019
 *      Author: krad2
 */

#ifndef MATH_H_
#define MATH_H_

#include <cstdint>
#include <type_traits>

/**
 * Helper templates to get types from sizes or typenames.
 */

template <class T> struct tag {
	using type = T;
};

/**
 * Finds the integral type that is twice as large as a given type.
 */

template <class T> struct NextSize;

template <> struct NextSize<std::uint8_t>  : tag<std::uint16_t> { };
template <> struct NextSize<std::uint16_t> : tag<std::uint32_t> { };
template <> struct NextSize<std::uint32_t> : tag<std::uint64_t> { };

template <> struct NextSize<std::int8_t>  : tag<std::int16_t> { };
template <> struct NextSize<std::int16_t> : tag<std::int32_t> { };
template <> struct NextSize<std::int32_t> : tag<std::int64_t> { };

/**
 * Finds an integral type with the specified width in bits.
 */

template <std::size_t bits, bool sign> struct MinSizeType;
template <> struct MinSizeType<8, false> : tag<std::uint8_t> { };
template <> struct MinSizeType<16, false> : tag<std::uint16_t> { };
template <> struct MinSizeType<32, false> : tag<std::uint32_t> { };
template <> struct MinSizeType<64, false> : tag<std::uint64_t> { };

template <> struct MinSizeType<8, true> : tag<std::int8_t> { };
template <> struct MinSizeType<16, true> : tag<std::int16_t> { };
template <> struct MinSizeType<32, true> : tag<std::int32_t> { };
template <> struct MinSizeType<64, true> : tag<std::int64_t> { };

/**
 * Templated multiply function implementation. Checks for the signed-ness of each operand,
 * then finds the smallest type that preserves the signed-ness and is at least as wide as the sum of
 * the operand widths.
 */

template <class T1, class T2>
inline auto multiply(T1 x, T2 y) {

	/**
	 * Determine which type is bigger, and choose that one. The product has to be at twice as wide as that.
	 */

	constexpr bool bigger = sizeof(T1) > sizeof(T2);
	using BiggerType = typename std::conditional<bigger, T1, T2>::type;


	/**
	 * Determine which type is signed. If one of the arguments is signed, then a signed type is necessary.
	 */

	using SignedType = typename std::conditional<std::is_signed<T1>::value, T1, T2>::type;

	/**
	 * If both arguments are signed / unsigned, then the output type is the type twice as wide as the larger
	 * width argument. Otherwise, we have these cases to deal with:
	 *
	 * - The signed type is smaller.
	 * - The unsigned type is smaller.
	 * - The signed type is larger.
	 * - The unsigned type is larger.
	 *
	 * Cases 1 and 4 are identical, and cases 2 and 3 are identical. If we are dealing with case 1, then
	 * the output type must be a signed version of the larger type's 'upgrade'. Otherwise, we just make the
	 * signed type larger.
	 */

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

	/**
	 * Using the determined type for the output, we implement a shift-and-add multiply routine.
	 */

	register ProductRetType ans = 0;

	/**
	 * Adding less of the larger number will speed the algorithm up slightly, so we determine the smaller
	 * and larger inputs.
	 */

	register ProductRetType smallerNum = x > y ? y : x;
	const register ProductRetType largerNum = x > y ? x : y;

	/**
	 * Perform the shift-and-add multiplication by multiplying each binary digit of the smaller number
	 * by the larger number to generate a partial product, then shifting that partial product to the left.
	 * The answer is the sum of all of these partial products.
	 */

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

/**
 * XORShift random number generator for various widths. The period of a XORShift generator with width N
 * is -1 + (1 << N). For more randomness, use a bigger one. The tradeoff is performance.
 */

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
