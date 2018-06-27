#pragma once
#ifndef UV_TCP_SERVER_H_
#define UV_TCP_SERVER_H_

#include <map>
#include <string>
#include <memory>
#include <assert.h>
#include <uv.h>
#include "uv_tcp_session.h"

namespace uv
{
	class uv_tcp_session;
	
	class uv_tcp_server
	{
		typedef void(*connect_callback)(uv_tcp_session* session);
		typedef void(*receive_callback)(uv_tcp_session* session, const char* buf, size_t length);

	public:
		uv_tcp_server(uv_loop_t* loop = uv_default_loop());
		virtual ~uv_tcp_server();

		bool			start_ipv4(const char* ip, const unsigned port);
		bool			start_ipv6(const char* ip, const unsigned port);

		void			close();
		virtual void	send(int sessionId, const char* data, const size_t length);
		virtual void	set_connect_callback(connect_callback callback);
		virtual void	set_receive_callback(int sessionId,receive_callback callback);
		bool			set_no_delay(bool enable);
		bool			set_keep_alive(int enable, unsigned int delay);
		
		const char*		error() { return m_error.c_str(); }

		const uv_tcp_session* session(int sessionId) const;

	protected:
		bool		close(int sessionId);

		static void on_accept(uv_stream_t* server, int status);
		static void on_receive(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);
		static void on_send(uv_write_t* req, int statu);
		static void on_alloc_buffer(uv_handle_t* hanle, size_t suggested_size, uv_buf_t* buf);
		static void on_close(uv_handle_t* handle);
		static void on_client_close(uv_handle_t* handle);

	private:
		bool init();
		bool run();

		bool bind_ipv4(const char* ip, const unsigned port);
		bool bind_ipv6(const char* ip, const unsigned port);
		bool listen(int backlog = 1024);

		void error(int status) ;

	private:
		uv_tcp_t						m_server;
		std::map<int, uv_tcp_session*>	m_sessions;
		uv_mutex_t						m_mutex;
		uv_loop_t*						m_loop;
		std::string						m_error;
		connect_callback				m_connect_callback;
		bool							m_init;
	};

}

#endif // !UV_TCP_SERVER_H_