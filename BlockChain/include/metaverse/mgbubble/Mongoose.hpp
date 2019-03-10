#pragma once

#ifndef MVSD_MONGOOSE_HPP
#define MVSD_MONGOOSE_HPP

#include <vector>
#include <metaverse/mgbubble/utility/Queue.hpp>
#include <metaverse/mgbubble/utility/String.hpp>
#include <metaverse/mgbubble/exception/Error.hpp>
#include <mongoose.h>
/**
 * @addtogroup Web
 * @{
 */

namespace mgbubble {

#define SESSION_COOKIE_NAME "2iBXdhW9rQxbnDdNQk9KdjiytM9X"

	inline string_view operator+(const mg_str& str) noexcept
	{
		return { str.p, str.len };//字符串p构造，并指定大小为len
	}

	inline string_view operator+(const websocket_message& msg) noexcept
	{
		return { reinterpret_cast<char*>(msg.data), msg.size };
	}

	class ToCommandArg {
	public:
		auto argv() const noexcept { return argv_; }
		auto argc() const noexcept { return argc_; }
		const auto& vargv() const noexcept { return vargv_; }
		const auto& get_command() const {
			if (!vargv_.empty())
				return vargv_[0];
			throw std::logic_error{ "no command found" };
		}
		void add_arg(std::string&& outside);

		//最大command数量
		static const int max_paramters{ 32 };
	protected:

		virtual void data_to_arg() = 0;
		const char* argv_[max_paramters]{ nullptr };
		int argc_{ 0 };

		std::vector<std::string> vargv_;
	};


	class HttpMessage : public ToCommandArg {
	public:
		HttpMessage(http_message* impl) noexcept : impl_{ impl } {}
		~HttpMessage() noexcept = default;

		// Copy.
		// http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#1778
		HttpMessage(const HttpMessage&) = default;
		HttpMessage& operator=(const HttpMessage&) = default;

		// Move.
		HttpMessage(HttpMessage&&) = default;
		HttpMessage& operator=(HttpMessage&&) = default;

		//得到http_message
		auto get() const noexcept { return impl_; }
		auto method() const noexcept { return +impl_->method; }
		auto uri() const noexcept { return +impl_->uri; } //return { uri.p, uri.len };
		auto proto() const noexcept { return +impl_->proto; }
		auto queryString() const noexcept { return +impl_->query_string; }
		auto header(const char* name) const noexcept
		{
			//Searches and returns the header name in parsed HTTP message hm. If header is not found, NULL is returned. 
			//找标头
			auto* val = mg_get_http_header(impl_, name);
			return val ? +*val : string_view{};
		}
		auto body() const noexcept { return +impl_->body; }

		void data_to_arg() override;

	private:

		http_message* impl_;
	};

	class WebsocketMessage :public ToCommandArg { // connect to bx command-tool
	public:
		WebsocketMessage(websocket_message* impl) noexcept : impl_{ impl } {}
		~WebsocketMessage() noexcept = default;

		// Copy.
		WebsocketMessage(const WebsocketMessage&) = default;
		WebsocketMessage& operator=(const WebsocketMessage&) = default;

		// Move.
		WebsocketMessage(WebsocketMessage&&) = default;
		WebsocketMessage& operator=(WebsocketMessage&&) = default;

		auto get() const noexcept { return impl_; }
		auto data() const noexcept { return reinterpret_cast<char*>(impl_->data); }
		auto size() const noexcept { return impl_->size; }

		void data_to_arg() override;
	private:
		websocket_message* impl_;
	};

	struct Session {
		Session() = default;
		Session(uint64_t a1, double a2, double a3,
			std::string&& a4, std::string a5) :
			id(a1), created(a2), last_used(a3), user(a4), pass(a5) {}
		~Session() = default;

		uint64_t        id;
		double          created;
		double          last_used;
		std::string     user;
		std::string     pass;
	};



	//事件管理器Mongoose event manager.
	template <typename DerivedT> 
	class Mgr {
	public:
		// Copy.
		Mgr(const Mgr&) = delete;
		Mgr& operator=(const Mgr&) = delete;

		// Move.
		Mgr(Mgr&&) = delete;
		Mgr& operator=(Mgr&&) = delete;

		mg_connection& bind(const char* addr)
		{
#if MG_ENABLE_MUTITHREADS
			auto* conn = mg_bind(&mgr_, addr, handler_mt);
#else
			auto* conn = mg_bind(&mgr_, addr, handler);
#endif
			if (!conn)
				throw Error{ "mg_bind() failed" };
			conn->user_data = this;
			return *conn;
		}

		//迭代所有的sockets,接受新的连接，发送和接收数据，关闭连接，调用指定的事件的回调函数。
		time_t poll(int milli) { return mg_mgr_poll(&mgr_, milli); }

