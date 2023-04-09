//GNU AFFERO 3.0 Useop GIm April 10. 2023

#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <sys/types.h>

#ifdef _WIN32
	#include <WS2tcpip.h>
	#include <winsock2.h>
	#pragma comment(lib, "Ws2_32.lib")
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
#endif

#define BUFFER_SIZE 1024

namespace TCP
{
#ifdef _WIN32
	class Client
	{
		WSADATA wsaData;
		SOCKET sockfd;
		std::string m_ip_addr;
		int m_port = 8888;
		int m_timeout_ms = 1000;

		bool TCPStart()
		{
			int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (result != NO_ERROR){
				fprintf(stderr, "Error: WSAStartup failed with error: %s\n", std::to_string(result).c_str());
				return true;
			}
			return false;				
		}
	public:
		bool connecting() 
		{
			if (TCPStart()) return true;

			sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (sockfd == INVALID_SOCKET) {
				fprintf(stderr, "Error: Could not create socket. Error: %s\n", std::to_string(WSAGetLastError()).c_str());
				return true;
			}

			sockaddr_in server_addr;
			ZeroMemory(&server_addr, sizeof(server_addr));
			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(m_port);
			inet_pton(AF_INET, m_ip_addr.c_str(), &server_addr.sin_addr);

			if (m_timeout_ms > 0) {
				unsigned long mode = 1;
				ioctlsocket(sockfd, FIONBIO, &mode); // Set socket to non-blocking mode

				int connect_result = connect(sockfd, (SOCKADDR*)&server_addr, sizeof(server_addr));
				if (connect_result == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) {
					timeval tv;
					tv.tv_sec = m_timeout_ms / 1000;
					tv.tv_usec = (m_timeout_ms % 1000) * 1000;

					fd_set writefds;
					FD_ZERO(&writefds);
					FD_SET(sockfd, &writefds);

					int select_result = select(0, NULL, &writefds, NULL, &tv);

					if (select_result == SOCKET_ERROR || select_result == 0) {
						// Socket error or select timeout
						fprintf(stderr, "Error: Connection timeout\n");
						return true;
					}
				}
				mode = 0;
				ioctlsocket(sockfd, FIONBIO, &mode); // Set socket back to blocking mode
			}
			else {
				if (connect(sockfd, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
					fprintf(stderr, "Error: Could not connect to server. Error: %s\n", std::to_string(WSAGetLastError()).c_str());
					return true;
				}
			}
			fprintf(stderr, "Connecting is success\n");
			return false;
		}

		void sending(const char* message)
		{
			if (m_timeout_ms > 0) {
				DWORD timeout = m_timeout_ms;
				setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
			}
			send(sockfd, message, static_cast<int>(strlen(message)), 0);
		}

		std::string recving()
		{
			char buffer[BUFFER_SIZE];

			if (m_timeout_ms > 0) {
				DWORD timeout = m_timeout_ms;
				setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
			}

			int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);

			if (bytes_received == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT) {

				fprintf(stderr, "Error: Receive timeout\n");
			}
			else if (bytes_received > 0) {
				buffer[bytes_received] = '\0';
				return std::string(buffer);
			}
			return "";
		}

		Client(const char* ip_addr, int port, int timeout_ms = 1000) : 
			m_ip_addr{ ip_addr }, m_port{ port }, m_timeout_ms{ timeout_ms } {}

		~Client() {
			if (sockfd != INVALID_SOCKET)
				closesocket(sockfd);
			WSACleanup();
		}
		
	};
#else
	class Client
	{
		int sockfd;
		std::string m_ip_addr;
		int m_port = 8888;
		int m_timeout_ms = 1000;

	public:
		bool connecting()
		{
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if (sockfd == -1) {
				fprintf(stderr, "Error: Could not create socket. Error: %s\n", strerror(errno));
				return true;
			}

			sockaddr_in server_addr;
			ZeroMemory(&server_addr, sizeof(server_addr));
			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(m_port);
			inet_pton(AF_INET, m_ip_addr.c_str(), &server_addr.sin_addr);

			if (m_timeout_ms > 0) {
				int flags = fcntl(sockfd, F_GETFL, 0);
				fcntl(sockfd, F_SETFL, flags | O_NONBLOCK); // Set socket to non-blocking mode

				int connect_result = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
				if (connect_result == -1 && errno == EINPROGRESS) {
					fd_set writefds;
					FD_ZERO(&writefds);
					FD_SET(sockfd, &writefds);

					timeval tv;
					tv.tv_sec = m_timeout_ms / 1000;
					tv.tv_usec = (m_timeout_ms % 1000) * 1000;

					int select_result = select(sockfd + 1, NULL, &writefds, NULL, &tv);

					if (select_result == -1 || select_result == 0) {
						// Socket error or select timeout
						fprintf(stderr, "Error: Connection timeout\n");
						return true;
					}
				}
				fcntl(sockfd, F_SETFL, flags); // Set socket back to blocking mode
			}
			else {
				if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
					fprintf(stderr, "Error: Could not connect to server. Error: %s\n", strerror(errno));
					return true;
				}
			}

			fprintf(stderr, "Connecting is success\n");
			return false;
		}

		void sending(const char* message)
		{
			if (m_timeout_ms > 0) {
				timeval timeout;
				timeout.tv_sec = m_timeout_ms / 1000;
				timeout.tv_usec = (m_timeout_ms % 1000) * 1000;
				setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
			}

			send(sockfd, message, static_cast<int>(strlen(message)), 0);
		}
		std::string recving()
		{
			char buffer[BUFFER_SIZE];
			memset(buffer, 0, sizeof(buffer));

			if (m_timeout_ms > 0) {
				timeval timeout;
				timeout.tv_sec = m_timeout_ms / 1000;
				timeout.tv_usec = (m_timeout_ms % 1000) * 1000;
				setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
			}

			int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);

			if (bytes_received == -1 && errno == ETIMEDOUT) {
				fprintf(stderr, "Error: Receive timeout\n");
			}
			else if (bytes_received > 0) {
				buffer[bytes_received] = '\0';
				return std::string(buffer);
			}
			return "";
		}

		Client(const char* ip_addr, int port, int timeout_ms = 1000) :
			m_ip_addr{ ip_addr }, m_port{ port }, m_timeout_ms{ timeout_ms } {}

		~Client() {
			if (sockfd != -1)
				close(sockfd);
		}
	};
#endif
}