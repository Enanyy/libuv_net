#pragma once
#ifndef UV_UDP_CLIENT_H_
#define UV_UDP_CLIENT_H_
#include <string>
#include <assert.h>
#include <uv.h>
#include "uv_net.h"

namespace uv
{
	class uv_udp_client
	{
		typedef void(*receive_callback)(char* data, size_t length, const struct sockaddr* addr, unsigned flags);
		typedef void(*start_callback)(uv_udp_client*);
	public:
		uv_udp_client(uv_loop_t* loop = uv_default_loop());
		virtual ~uv_udp_client();
	
		bool start_ipv4(const char* ip, const unsigned port, bool broadcast = false);
		bool start_ipv6(const char* ip, const unsigned port, bool broadcast = false);
		void close();

		void send_ipv4(const char* ip, const unsigned port, const char* data, const size_t length);
		void send_ipv6(const char* ip, const unsigned port, const char* data, const size_t length);
		
		void send(const sockaddr* addr, const char* data, const size_t length);
	

		void set_receive_callback(receive_callback callback) { m_receive_callback = callback; }
		void set_start_callback(start_callback callback) { m_start_callback = callback; }

		uv_buf_t& read_buffer() { return m_read_buffer; }
		uv_buf_t& write_buffer() { return m_write_buffer; }

		const std::string& error() { return m_error; }

	protected:
		bool init();
		bool bind_ipv4(const char* ip, const unsigned port);
		bool bind_ipv6(const char* ip, const unsigned port);
		bool set_broadcast(bool enable);
		bool listen();
		bool run();

		void error(int status);

		static void on_alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
		static void on_send(uv_udp_send_t* req, int status);
		static void on_receive(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags);
		static void on_close(uv_handle_t* handle);

	private:
		uv_loop_t*			m_loop;
		uv_udp_t			m_handle;
		uv_udp_send_t		m_req;
		uv_buf_t			m_write_buffer;
		uv_buf_t			m_read_buffer;
		receive_callback	m_receive_callback;
		start_callback		m_start_callback;
	
		std::string			m_error;
		bool				m_init;
	};

	
}


#endif // !UV_UDP_CLIENT_H_
