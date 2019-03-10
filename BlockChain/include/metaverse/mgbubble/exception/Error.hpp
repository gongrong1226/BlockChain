#pragma once
#ifndef MVSD_EXCEPTION_HPP
#define MVSD_EXCEPTION_HPP

#include <metaverse/mgbubble/exception/Exception.hpp>

/**
 * @addtogroup App
 * @{
 */

namespace mgbubble {

	class Error : public Exception {
	public:
		explicit Error(string_view what) noexcept : Exception{ what } {}
		~Error() noexcept = default;

		// Copy.
		Error(const Error&) noexcept = default;
		Error& operator=(const Error&) noexcept = default;

		// Move.
		Error(Error&&) noexcept = default;
		Error& operator=(Error&&) noexcept = default;
	};

} // http

/** @} */

#endif // MVSD_EXCEPTION_HPP