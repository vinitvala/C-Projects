#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#include "wolfpack.h"

int counter=0;

struct header_fields {
    unsigned char src_addr[5], dst_addr[5];
    unsigned char src_port, dst_port;
    unsigned char frag_offset[3];
    unsigned char flags[2];
    unsigned char total_len[3];
    unsigned char checksum[4];
};

struct header_fields parse_packet(const unsigned char *packet)
{
	struct header_fields h;

    // Source Address (5 bytes)
    for (int i = 0; i < 5; i++) {
        h.src_addr[i] = packet[i];
    }

    // Destination Address (5 bytes)
    for (int i = 0; i < 5; i++) {
        h.dst_addr[i] = packet[5 + i];
    }

    // Source Port (1 byte)
    h.src_port = packet[10];

    // Destination Port (1 byte)
    h.dst_port = packet[11];

    // Fragment Offset (3 bytes)
    for (int i = 0; i < 3; i++) {
        h.frag_offset[i] = packet[12 + i];
    }

    // Flags (2 bytes)
    for (int i = 0; i < 2; i++) {
        h.flags[i] = packet[15 + i];
    }

    // Total Length (3 bytes)
    for (int i = 0; i < 3; i++) {
        h.total_len[i] = packet[17 + i];
    }

    // Checksum (4 bytes)
    for (int i = 0; i < 4; i++) {
        h.checksum[i] = packet[20 + i];
    }

	return h;
}

void print_packet_sf(const unsigned char *packet) {

const unsigned char WOLF_PACKET_HEADER_SIZE = 24;

// Parse the header fields from the packet buffer
	struct header_fields h = parse_packet(packet);

    // Print header fields in the specified order
    printf("%02x%02x%02x%02x%02x\n", h.src_addr[0], h.src_addr[1], h.src_addr[2], h.src_addr[3], h.src_addr[4]);
    printf("%02x%02x%02x%02x%02x\n", h.dst_addr[0], h.dst_addr[1], h.dst_addr[2], h.dst_addr[3], h.dst_addr[4]);
    printf("%02x\n", h.src_port);
    printf("%02x\n", h.dst_port);
    printf("%02x%02x%02x\n", h.frag_offset[0], h.frag_offset[1], h.frag_offset[2]);
    printf("%02x%02x\n", h.flags[0], h.flags[1]);
    printf("%02x%02x%02x\n", h.total_len[0], h.total_len[1], h.total_len[2]);
    printf("%02x%02x%02x%02x\n", h.checksum[0], h.checksum[1], h.checksum[2], h.checksum[3]);

	unsigned int len = ((unsigned int)h.total_len[0] << 16) + ((unsigned int)h.total_len[1] << 8) + h.total_len[2];

    // Print payload as ASCII string
    printf("%.*s\n", (int)(len - WOLF_PACKET_HEADER_SIZE), packet + WOLF_PACKET_HEADER_SIZE);
}

unsigned int packetize_sf(const char *message, unsigned char *packets[], unsigned int packets_len, unsigned int max_payload,
    unsigned long src_addr, unsigned long dest_addr, unsigned short flags) {

	unsigned int num_payloads = 0;
	unsigned int message_length = strlen(message);

	// While we haven't filed every packet and there is still message to be packetized
	while(num_payloads != packets_len && message_length != 0)
	{
		int size;
		if(max_payload < message_length)
			size = max_payload;
		else
			size = message_length;
		
		char *packet = malloc(size + 24);
		for(int i = 0; i < 5; i++)
			packet[i] = (char)(src_addr >> ((5 - i - 1) * 8));

		for(int i = 0; i < 5; i++)
			packet[i + 5] = (char)(dest_addr >> ((5 - i - 1) * 8));

		packet[10] = 32;
		packet[11] = 64;

		unsigned int frag_offset = num_payloads * max_payload;
		for(int i = 0; i < 3; i++)
			packet[i + 12] = (char)(frag_offset >> ((3 - i - 1) * 8));

		for(int i = 0; i < 2; i++)
			packet[i + 15] = (char)(flags >> ((2 - i - 1) * 8));

		unsigned int total_length = 24 + (unsigned int)size;

		for(int i = 0; i < 3; i++)
			packet[i + 17] = (char)(total_length >> ((3 - i - 1) * 8));

		unsigned int checksum = checksum_sf(packet);
		for(int i = 0; i < 4; i++)
			packet[i + 20] = (char)(checksum >> ((4 - i - 1) * 8));

		for(int i = 0; i < size; i++)
			packet[i + 24] = message[frag_offset + i];

		message_length -= size;
		packets[num_payloads] = packet;
		num_payloads++;
	}

	return num_payloads;
}

unsigned int checksum_sf(const unsigned char *packet) {

	struct header_fields h = parse_packet(packet);

	unsigned long c = 0;
	for(int i = 5 - 1; i >= 0; i--)
		c += ((unsigned long)h.src_addr[5 - i - 1] << (i * 8));

	for(int i = 5 - 1; i >= 0; i--)
		c += ((unsigned long)h.dst_addr[5 - i - 1] << (i * 8));

	c += (unsigned long)h.src_port;
	c += (unsigned long)h.dst_port;

	for(int i = 3 - 1; i >= 0; i--)
		c += ((unsigned long)h.frag_offset[3 - i - 1] << (i * 8));

	for(int i = 2 - 1; i >= 0; i--)
		c += ((unsigned long)h.flags[2 - i - 1] << (i * 8));

	for(int i = 3 - 1; i >= 0; i--)
		c += ((unsigned long)h.total_len[3 - i - 1] << (i * 8));
	
	return (unsigned int)(c % 0xFFFFFFFF);
}

unsigned int reconstruct_sf(unsigned char *packets[], unsigned int packets_len, char *message, unsigned int message_len) {
	int p = 0;
	int max_index = -1;

	for(int i = 0; i < packets_len; i++)
	{
		struct header_fields fields = parse_packet(packets[i]);
		unsigned int checksum = 0;
		for(int j = 0; j < 4; j++)
			checksum += (unsigned int)fields.checksum[j] << ((3 - j) * 8);

		if(checksum == checksum_sf(packets[i]))
		{
			int frag_offset = 0;
			for(int j = 0; j < 3; j++)
				frag_offset += (int)fields.frag_offset[j] << ((2 - j) * 8);

			if(frag_offset < message_len - 1)
			{
				int len = 0;
				for(int j = 0; j < 3; j++)
					len += (int)fields.total_len[j] << ((2 - j) * 8);

				for(int j = 0; j < len - 24; j++)
				{
					if(j + frag_offset >= message_len - 1)
						break;

					message[j + frag_offset] = packets[i][j + 24];
					if(max_index < j + frag_offset + 1)
						max_index = j + frag_offset + 1;
				}
				
				p++;
			}
		}
	}

	if(max_index != -1)
		message[max_index] = '\0';
	
    return p;
}
