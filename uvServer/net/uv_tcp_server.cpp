#include "uv_tcp_server.h"


namespace uv
{
	uv_tcp_server::uv_tcp_server(uv_loop_t* loop /* = uv_default_loop() */):
		m_connect_callback(nullptr),m_init(false)
	{
		m_loop = loop;
	}

	uv_tcp_server:: ~uv_tcp_server()
	{
		m_connect_callback = nullptr;

		close();
		LOG("tcp server exit.");
	}

	void uv_tcp_server::error(int status)
	{
		m_error = uv_strerror(status);

		printf("%s.\n",m_error.c_str());
	}

	bool uv_tcp_server::start_ipv4(const char* ip, const unsigned port)
	{
		close();
		if (!init())
		{
			LOG("init tcp ipv4 server fail.");

			return false;
		}

		if (bind_ipv4(ip, port) == false)
		{
			LOG("bind tcp ipv4 server fail.");

			return false;
		}

		if (listen() == false)
		{
			LOG("listen tcp ipv4 server fail.");

			return false;
		}

		LOG("tcp server starting.");

		if (run() == false)
		{
			LOG("run tcp ipv4 server fail.");

			return false;
		}

		

		return true;
	}

	bool uv_tcp_server::start_ipv6(const char* ip, const unsigned port)
	{
		close();
		if (!init())
		{
			LOG("init tcp ipv6 server fail.");
			return false;
		}

		if (bind_ipv6(ip, port) == false)
		{
			LOG("bind tcp ipv6 server fail.");
			return false;
		}

		if (listen() == false)
		{
			LOG("listen tcp ipv6 server fail.");
			return false;
		}

		if (run() == false)
		{
			LOG("run tcp ipv6 server fail.");
			return false;
		}

		return true;
	}

	void uv_tcp_server::close()
	{
		for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it)
		{
			auto c = it->second;
			uv_close((uv_handle_t*)c->handle(), on_client_close);
		}
		m_sessions.clear();

		if (m_init)
		{
			uv_close((uv_handle_t*)&m_server, on_close);	
			uv_loop_close(m_loop);

		}
		m_init = false;
		
