#include <Winsock2.h>

#include "hb/net.h"

HbSocket create_game_socket()
{
	// Initialize Winsock2
	WSADATA wsadata;
	assert(WSAStartup(MAKEWORD(2, 2), &wsadata) == 0);

	// Create the socket
	HbSocket sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert(sock != INVALID_SOCKET);

	// Set it to nonblocking
	u_long nonblocking = 1;
	ioctlsocket(sock, FIONBIO, &nonblocking);

	return sock;
}

HbSockaddr create_sockaddr(uint32_t ip, uint16_t port)
{
	HbSockaddr result;
	result.sin_family = AF_INET;
	result.sin_addr.s_addr = htonl(ip);
	result.sin_port = htons(port);

	return result;
}

void send_game_packet(
	HbSocket sock,
	HbSockaddr to,
	ClientId sender_id,
	GamePacketType packet_type,
	void *packet_data,
	size_t data_size)
{
	// Build header
	GamePacketHeader header(packet_type, sender_id);
	GamePacketOut packet(header, packet_data, data_size);
	sendto(
		sock,
		(char*)&packet,
		(int)(sizeof(header) + data_size),
		0,
		(sockaddr*)&to,
		(int)sizeof(to));
}

bool recv_game_packet(HbSocket sock, GamePacket *packet, HbSockaddr *from)
{
	int fromlen = (int)sizeof(*from);

	int n = recvfrom(
		sock,
		(char*)packet,
		sizeof(*packet),
		0,
		(sockaddr*)from,
		&fromlen);

	return n != -1;     // returns false if none available (or error)
}
