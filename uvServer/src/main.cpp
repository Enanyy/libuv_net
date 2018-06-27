#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <functional>
#include <uv.h>
#include "uv_tcp_server.h"
#include "uv_tcp_client.h"
#include "uv_udp_client.h"

using namespace std;
using namespace uv;

#define DEFAULT_PORT 7000

static uv_tcp_server server;
void on_tcp_receive_callback(uv_tcp_session* session, const char* buf, size_t length)
{
	printf("session %d receive %d  %s.\n", session->id(), length, buf);
	uv_tcp_server* server = session->server();
	server->send(session->id(), buf, length);
	const char* a = "aaaaslslsl";
	server->send(session->id(), a, strlen(a));
}

void on_tcp_connection(uv_tcp_session* session)
{
	printf("new connection %d\n", session->id());

	//uv_tcp_server* server = session->server();
	session->set_receive_callback(on_tcp_receive_callback);

}

void test_tcp_server()
{
	server.set_connect_callback(on_tcp_connection);
	server.start_ipv4("0.0.0.0", DEFAULT_PORT);
}

static uv::uv_udp_client udp;

void on_udp_receive_callback(char* data, size_t length, const struct sockaddr* addr, unsigned flags)
{
	printf("udp client receive %d : %s\n", strlen(data), data);

	udp.send(addr, data, strlen(data));
}

void test_udp_server()
{
	
	udp.set_receive_callback(on_udp_receive_callback);
	udp.start_ipv4("0.0.0.0", 7000, true);

}

int main() {

	try
	{
		test_tcp_server();
		
		//test_udp_server();
	}
	catch (const std::exception& e)
	{
		auto message = e.what();
		printf(message);
	}
	return 0;
}
