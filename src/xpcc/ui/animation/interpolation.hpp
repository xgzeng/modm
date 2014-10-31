// coding: utf-8
/* Copyright (c) 2014, Roboterclub Aachen e.V.
* All Rights Reserved.
*
* The file is part of the xpcc library and is released under the 3-clause BSD
* license. See the file `LICENSE` for the full license governing this code.
*/
// ----------------------------------------------------------------------------

#ifndef XPCC_UI_LINEAR_INTERPOLATION_HPP
#define XPCC_UI_LINEAR_INTERPOLATION_HPP

#include <stdint.h>
#include <xpcc/utils/arithmetic_traits.hpp>
#include <xpcc/utils/template_metaprogramming.hpp>

namespace xpcc
{

namespace ui
{

/**
 * This class allows the linear interpolation of one value over a number of steps.
 *
 * All integer types use binary scaling through a fixed-point
 * arithmetic, however, all other types default to floating-point
 * arithmetic with casting between types.
 *
 * Be aware that the algortihm for 8bit types is optimized for low computational costs,
 * originally developed for fast LED fading (@see xpcc::ui::Led).
 * Therefore the steps are limited to `128 steps * value_difference`, which is
 * 32768 steps over the full 8bit range.
 * If you specify a more steps in this case, the interpolation simply finishes before.
 * If this is a problem, consider using a 16bit type, which does not have
 * this limitation.
 *
 * @author	Niklas Hauser
 * @ingroup ui
 */
template< typename T = uint8_t >
class LinearInterpolation
{
private:
	typedef typename xpcc::ArithmeticTraits<T>::UnsignedType UnsignedType;
public:
	/// for 8bit value types, the steps are limited to 2^15 anyway,
	/// so we do not need uint32_t for the steps, but we can use uint16_t
	typedef typename xpcc::tmp::Select<
			xpcc::tmp::SameType<UnsignedType, uint8_t>::value,
			uint16_t,
			uint32_t >::Result StepType;
private:
	/// @internal
	/// the default implementation uses floating-point arithmetic
	template< typename Type, typename Unsigned >
	class Computations
	{
		float accumulatedValue;
		float deltaValue;

	public:
		void inline
		initialize(Type currentValue, Type endValue, uint32_t steps)
		{
			float delta = (static_cast<float>(endValue) - currentValue);
			deltaValue = delta / steps;
			if (deltaValue == 0)
				deltaValue = delta > 0 ? xpcc::ArithmeticTraits<float>::epsilon : -xpcc::ArithmeticTraits<float>::epsilon;
			accumulatedValue = static_cast<float>(currentValue) + deltaValue / 2;
		}

		Type inline
		step()
		{
			accumulatedValue += deltaValue;
			return static_cast<Type>(accumulatedValue);
		}
	};

	/// @internal
	/// uint8_t implementation using signed 8.7 fixed-point arithmetic.
	/// The maximum change can be +-255 since the value type is 8 bit wide.
	/// The remaining 7 bits are used for fractional delta value:
	/// 1 value difference can take at most 128 steps, which is equivalent
	/// to 2^15 (32768) steps over the whole 8bit range.
	template< typename Type >
	struct Computations <Type, uint8_t>
	{
		uint16_t accumulatedValue;
		int16_t deltaValue;

		void inline
		initialize(Type currentValue, Type endValue, uint16_t steps)
		{
			int16_t delta = (static_cast<int16_t>(endValue) - currentValue) << 7;
			deltaValue = delta / static_cast<int16_t>(steps);
			if (deltaValue == 0)
				deltaValue = delta > 0 ? 1 : -1;
			accumulatedValue = (static_cast<uint16_t>(currentValue) << 7) + deltaValue / 2;
		}

		Type inline
		step()
		{
			accumulatedValue += deltaValue;
			return static_cast<Type>(accumulatedValue >> 7);
		}
	};

	/// @internal
	/// uint16_t implementation using signed 16.15 fixed point arithmetic.
	/// The maximum change can be +-65535 since the value type is 16 bit wide.
	/// The remaining 15 bits are used for fractional delta value:
	/// 1 value difference can take at most 32768 steps, which is equivalent
	/// to 2^31 steps over the whole 16bit range.
	template< typename Type >
	struct Computations <Type, uint16_t>
	{
		uint32_t accumulatedValue;
		int32_t deltaValue;

		void inline
		initialize(Type currentValue, Type endValue, uint32_t steps)
		{
			int32_t delta = (static_cast<int32_t>(endValue) - currentValue) << 15;
			deltaValue = delta / static_cast<int32_t>(steps);
			if (deltaValue == 0)
				deltaValue = delta > 0 ? 1 : -1;
			accumulatedValue = (static_cast<uint32_t>(currentValue) << 15) + deltaValue / 2;
		}

		Type inline
		step()
		{
			accumulatedValue += deltaValue;
			return static_cast<Type>(accumulatedValue >> 15);
		}
	};

	// On the AVR 64bit variables do not exist, therefore this must be excluded,
	// so that it may revert back to using floats.
	// It's not pretty, but neither is using uint32_t on an 8bit CPU to begin with.
#if !defined(XPCC__CPU_AVR)

	/// @internal
	/// uint32_t implementation using signed 32.16 fixed point arithmetic.
	/// The maximum change can be +-2^32 since the value type is 32 bit wide.
	/// The remaining 16 bits are used for fractional delta steps:
	/// 1 value difference can take at most 65356 steps, which is equivalent
	/// to 2^63 steps over the whole range.
	/// Note: we use 16 bits for the fractionals here, so that it is byte-aligned.
	/// This should allow CPUs without a barrelshifter to copy the value instead of shifting it.
	template< typename Type >
	struct Computations <Type, uint32_t>
	{
		uint64_t accumulatedValue;
		int64_t deltaValue;

		void inline
		initialize(Type currentValue, Type endValue, uint32_t steps)
		{
			int64_t delta = (static_cast<int64_t>(endValue) - currentValue) << 16;
			deltaValue = delta / static_cast<int32_t>(steps);
			if (deltaValue == 0)
				deltaValue = delta > 0 ? 1 : -1;
			accumulatedValue = (static_cast<uint64_t>(currentValue) << 16) + deltaValue / 2;
		}

		Type inline
		step()
		{
			accumulatedValue += deltaValue;
			return static_cast<Type>(accumulatedValue >> 16);
		}
	};
#endif

	// create an instance of the calculation helper
	Computations<T, UnsignedType> computations;

public:
	/*
	 *
	 */
	void inline
	initialize(T currentValue, T endValue, StepType steps)
	{
		computations.initialize(currentValue, endValue, steps);
	}

	T inline
	step()
	{
		return computations.step();
	}

	void inline
	reset()
	{
		computations.deltaValue = 0;
	}
};

}	// namespace ui

}	// namespace xpcc

#endif	// XPCC_UI_LINEAR_INTERPOLATION_HPP