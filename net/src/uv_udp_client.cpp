#include "uv_udp_client.h"
namespace uv
{
	uv_udp_client::uv_udp_client(uv_loop_t* loop /*= uv_default_loop()*/):
		m_loop(loop),
		m_init(false),
		m_receive_callback(nullptr)
	{
	}

	uv_udp_client::~uv_udp_client()
	{
		close();
	}

	bool uv_udp_client::start_ipv4(const char* ip, const unsigned port , bool broadcast)
	{
		close();
		if (init() ==false)
		{
			LOG("udp client init fail.");
			return false;
		}
		
		if (ip != nullptr && port > 0)
		{
			if (bind_ipv4(ip, port) == false)
			{
				LOG("udp client bind ipv4 fail.");

				return false;
			}

			if (set_broadcast(broadcast) == false)
			{
				LOG("udp client set broadcast fail.");
				return false;
			}
		}

		
		if (listen() == false)
		{
			LOG("udp client listen fail.");
			return false;
		}

		LOG("udp client running.");

		if (m_start_callback != nullptr)
		{
			m_start_callback(this);
		}

		if (run() == false)
		{
			LOG("udp client run fail.");
			return false;
		}

		return true;
	}

	bool uv_udp_client::start_ipv6(const char* ip, const unsigned port, bool broadcast)
	{
		close();
		if (init() == false)
		{
			LOG("udp client init fail.");
			return false;
		}

		if (ip != nullptr && port > 0) 
		{
			if (bind_ipv6(ip, port) == false)
			{
				LOG("udp client bind ipv6 fail.");

				return false;
			}

			if (set_broadcast(broadcast) == false)
			{
				LOG("udp client set broadcast fail.");

				return false;
			}
		}

		if (listen() == false)
		{
			LOG("udp client listen fail.");
			return false;
		}

		LOG("udp client running.");

		if (m_start_callback != nullptr)
		{
			m_start_callback(this);
		}

		if (run() == false)
		{
			LOG("udp client run fail.");
			return false;
		}

		return true;
	}

	bool uv_udp_client::set_broadcast(bool enable)
	{
		int r = uv_udp_set_broadcast(&m_handle, enable ? 1 : 0);
		if (r != 0)
		{
			error(r);
			return false;
		}
		return true;
	}

	void uv_udp_client::close()
	{
		if (m_init)
		{
			uv_close((uv_handle_t*)&m_handle, on_close);

			m_init = false;

			free(m_write_buffer.base);
			free(m_read_buffer.base);

			m_write_buffer.base = nullptr;
			m_read_buffer.base = nullptr;

			m_write_buffer.len = 0;
			m_read_buffer.len = 0;
		}
	}

	void uv_udp_client::send_ipv4(const char* ip, const unsigned port, const char* data, const size_t length)
	{
		struct sockaddr_in addr;
		int r = uv_ip4_addr(ip, port,&addr);
		if (r != 0)
		{
			error(r);
			return;
		}

		send((const sockaddr*)&addr, data, length);

	}
	void uv_udp_client::send_ipv6(const char* ip, const unsigned port, const char* data, const size_t length)
	{
		struct sockaddr_in6 addr;
		int r = uv_ip6_addr(ip, port, &addr);
		if (r != 0)
		{
			error(r);
			return;
		}
		send((const sockaddr*)&addr, data, length);
	}

	void uv_udp_client::send(const sockaddr* addr, const char* data, const size_t length)
	{
		if (m_write_buffer.len < length)
		{
			//重新分配内存
			m_write_buffer.base = (char*)realloc(m_write_buffer.base, length);
			m_write_buffer.len = length;
		}
		else
		{
			memset(m_write_buffer.base, 0, m_write_buffer.len);
		}

		memcpy(m_write_buffer.base, data, length);
		m_write_buffer.len = length;

		int r = uv_udp_send(&m_req, &m_handle, &m_write_buffer, 1, addr, on_send);
		if (r != 0)
		{
			error(r);
		}
	}
	
	bool uv_udp_client::init()
	{
		if (m_init)
		{
			return true;
		}
		if (!m_loop)
		{
			return false;
		}

		int r = uv_udp_init(m_loop, &m_handle);
		if (r != 0)
		{
			error(r);
			return false;
		}

		m_handle.data = this;

		m_write_buffer = uv_buf_init((char*)malloc(BUFFER_SIZE), BUFFER_SIZE);
		m_read_buffer = uv_buf_init((char*)malloc(BUFFER_SIZE), BUFFER_SIZE);

		m_init = true;

		return true;
	}
	bool uv_udp_client::bind_ipv4(const char* ip, const unsigned port)
	{
		struct sockaddr_in addr;
		int r = uv_ip4_addr(ip, port, &addr);
		if (r != 0)
		{
			error(r);
			return false;
		}

		r = uv_udp_bind(&m_handle, (const sockaddr*)&addr, 0);
		if (r != 0)
		{
			error(r);
			return false;
		}

		return true;
	}

	bool uv_udp_client::bind_ipv6(const char* ip, const unsigned port)
	{
		struct sockaddr_in6 addr;
		int r = uv_ip6_addr(ip, port, &addr);
		if (r != 0)
		{
			error(r);
			return false;
		}

		r = uv_udp_bind(&m_handle, (const sockaddr*)&addr, 0);
		if (r != 0)
		{
			error(r);
			return false;
		}

		return true;
	}

	bool uv_udp_client::listen()
	{
		int r = uv_udp_recv_start(&m_handle, on_alloc_buffer, on_receive);
		if (r != 0)
		{
			error(r);
			return false;
		}
		return true;
	}

	bool uv_udp_client::run()
	{
		int r = uv_run(m_loop, UV_RUN_DEFAULT);
		if (r != 0)
		{
			error(r);

			return false;
		}
		return true;
	}

	void uv_udp_client::error(int status)
	{
		m_error = uv_strerror(status);
		LOG(m_error.c_str());
	}

	void uv_udp_client::on_alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
	{
		assert(handle->data != nullptr);
		uv_udp_client* client = (uv_udp_client*)handle->data;
		*buf = client->read_buffer();
	}
	void uv_udp_client::on_send(uv_udp_send_t* req, int status)
	{
		if (status != 0)
		{
			fprintf(stderr, "%s\n", uv_strerror(status));
		}
		else
		{
			LOG("send success callback.");
		}
	}

	void uv_udp_client::on_receive(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)
	{
		uv_udp_client* client = (uv_udp_client*)handle->data;

		if (nread > 0)
		{
			if (client->m_receive_callback != nullptr)
			{
				client->m_receive_callback(buf->base, buf->len, addr, flags);
			}
		}
		else if (nread == 0)
		{
			/* Everything OK, but nothing read. */
		}
		else
		{

			if (nread == UV_EOF) {

				fprintf(stdout, "udp client disconnected, close it.\n");
			}
			else if (nread == UV_ECONNRESET) {
				fprintf(stdout, "udp client disconnected unusually, close it.\n");
			}
			else
			{
				client->error(nread);
			}
			client->close();
		}
	}

	void uv_udp_client::on_close(uv_handle_t* handle)
	{
		if (handle)
		{
			free(handle);
		}

		LOG("udp client close.");
	}
}