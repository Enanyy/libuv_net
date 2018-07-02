#include "uv_tcp_session.h"

namespace uv
{
	uv_tcp_session::uv_tcp_session(int id, uv_tcp_server* server) :
		m_id(id),
		m_server(server),
		m_receive_callback(nullptr)
	{
		m_handle = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
		m_handle->data = this;
		m_read_buffer = uv_buf_init((char*)malloc(BUFFER_SIZE), BUFFER_SIZE);
		m_write_buffer = uv_buf_init((char*)malloc(BUFFER_SIZE), BUFFER_SIZE);

	}
	uv_tcp_session::~uv_tcp_session()
	{
		free(m_read_buffer.base);
		free(m_write_buffer.base);

		m_read_buffer.base = nullptr;
		m_write_buffer.base = nullptr;
		m_read_buffer.len = 0;
		m_write_buffer.len = 0;

		free(m_handle);
		m_handle = nullptr;
	}
	void uv_tcp_session::on_receive(const char* buf, size_t length)
	{
		if (m_receive_callback != nullptr)
		{
			m_receive_callback(this, buf, length);
		}
	}

	void uv_tcp_session::send(const char* data, const size_t length)
	{
		if (m_server == nullptr)
		{
			return;
		}
		m_server->send(m_id, data, length);
	}
}