		// session control
		static void login_handler(mg_connection* conn, int ev, void* data) {
			http_message* hm = static_cast<http_message*>(data);
			auto* self = static_cast<DerivedT*>(conn->user_data);

			if (mg_vcmp(&hm->method, "POST") != 0) {
				mg_serve_http(conn, hm, self->get_httpoptions());
			}
			else {
				if (self->user_auth(*conn, hm)) {
					std::ostringstream shead;
					auto ret = self->push_session(hm);
					shead << "Set-Cookie: " SESSION_COOKIE_NAME "=" << ret->id << "; path=/";
					mg_http_send_redirect(conn, 302, mg_mk_str("/"), mg_mk_str(shead.str().c_str()));
				}
			}
			conn->flags |= MG_F_SEND_AND_CLOSE;/* Push remaining data and close  */
		}

		static void logout_handler(mg_connection* conn, int ev, void* data) {
			http_message* hm = static_cast<http_message*>(data);
			auto* self = static_cast<DerivedT*>(conn->user_data);

			std::ostringstream shead;
			shead << "Set-Cookie: " SESSION_COOKIE_NAME "=";
			mg_http_send_redirect(conn, 302, mg_mk_str("/login.html"), mg_mk_str(shead.str().c_str()));
			self->remove_from_session_list(hm);

			conn->flags |= MG_F_SEND_AND_CLOSE;
		}

		constexpr static const double session_check_interval = 5.0;
		static const int thread_num_ = 2;

	protected:
		Mgr() noexcept {
			mg_mgr_init(&mgr_, this);
#if MG_ENABLE_MUTITHREADS
			start();
#endif
		}
		~Mgr() noexcept { mg_mgr_free(&mgr_); }

	private:

#if MG_ENABLE_MUTITHREADS
		void start() {
			for (int i = 0; i < thread_num_; i++) {
				std::thread th([this, i] {
					mg_mgr& mg = child_mgrs_[i];
					mg_mgr_init(&mg, this);

					Queue<mg_connection*> &queue = queue_connections[i];

					while (1) {
						while (1) {
							mg_connection* conn = NULL;
							if (queue.pop(conn) == false) {
								break;
							}
							conn->handler = handler;
							mg_add_conn(&mg, conn);
						}

						mg_mgr_poll(&mg, 100);
					}
				});
				th.detach();
			}
		}

		static void handler_mt(mg_connection* conn, int event, void* data)
		{
			if (event == MG_EV_ACCEPT) {
				if (conn) {
					mg_remove_conn(conn);
					int index = conn->sock % thread_num_;
					queue_connections[index].push(conn);
					MG_SEND_FUNC(child_mgrs_[index].mthread_ctl[1], "a", 1, 0);
				}
			}
		}
#endif

		static void handler(mg_connection* conn, int event, void* data)
		{
			http_message* hm = static_cast<http_message*>(data);
			websocket_message* ws = static_cast<websocket_message*>(data);
			auto* self = static_cast<DerivedT*>(conn->user_data);

			switch (event) {
			case MG_EV_CLOSE: {
				if (conn->flags & MG_F_IS_WEBSOCKET) {
					//self->websocketBroadcast(*conn, "left", 4);
				}
				else {
					conn->user_data = nullptr;
				}
				break;
			}
			case MG_EV_HTTP_REQUEST: {//HTTP请求到达

				// rpc call
				if (mg_ncasecmp((&hm->uri)->p, "/rpc", 4u) == 0) {
					self->httpRpcRequest(*conn, hm);
					break;
				}
				else {
					self->httpStatic(*conn, hm);
					conn->flags |= MG_F_SEND_AND_CLOSE;
					break;
				}
			}
			case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {//服务器已完成WebSocket握手
				self->websocketSend(conn, "connected", 9);
				break;
			}
			case MG_EV_WEBSOCKET_FRAME: { //新的WebSocket帧数据已到达
				self->websocketSend(*conn, ws);
				break;
			}
			case MG_EV_SSI_CALL: {
			}
			case MG_EV_TIMER: {	//case mg_set_timer()
				self->check_sessions();
				mg_set_timer(conn, mg_time() + session_check_interval);
				break;
			}
			}// switch
		}// handler

		//事件管理器
		mg_mgr mgr_;
#if MG_ENABLE_MUTITHREADS
		static Queue<mg_connection*> queue_connections[thread_num_];
		static mg_mgr child_mgrs_[thread_num_];
#endif
	};

#if MG_ENABLE_MUTITHREADS
	template<typename DerivedT >
	Queue<mg_connection*> Mgr<DerivedT>::queue_connections[thread_num_];

	template<typename DerivedT >
	mg_mgr Mgr<DerivedT>::child_mgrs_[thread_num_];
#endif

} // http

/** @} */

#endif // MVSD_MONGOOSE_HPP