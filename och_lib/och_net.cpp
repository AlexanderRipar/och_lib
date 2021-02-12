#include "och_net.h"

#include <cstdint>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>

#include "och_fmt.h"

#pragma comment(lib, "Ws2_32.lib")

namespace och
{
	WSAData sock_props;

	uint32_t initialize_sockets() noexcept
	{
		return WSAStartup(MAKEWORD(2, 2), &sock_props);
	}

	void test_net(const char* addr, const char* port)
	{
		addrinfo hints;
		addrinfo* ptr = nullptr;
		addrinfo* result = nullptr;

		ZeroMemory(&hints, sizeof(hints));

		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		
		if (int err = getaddrinfo(addr, port, &hints, &result))
		{
			och::print("getaddrinfo failed: {}\n", err);
			WSACleanup();
			return;
		}

		SOCKET connect_socket = INVALID_SOCKET;

		ptr = result;

		connect_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (connect_socket == INVALID_SOCKET)
		{
			och::print("socket failed: {}\n", WSAGetLastError());
			WSACleanup();
			return;
		}

		if (int err = connect(connect_socket, ptr->ai_addr, (int)ptr->ai_addrlen); err == SOCKET_ERROR)
		{
			closesocket(connect_socket);
			connect_socket = INVALID_SOCKET;
		}

		freeaddrinfo(result);

		if (connect_socket == INVALID_SOCKET)
		{
			och::print("Could not connect\n");
			WSACleanup();
			return;
		}

		const int receive_bytes = 512;

		const char* send_buf = "Test-message";

		char recv_buf[receive_bytes];

		if (int bytes_sent = send(connect_socket, send_buf, (int)strlen(send_buf), 0); bytes_sent == SOCKET_ERROR)
		{
			och::print("send failed: {}\n", bytes_sent);
			WSACleanup();
			return;
		}
		else
			och::print("bytes sent: {}\n", bytes_sent);

		if (int err = shutdown(connect_socket, SD_SEND); err == SOCKET_ERROR)
		{
			och::print("shutdown failed: {}\n", err);
			WSACleanup();
			return;
		}

		for(int bytes_recv = recv(connect_socket, recv_buf, sizeof(recv_buf), 0); bytes_recv > 0; bytes_recv = recv(connect_socket, recv_buf, sizeof(recv_buf), 0))
		{
			if (bytes_recv > 0)
			{
				och::print("\n\nbytes received: {}\n\n", bytes_recv);
				och::print("{}", och::range(recv_buf, sizeof(recv_buf)));
			}
			else if (!bytes_recv)
			{
				och::print("\n\nconnection closed\n\n");
			}
			else
			{
				och::print("\n\nrecv failed: {}\n\n", WSAGetLastError());
				WSACleanup();
				return;
			}
		}

		if (int err = shutdown(connect_socket, SD_SEND); err == SOCKET_ERROR)
		{
			och::print("shutdown failed: {}", WSAGetLastError());
			WSACleanup();
			return;
		}

		closesocket(connect_socket);

		WSACleanup();

		return;
	}
}
