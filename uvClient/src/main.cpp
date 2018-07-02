#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include "uv_tcp_client.h"
#include "uv_udp_client.h"

#ifdef _MSC_VER

#pragma comment(lib,"net.lib")
#endif // _MSC_VER

#define  DEFAULT_PORT 7000

uv::uv_tcp_client client;

void on_tcp_connect_callback(int status)
{
	if (status == 0)
	{
		printf("on connect callback.\n");
		char* msg = "Hello World";
		client.send(msg, strlen(msg));
	}
	else
	{

	}
}

void on_tcp_receive_callback(char* data, std::size_t length)
{
	printf("receive:%s\n", data);
}

void test_tcp_client()
{
	client.set_connect_callback(on_tcp_connect_callback);
	client.set_receive_callback(on_tcp_receive_callback);

	client.start("127.0.0.1", DEFAULT_PORT);
}
void on_udp_receive_callback(char* data, size_t length, const struct sockaddr* addr, unsigned flags)
{
	printf("udp client receive %d : %s\n",length, data);

	
}

void on_udp_start_callback(uv::uv_udp_client* client)
{
	printf("udp start callback.\n");
	char* msg = "Hello udp client.";
	client->send_ipv4("127.0.0.1", 7000, msg, strlen(msg));
}

void test_udp_client()
{
	uv::uv_udp_client udp;
	udp.set_receive_callback(on_udp_receive_callback);
	udp.set_start_callback(on_udp_start_callback);
	udp.start_ipv4("127.0.0.1", 8000);
}

int main()
{
	

	test_tcp_client();

	//test_udp_client();
	return  0;
}