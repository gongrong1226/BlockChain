#pragma once

#ifndef MVS_ASH_COMPARE_HPP
#define MVS_ASH_COMPARE_HPP

#include <type_traits>

/**
 * @addtogroup Util
 * @{
 */

namespace mgbubble {

	template <typename ValueT>
	class Comparable {
	public:
		friend constexpr bool operator==(const ValueT& lhs, const ValueT& rhs) noexcept
		{
			return lhs.compare(rhs) == 0;
		}

		friend constexpr bool operator!=(const ValueT& lhs, const ValueT& rhs) noexcept
		{
			return lhs.compare(rhs) != 0;
		}

		friend constexpr bool operator<(const ValueT& lhs, const ValueT& rhs) noexcept
		{
			return lhs.compare(rhs) < 0;
		}

		friend constexpr bool operator<=(const ValueT& lhs, const ValueT& rhs) noexcept
		{
			return lhs.compare(rhs) <= 0;
		}

		friend constexpr bool operator>(const ValueT& lhs, const ValueT& rhs) noexcept
		{
			return lhs.compare(rhs) > 0;
		}

		friend constexpr bool operator>=(const ValueT& lhs, const ValueT& rhs) noexcept
		{
			return lhs.compare(rhs) >= 0;
		}

	protected:
		~Comparable() noexcept = default;
	};

	template <typename EnumT, typename std::enable_if_t<std::is_enum<EnumT>::value>* = nullptr>
	constexpr int compare(EnumT lhs, EnumT rhs) noexcept
	{
		int i{};
		if (lhs < rhs) {
			i = -1;
		}
		else if (lhs > rhs) {
			i = 1;
		}
		else {
			i = 0;
		}
		return i;
	}

	template <typename IntegralT,
		typename std::enable_if_t<std::is_integral<IntegralT>::value>* = nullptr>
		constexpr int compare(IntegralT lhs, IntegralT rhs) noexcept
	{
		int i{};
		if (lhs < rhs) {
			i = -1;
		}
		else if (lhs > rhs) {
			i = 1;
		}
		else {
			i = 0;
		}
		return i;
	}

} // http

/** @} */

#endif // MVS_ASH_COMPARE_HPP
