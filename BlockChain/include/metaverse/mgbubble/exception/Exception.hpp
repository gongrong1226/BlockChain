#pragma once

#include <cstring> // strcpy()
#include <exception>
#include <metaverse/mgbubble/utility/Stream.hpp>


namespace mgbubble {

	/**
	 * Maximum error message length.
	 */
	constexpr std::size_t ErrMsgMax{ 127 };

	using ErrMsg = StringBuilder<ErrMsgMax>;

	class MVS_API Exception : public std::exception {
	public:
		explicit Exception(string_view what) noexcept;

		~Exception() noexcept override;

		// Copy.
		Exception(const Exception& rhs) noexcept { *this = rhs; }
		Exception& operator=(const Exception& rhs) noexcept
		{
			std::strcpy(what_, rhs.what_);
			return *this;
		}

		// Move.
		Exception(Exception&&) noexcept = default;
		Exception& operator=(Exception&&) noexcept = default;

		const char* what() const noexcept override;

	private:
		char what_[ErrMsgMax + 1];
	};

	/**
	 * Thread-local error message. This thread-local instance of StringBuilder can be used to format
	 * error messages before throwing. Note that the StringBuilder is reset each time this function is
	 * called.
	 */
	MVS_API ErrMsg& errMsg() noexcept;

} // mgbubble
