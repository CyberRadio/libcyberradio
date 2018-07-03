/*
 * Name: ZeroCopyReceiver
 * Date: 3/29/2017
 * Author: Joseph Martin
 * Description: Simplest example of ZeroCopySocket application.  This should be a guideline for usage
 *    Please run this application as root, or with cap_net_raw permission.
 */
#include <sstream>
#include <LibCyberRadio/ZeroCopy/ZeroCopySocket.hpp>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* Example BPF Functions */
std::string captureDestPort(unsigned short destPort);
std::string captureSourceIP(std::string srcIP);
std::string captureDestIP(std::string dstIP);
std::string captureDevice(std::string ethDevice);

/* Struct Defines For Reference

// ZC Custom Structs
struct block_desc {
    uint32_t version; // This is not actively used. Don't use.
    uint32_t offset_to_priv; // This is not actively used. Don't use.
    struct tpacket_hdr_v1 h1; // Pointer information about this block
};

struct ring {
    struct iovec *rd; // Array of blocks. rd[0] is block0, rd[1] is block 1
    uint8_t *map; // Pointer to the beginning of the mmaped segment that holds all of the ring buffer blocks.  Direct access (map[i]) is discouraged
    struct tpacket_req3 req; // Pointer to description of ring buffer and block info
};

 */

int main()
{
    /* Create a Berkely Packet Filter String. Great examples: https://biot.com/capstats/bpf.html */
    std::string bpfString = captureDevice("eth2") + " and " + captureDestPort(34567);

    std::cout << "Your capture filter: " << bpfString << std::endl;

    /* Instantiate ZeroCopySocket with BPF string */
    ZeroCopySocket *recvSock = new ZeroCopySocket(bpfString);
    //recvSock->setBlocking(false);
    //recvSock->setPollTimeout(1000);
    /*
         This socket is backed by a ring buffer.  The usage is essentially:
         1) Obtain a block
         2) Loop over all packets in a block
         3) Release the block
     */
    int captureNum = 100;
    for (int i = 0; i < captureNum; i++)
    {
        int numPktsReceived = 0;
        struct block_desc *pbd = recvSock->holdBlock();
        std::cout << "ret" << std::endl;
        if (pbd == NULL)
        {
            // The call to holdBlock() timed out.
            usleep(1000); // Wait a little bit, data may not be coming in
            continue;
        }

        /* Init a packet header pointer, and find out how many packets are in this block */
        int num_pkts = pbd->h1.num_pkts;
        struct tpacket3_hdr * ppd = (struct tpacket3_hdr *) ((uint8_t *) pbd + pbd->h1.offset_to_first_pkt);
        uint8_t *startOfPacket;

        /*
            If we are going to be sending a packet to the same machine, we need to avoid double receiving the packet.
            As far as I can tell, the only way to do this is my filtering packets with the PACKET_OUTGOING flag.
            Here, we set up a variable to do so.  TLDR: This was a total pain to figure out, just cut and paste it if you
            need to avoid double counting local TX Packets
         */
        struct sockaddr_ll *src_addr;
        unsigned int offsetToSockAddr = TPACKET_ALIGN(sizeof(struct tpacket3_hdr));
        bool packetIsOutgoing;

        for (int j = 0; j < num_pkts; j++) {

            /* Access the packet.  'startOfPacket' should be the start of the ethernet frame */
            startOfPacket = (uint8_t *) ppd + ppd->tp_mac;

            /* Is this packet outgoing?  If so, filter it */
            struct sockaddr_ll *src_addr = (struct sockaddr_ll *)((uint8_t*)ppd + offsetToSockAddr);
            packetIsOutgoing = (((unsigned int) src_addr->sll_pkttype) == PACKET_OUTGOING);
            if (!packetIsOutgoing)
            {
                numPktsReceived++;
                /*
                 *    Do something with the packet
                 */
                // In my case, this prints the P in PLRV from NDR308
                std::cout << (startOfPacket + 42)[0] << std::endl;
                //std::cout << ppd->tp_snaplen << std::endl; // Length of packet

            }

            /* Go to the next packet */
            ppd = (struct tpacket3_hdr *) ((uint8_t *) ppd +
                    ppd->tp_next_offset);
        }

        /* Release the block. Note: Release does not need to be called if the block was NULL */
        recvSock->releaseBlock();
        std::cout << "Counted " << numPktsReceived << " in block." << std::endl;
    }
    delete recvSock;
    std::cout << "Exiting" << std::endl;
}

// Utility Functions
std::string captureDestPort(unsigned short destPort)
{
    std::stringstream ss;
    ss << "dst port " << destPort;
    return ss.str();
}

std::string captureSourceIP(std::string srcIP)
{
    std::stringstream ss;
    ss << "src ip " << srcIP;
    return ss.str();
}

std::string captureDestIP(std::string dstIP)
{
    std::stringstream ss;
    ss << "dst ip " << dstIP;
    return ss.str();
}

std::string captureDevice(std::string ethDevice)
{
    // BPF does not natively handle devices, but we can lookup the IP of device and use that
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) 
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    // Loop through all addresses and see if any in the list match input 'ethDevice'
    // Note: If you use this, I would reccommend adding some error checking for getnameinfo()
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
    {
        if (ifa->ifa_addr == NULL)
            continue;  

        s = getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if((strcmp(ifa->ifa_name, ethDevice.c_str())==0)&&(ifa->ifa_addr->sa_family==AF_INET))
        {
            break;
        }

    }
    std::string dstIP = std::string(host);
    freeifaddrs(ifaddr);

    std::stringstream ss;
    ss << "dst " << dstIP;
    return ss.str();
}
