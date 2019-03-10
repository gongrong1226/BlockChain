#pragma once

#ifndef MVSD_STREAM_HPP
#define MVSD_STREAM_HPP

#include <mongoose.h>

#include <ostream>

/**
 * @addtogroup App
 * @{
 */

namespace mgbubble {

	class StreamBuf : public std::streambuf {
	public:
		explicit StreamBuf(mbuf& buf) throw(std::bad_alloc);
		~StreamBuf() noexcept override;

		// Copy.
		StreamBuf(const StreamBuf& rhs) = delete;
		StreamBuf& operator=(const StreamBuf& rhs) = delete;

		// Move.
		StreamBuf(StreamBuf&&) = delete;
		StreamBuf& operator=(StreamBuf&&) = delete;

		const char_type* data() const noexcept { return buf_.buf; }
		std::streamsize size() const noexcept { return buf_.len; }
		void reset() noexcept;
		void setContentLength(size_t pos, size_t len) noexcept;

	protected:
		int_type overflow(int_type c) noexcept override;

		std::streamsize xsputn(const char_type* s, std::streamsize count) noexcept override;

	private:
		mbuf& buf_;
	};

	class OStream : public std::ostream {
	public:
		OStream();
		~OStream() noexcept override;

		// Copy.
		OStream(const OStream& rhs) = delete;
		OStream& operator=(const OStream& rhs) = delete;

		// Move.
		OStream(OStream&&) = delete;
		OStream& operator=(OStream&&) = delete;

		StreamBuf* rdbuf() const noexcept { return static_cast<StreamBuf*>(std::ostream::rdbuf()); }
		const char_type* data() const noexcept { return rdbuf()->data(); }
		std::streamsize size() const noexcept { return rdbuf()->size(); }
		StreamBuf* rdbuf(StreamBuf* sb) noexcept
		{
			return static_cast<StreamBuf*>(std::ostream::rdbuf(sb));
		}
		void reset(int status, const char* reason,
					const char *content_type = "text/plain",/*将文件设置为纯文本的形式，浏览器在获取到这种文件时并不会对其进行处理*/
					const char *charset = "utf-8") noexcept;
		void setContentLength() noexcept;

	private:
		size_t headSize_{ 0 };
		size_t lengthAt_{ 0 };
	};

} // http

/** @} */

#endif // MVSD_STREAM_HPP