		uv_mutex_destroy(&m_mutex);
	}

	bool uv_tcp_server::close(int sessionId)
	{
		uv_mutex_lock(&m_mutex);
		auto it = m_sessions.find(sessionId);
		if (it == m_sessions.end())
		{
			uv_mutex_unlock(&m_mutex);

			return false;
		}

		auto handle = it->second->handle();

		if (uv_is_active((uv_handle_t*)handle))
		{
			uv_read_stop((uv_stream_t*)handle);
		}

		uv_close((uv_handle_t*)handle, on_client_close);
		m_sessions.erase(it);
	
		uv_mutex_unlock(&m_mutex);
		
		return true;
	}

	bool uv_tcp_server::init() 
	{
		if (m_init)
		{
			return true;
		}

		if (!m_loop)
		{
			return false;
		}

		int r = uv_mutex_init(&m_mutex);
		if (r != 0)
		{
			error(r);
			return false;
		}

		r = uv_tcp_init(m_loop, &m_server);
		if (r != 0)
		{
			error(r);
			return false;
		}

		m_init = true;

		m_server.data = this;

		return m_init;

	}
	bool uv_tcp_server::run()
	{
		int r = uv_run(m_loop, UV_RUN_DEFAULT);
		if (r != 0)
		{
			error(r);

			return false;
		}
		return true;
	}

	bool uv_tcp_server::bind_ipv4(const char* ip, const unsigned port)
	{
		struct sockaddr_in addr;
		int r = uv_ip4_addr(ip, port, &addr);
		if (r != 0)
		{	
			error(r);

			return false;
		}
		r = uv_tcp_bind(&m_server,(const sockaddr*)&addr, 0);
		if (r != 0)
		{			
			error(r);

			return false;
		}	
		return true;
	}
	bool uv_tcp_server::bind_ipv6(const char* ip, const unsigned port)
	{
		struct sockaddr_in6 addr;
		int r = uv_ip6_addr(ip, port,&addr);
		if (r != 0)
		{			
			error(r);
			return false;
		}
		r = uv_tcp_bind(&m_server, (const sockaddr*)&addr, 0);
		if (r != 0)
		{			
			error(r);

			return false;
		}

		return true;
	}
	bool uv_tcp_server::listen(int backlog /*= 1024*/)
	{
		int r = uv_listen((uv_stream_t*)&m_server, backlog, on_accept);
		if (r != 0)
		{			
			error(r);

			return false;
		}
		return true;
	}


	void uv_tcp_server::send(int sessionId, const char* data, const std::size_t length)
	{
		auto it = m_sessions.find(sessionId);
		if (it == m_sessions.end())
		{
			LOG("can't find client to send.");
			return;
		}

		auto buffer = it->second->write_buffer();

		if (buffer.len < length)
		{
			//重新分配内存
			buffer.base = (char*)realloc(buffer.base, length);
			buffer.len = length;
		}
		else
		{
			memset(buffer.base, 0, buffer.len);
		}

		memcpy(buffer.base, data, length);
		buffer.len = length;
	
		int  r = uv_write(&it->second->req(), (uv_stream_t*)it->second->handle(), &buffer, 1, on_send);

		if (r != 0)
		{
			error(r);
		}
	}

	void uv_tcp_server::set_connect_callback(connect_callback callback)
	{
		m_connect_callback = callback;
	}

	void uv_tcp_server::set_receive_callback(int clientId, receive_callback callback)
	{
		auto it = m_sessions.find(clientId);
		if (it == m_sessions.end())
		{
			LOG("can't find client.");
			return;
		}

		it->second->set_receive_callback(callback);
	}

	bool uv_tcp_server::set_no_delay(bool enable) 
	{
		int r = uv_tcp_nodelay(&m_server, enable ? 1 : 0);
		if (r != 0)
		{	
			error(r);
			return false;
		}
		return true;
	}

	bool uv_tcp_server::set_keep_alive(int enable, unsigned int delay)
	{
		int r = uv_tcp_keepalive(&m_server, enable, delay);
		if (r != 0)
		{		
			error(r);
			return false;
		}
		return true;
	}

	void uv_tcp_server::on_receive(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf)
	{
		if (client->data == nullptr)
		{
			return;
		}

		uv_tcp_session* session = (uv_tcp_session*)client->data;

		if (nread > 0)
		{
			session->on_receive(buf->base, nread);
		}
		else if(nread == 0)
		{
			/* Everything OK, but nothing read. */
		}
		else
		{
			auto server = session->server();

			if (nread == UV_EOF) {

				fprintf(stdout, "client %d disconnected, close it.\n", session->id());
			}
			else if (nread == UV_ECONNRESET) {
				fprintf(stdout, "client %d disconnected unusually, close it.\n", session->id());
			}
			else
			{
				server->error(nread);
			}
			server->close(session->id());
		}
		
	}

	void uv_tcp_server::on_send(uv_write_t* req, int status)
	{
		if (status < 0)
		{
			printf(uv_strerror(status));
		}
		
	}

	void uv_tcp_server::on_alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
	{
		assert(handle->data != nullptr);

		uv_tcp_session* session = (uv_tcp_session*)handle->data;

		*buf = session->read_buffer();
	
	}

	void uv_tcp_server::on_close(uv_handle_t* handle) 
	{
		free(handle);
		LOG("tcp server close callback.\n");
	}

	void uv_tcp_server::on_client_close(uv_handle_t* handle)
	{
		uv_tcp_session* session = (uv_tcp_session*)handle->data;

		
		char* msg = "";
		sprintf(msg, "client %d close callback.", session->id());
		LOG(msg);

		delete session;
	}

	

	void uv_tcp_server::on_accept(uv_stream_t* server, int status)
	{
		if (status != 0)
		{
			printf(uv_strerror(status));

			return;
		}
		if (server == nullptr)
		{
			return;
		}

		if (server->data == nullptr)
		{
			return;
		}

		uv_tcp_server *tcp = (uv_tcp_server*)server->data;

		if (tcp == nullptr)
		{
			return;
		}

		static int sessionId = 0;

		++sessionId;

		uv_tcp_session* session = new uv_tcp_session(sessionId, tcp);

		session->server(tcp);
	

		int r = uv_tcp_init(tcp->m_loop, session->handle());

		if (r != 0)
		{
		    tcp->error(r);
			delete session;

			return;
		}

		r = uv_accept((uv_stream_t*)&tcp->m_server, (uv_stream_t*)session->handle());
		if (r != 0)
		{
			tcp->error(r);
		
			uv_close((uv_handle_t*)session->handle(), NULL);
			delete session;
			return;
		}

		tcp->m_sessions.insert(std::make_pair(sessionId, session));
		if (tcp->m_connect_callback != nullptr)
		{
			tcp->m_connect_callback(session);
		}

		r = uv_read_start((uv_stream_t*)session->handle(), on_alloc_buffer, on_receive);
		if (r != 0)
		{
			tcp->error(r);
		}
	}

	const uv_tcp_session* uv_tcp_server::session(int sessionId) const
	{
		auto it = m_sessions.find(sessionId);
		if (it != m_sessions.end())
		{
			return it->second;
		}
		return nullptr;
	}
}