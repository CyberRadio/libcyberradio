#ifndef G3_ZEROCOPYSOCKET_HPP
#define G3_ZEROCOPYSOCKET_HPP

// C
#include <pcap.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/filter.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
//#include <linux/filter.h>

// C++11
#include <iostream>

// Ring Buffer Configuration
#define BLOCK_SIZE 4194304L // Bytes per ring block
#define BLOCK_NUM 256L // Number of blocks in the ring
#define FRAME_SIZE 2048L // Size of a frame (other than having to be a power of 2, changing this appears to have no effect)
#define BLOCK_TIMEOUT 250L // Millisecond timeout before block expires (use 0 to let kernel decide)

struct block_desc {
	uint32_t version;
	uint32_t offset_to_priv;
	struct tpacket_hdr_v1 h1;
};

struct ring {
	struct iovec *rd;
	uint8_t *map;
	struct tpacket_req3 req;
};

class ZeroCopySocket
{

private:
    // Disable Copying
    ZeroCopySocket( const ZeroCopySocket& other ); // non construction-copyable
    ZeroCopySocket& operator=( const ZeroCopySocket& ); // non copyable

    // Variables
    std::string captureFilter;  // TCPDUMP / Berkely Packet Filter for packet capture
    int blockTimeout;
    int pollTimeout;
    int sockfd; // Socket File Descriptor
    struct ring ring;
    unsigned int currentBlockInd;
    bool blocking;

    // Functions
    void generatePacketFilter(const char * filter_string, struct sock_fprog ** capture_prog_ptr);
    void initSocket();
    void flushBlock(struct block_desc *pbd);


public:
    // Constructor/Destructor
    ZeroCopySocket(std::string captureFilter);
    ~ZeroCopySocket();

    // Functions
    struct block_desc * holdBlock(); // waits until the next block in the ring is available to the user
    void releaseBlock(); // passes block access to kernel and increments to the next block
    void setPollTimeout(int);
    void clearBuffer();
    void setBlocking(bool);

};

#endif
