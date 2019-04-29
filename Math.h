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
#include <limits>

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

/**
 * Pair return type for divmod().
 */

template <class T>
struct DivModPair {
	T quotient;
	T remainder;
};

/**
 * Computes the integer division and modulus of two numbers.
 */

template <class T1, class T2>
inline auto divmod(T1 x, T2 y) {

	/**
	 * Calculate which type is larger by comparing max values. The quotient and remainder have to be able to hold
	 * values at most as large as the larger type of the two.
	 */

	constexpr bool bigger = std::numeric_limits<T1>::max() > std::numeric_limits<T2>::max();
	using BiggerType = typename std::conditional<bigger, T1, T2>::type;

	/**
	 * If the bigger type of the two is unsigned and the smaller type is signed, then we have to widen the result return type
	 * to be as large as the first signed type larger than the bigger type. This is to hold infinity values or large positives.
	 */

	using SmallerType = typename std::conditional<bigger, T2, T1>::type;
	constexpr bool biggerTypeUnsigned = std::is_unsigned<BiggerType>::value && std::is_signed<SmallerType>::value;
	using UnsignedBiggerType = typename std::make_unsigned<BiggerType>::type;

	/**
	 * Performs widen operation in the event a widen operation is needed.
	 */

	using DivType = typename std::conditional <
		biggerTypeUnsigned,
		typename NextSize<UnsignedBiggerType>::type,
		UnsignedBiggerType
	>::type;

	/**
	 * Return type may be signed. If both arguments aren't signed, use an unsigned type.
	 */

	constexpr bool signNotNeeded = std::is_unsigned<T1>::value && std::is_unsigned<T2>::value;
	using RetType = typename std::conditional <
		signNotNeeded, DivType, typename std::make_signed<DivType>::type
	>::type;

	/**
	 * Convert arguments to unsigned for unsigned shift-and-subtract division routine.
	 */

	register RetType dividend = x < static_cast<RetType>(0) ? -x : x;
	register RetType divisor = y < static_cast<RetType>(0) ? -y : y;

	/**
	 * Return values come in a struct of RetTypes.
	 */

	struct DivModPair<RetType> ret;

	if (divisor == 0) {

		/**
		 * If the quotient will be negative, return -infty. Else return +infty.
		 */

		const bool useNegInfty = x < 0;
		constexpr RetType retMax = std::numeric_limits<BiggerType>::max();
		constexpr RetType retMin = -retMax - static_cast<RetType>(1);

		/**
		 * No remainder because infty doesn't have a remainder.
		 */

		ret.quotient = useNegInfty ? retMin : retMax;
		ret.remainder = 0;

		return ret;
	}

	/**
	 * If the quotient is all remainder, just return the remainder with a quotient of 0.
	 */

	if (divisor > dividend) {
		ret.quotient = 0;
		ret.remainder = dividend;

		return ret;
	}

	/**
	 * If the quotient is exactly 1 in integer divisions (x == y) then return 1.
	 */

	if (divisor == dividend) {
		ret.quotient = 1;
		ret.remainder = 0;

		return ret;
	}

	/**
	 * Need to shift the dividend into the remainder to align MSBs.
	 */

	constexpr std::uint8_t retBits = sizeof(DivType) << 3;
    constexpr std::uint8_t bitIndex = retBits - 1;
	constexpr DivType highestBit = static_cast<DivType>(1) << bitIndex;

	register DivType quotient = 0;
	register DivType remainder = 0;
	register DivType d = 0;
	register DivType t = 0;

	/**
	 * Mask the bits of the dividend and push them into the remainder so that subtractions of the divisor
	 * annihilate the dividend, leaving only the remainder. Only perform the subtractions up to where the
	 * remainder starts.
	 */

	std::uint8_t numBits = retBits;
	while (remainder < divisor) {

		/**
		 * Mask the bit and push it.
		 */

		const std::uint8_t bit = (dividend & highestBit) >> bitIndex;
		remainder = (remainder << 1) | bit;

		/**
		 * Shift the dividend and move the pointer to the start of the remainder back.
		 */

		d = dividend;
		dividend <<= 1;
		numBits--;
	}

	/**
	 * Loop always runs 1 time extra so we have to revert the changes.
	 */

	dividend = d;
	remainder >>= 1;
	numBits++;

	/**
	 * Shift and subtract routine to annihilate the dividend, leaving the appropriate remainder.
	 */

	for (std::uint8_t i = 0; i < numBits; i++) {

		/**
		 *
		 */

		const std::uint8_t bit = (dividend & highestBit) >> (retBits - 1);
		remainder = (remainder << 1) | bit;

		t = remainder - divisor;
		const std::uint8_t tempQ = !((t & highestBit) >> (retBits - 1));
		dividend <<= 1;
		quotient = (quotient << 1) | tempQ;
		if (tempQ) {
			remainder = t;
		}
	}

	/**
	 * If either argument is negative then the quotient is negative. Adjust the modulo result too
	 * because the signs matter.
	 */

	const bool neg = x < 0 != y < 0;
	ret.quotient = neg ? -quotient : quotient;

	/**
	 * Change modulo result observing sign.
	 */

	if (x < 0 && y < 0) ret.remainder = -remainder;
	if (x > 0 && y < 0) ret.remainder = remainder - divisor;
	if (x < 0 && y > 0) ret.remainder = divisor - remainder;
	if (x > 0 && y > 0) ret.remainder = remainder;

	return ret;
}

template <class T>
inline T nextHighestPowerOfTwo(T x) {
	x--;
	for (std::size_t i = 0; i < sizeof(x) << 3; ++i) {
		x |= (x >> i);
	}
	x++;
	return x;
}

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
