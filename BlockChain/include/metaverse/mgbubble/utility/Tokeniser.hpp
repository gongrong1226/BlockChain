#pragma once

#include <metaverse/mgbubble/utility/String.hpp>

#include <algorithm>

/**
 * @addtogroup Util
 * @{
 */

namespace mgbubble {

	template <char DelimN>
	class Tokeniser {
	public:
		explicit Tokeniser(string_view buf) noexcept { reset(buf); }
		explicit Tokeniser() noexcept { reset(""); }
		~Tokeniser() noexcept = default;

		// Copy.
		Tokeniser(const Tokeniser& rhs) noexcept = default;
		Tokeniser& operator=(const Tokeniser& rhs) noexcept = default;

		// Move.
		Tokeniser(Tokeniser&&) noexcept = default;
		Tokeniser& operator=(Tokeniser&&) noexcept = default;

		string_view top() const noexcept { return buf_.substr(i_ - buf_.cbegin(), j_ - i_); }
		bool empty() const noexcept { return i_ == buf_.cend(); }

		//格式化，定位DelimN
		void reset(string_view buf) noexcept
		{
			buf_ = buf;
			i_ = buf_.cbegin();
			j_ = std::find(i_, buf_.cend(), DelimN);
		}
		void pop() noexcept
		{
			if (j_ != buf_.cend()) {
				i_ = j_ + 1;
				j_ = std::find(i_, buf_.cend(), DelimN);
			}
			else {
				i_ = j_;
			}
		}

	private:
		string_view buf_;
		string_view::const_iterator i_, j_;
	};

} // http
