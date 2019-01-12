#include <Winsock.h>

#include "hb/net.h"

// TODO: Can this be merged with net_unix.cpp?

void send_game_packet(
	int sock,
	sockaddr_in to,
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
		sizeof(header) + data_size,
		0,
		(sockaddr*)&to,
		(int)sizeof(to));
}

bool recv_game_packet(int sock, GamePacket *packet, sockaddr_in *from)
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
