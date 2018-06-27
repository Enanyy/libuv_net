#pragma once
#ifndef UV_TCP_SESSION_H_
#define UV_TCP_SESSION_H_
#include <string>
#include <uv.h>
#include "uv_tcp_server.h"
#include "uv_net.h"

namespace uv {

	class uv_tcp_server;
	class uv_tcp_session
	{
		typedef void(*receive_callback)(uv_tcp_session* session, const char* buf, size_t length);

	public:
		uv_tcp_session(int id, uv_tcp_server* server);
		virtual ~uv_tcp_session();
		

		int				id()							const { return m_id; }
		uv_tcp_t*		handle()						const { return m_handle; }
		uv_tcp_server*	server()						const { return m_server; }
		void			server(uv_tcp_server* server) { m_server = server; }
		void			set_receive_callback(receive_callback callback) { m_receive_callback = callback; }
		uv_buf_t&		write_buffer() { return m_write_buffer; }
		uv_buf_t&		read_buffer() { return m_read_buffer; }
		uv_write_t&     req() { return m_req; }

		void			on_receive(const char* buf, size_t length);
		void			send(const char* data, const size_t length);
		

	private:
		int					m_id;
		uv_tcp_t*			m_handle;
		uv_tcp_server*		m_server;
		uv_buf_t			m_read_buffer;
		uv_buf_t			m_write_buffer;
		uv_write_t			m_req;
		receive_callback	m_receive_callback;
	};
}


#endif // !UV_TCP_SESSION_H_
