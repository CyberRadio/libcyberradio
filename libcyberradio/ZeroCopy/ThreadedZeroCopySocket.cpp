/**
 * \file ThreadedZeroCopySocket.cpp
 * \brief Implements a thread-aware version of the
 *    zero-copy socket.
 * \author JM
 * \author DA
 * \copyright Copyright (c) 2018 CyberRadio Solutions, Inc.
 *    All rights reserved.
 */

#include "LibCyberRadio/ZeroCopy/ThreadedZeroCopySocket.hpp"
#include <pcap.h>
#include <linux/filter.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <exception>


std::mutex ThreadedZeroCopySocket::sharedMtx;


ThreadedZeroCopySocket::ThreadedZeroCopySocket(
        const std::string& captureFilter,
        bool blocking,
        unsigned long blockSize,
        unsigned long numBlocks,
        unsigned long frameSize,
        unsigned long blockTimeout,
        unsigned long pollTimeout,
        bool debug,
        const std::string& debugName) :
    LibCyberRadio::Debuggable(debug, debugName),
    _captureFilter(captureFilter),
    _blocking(blocking),
    _blockSize(blockSize),
    _numBlocks(numBlocks),
    _frameSize(frameSize),
    _blockTimeout(blockTimeout),
    _pollTimeout(pollTimeout),
    _sockfd(-1),
    _currentBlockInd(0),
    _lastError("")
{
}

ThreadedZeroCopySocket::~ThreadedZeroCopySocket()
{
    this->doCleanup();
}

bool ThreadedZeroCopySocket::initialize()
{
    bool ret = true;
    int err, v = TPACKET_V3;
    unsigned int i;
    struct sock_fprog* capture_prog = NULL;
    std::ostringstream oss;
    try
    {
        // The most likely place where this will fail is in compiling the packet filter,
        // so deal with this first.
        // -- The shared mutex calls here make sure that only one object of this class is
        //    trying to compile a packet filter at any given time.
        sharedMtx.lock();
        this->generatePacketFilter(this->_captureFilter.c_str(), &capture_prog);
        sharedMtx.unlock();
        if ( capture_prog == NULL )
        {
            oss << "Invalid TCPDUMP Filter: " << this->_lastError;
            throw std::runtime_error(oss.str());
        }
        // Create a raw socket
        this->_sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (this->_sockfd < 0)
        {
            oss << "Could not create socket: " << strerror(errno);
            throw std::runtime_error(oss.str());
        }
        // Attach filter to socket (Doing so before allocating packet_mmap avoids junk in RX Buffer)
        if ((setsockopt(this->_sockfd, SOL_SOCKET, SO_ATTACH_FILTER, capture_prog, sizeof(struct sock_fprog))) != 0)
        {
            oss << "Could not attach filter: " << strerror(errno);
            throw std::runtime_error(oss.str());
        }
        // Use TPACKET_V3 for packet version (in hopes of data packing efficiency)
        err = setsockopt(this->_sockfd, SOL_PACKET, PACKET_VERSION, &v, sizeof(v));
        if (err < 0)
        {
            oss << "Could not set packet version: " << strerror(errno);
            throw std::runtime_error(oss.str());
        }
        // Set the ring buffer size (to get the correct size: total_bytes = blocksiz*blocknum), framesiz doesn't seem to matter
        memset(&this->_ring.req, 0, sizeof(this->_ring.req));
        this->_ring.req.tp_block_size = this->_blockSize; // Size of a block
        this->_ring.req.tp_frame_size = this->_frameSize; // Size of a frame
        this->_ring.req.tp_block_nr = this->_numBlocks; // Number of blocks
        this->_ring.req.tp_frame_nr = (this->_blockSize * this->_numBlocks) / this->_frameSize; // Number of frames
        this->_ring.req.tp_retire_blk_tov = this->_blockTimeout; // Millisecond timeout for block (0 lets kernel decide)
        this->_ring.req.tp_feature_req_word = TP_FT_REQ_FILL_RXHASH;
        // Set the socket to recognize the RX Ring patameters
        err = setsockopt(this->_sockfd, SOL_PACKET, PACKET_RX_RING, &this->_ring.req, sizeof(this->_ring.req));
        if (err < 0)
        {
            oss << "Could not set ring buffer: " << strerror(errno);
            throw std::runtime_error(oss.str());
        }
        // Allocate the rx ring and mmap it to the socket (socket will now use this space for its rx buffer)
        this->_ring.map = (uint8_t *)(mmap(NULL, this->_ring.req.tp_block_size * this->_ring.req.tp_block_nr,
                 PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, this->_sockfd, 0));
        if (this->_ring.map == MAP_FAILED)
        {
            oss << "Could not set memory mapping: " << strerror(errno);
            throw std::runtime_error(oss.str());
        }
        // Create an IO Vector for each block in the ring (this allows a possible readv for reading multiple blocks at a time)
        this->_ring.rd = (struct iovec *)(malloc(this->_ring.req.tp_block_nr * sizeof(struct iovec)));
        // Assign I/O vectors to ring blocks
        for (i = 0; i < this->_ring.req.tp_block_nr; ++i) {
            this->_ring.rd[i].iov_base = this->_ring.map + (i * this->_ring.req.tp_block_size);
            this->_ring.rd[i].iov_len = this->_ring.req.tp_block_size;
        }
    }
    catch( std::exception& ex )
    {
        this->_lastError = ex.what();
        ret = false;
    }
    // Free filters
    if ( capture_prog != NULL )
    {
        if ( capture_prog->filter != NULL )
            free(capture_prog->filter);
        free(capture_prog);
    }
    return ret;
}

struct block_desc* ThreadedZeroCopySocket::holdBlock()
{
    // Set a pointer to the current ring buffer block
    struct block_desc *pbd = (struct block_desc *) this->_ring.rd[this->_currentBlockInd].iov_base;
    // Setup a poll struct (to efficiently wait for the block to be ready)
    struct pollfd pfd;
    int poll_res;
    // Loop polling the socket until poll returns.
    // This appears fixed in later kernels
    while (true)
    {
        // Can the current block be processed?
        if ((pbd->h1.block_status & TP_STATUS_USER))
        {
            // Block is ready for user space
            if (pbd->h1.num_pkts != 0)
            {
                // Block has data
                return pbd;
            }
            else
            {
                // Block is empty, don't return it
                this->releaseBlock();
                pbd = (struct block_desc *) this->_ring.rd[this->_currentBlockInd].iov_base;
                continue;
            }
        }
        // Clear polling struct
        memset(&pfd, 0, sizeof(pfd));
        pfd.fd = this->_sockfd;
        pfd.events = POLLIN | POLLERR;
        pfd.revents = 0;
        // Poll
        poll_res = poll(&pfd, 1, (int)this->_pollTimeout);
        // Handle polling result
        if ((poll_res == 0) && (!this->_blocking))
        {
            // User requested timeout occured before block timeout
            return NULL;
        }
        if ((poll_res == 1) && (!this->_blocking) && (pbd->h1.num_pkts == 0))
        {
            // Annoying Ubuntu 12 Kernel Case: Poll says there is data, but the block is empty.
            // The only sane thing to do is treat this as a timeout.
            // The next time the user calls holdBlock(), this block get's flushed
            return NULL;
        }
    }
}

void ThreadedZeroCopySocket::releaseBlock()
{
    // Create a pointer to the current ring buffer block
    struct block_desc *pbd = (struct block_desc *) this->_ring.rd[this->_currentBlockInd].iov_base;
    // Release the block to the kernel
    this->flushBlock(pbd);
    // Update ring buffer counter
    this->_currentBlockInd = (this->_currentBlockInd + 1) % this->_numBlocks;
}

void ThreadedZeroCopySocket::setPollTimeout(long pollTimeout)
{
    if (pollTimeout < 0)
    {
        // The user is requesting infinite timeout.
        // This is handled by the loop, not by poll() itself.
        // You could use -1 in the poll() call, but it breaks the holdBlock function.
        this->_pollTimeout = 2 * this->_blockTimeout;
    }
    else
        this->_pollTimeout = pollTimeout;
}

void ThreadedZeroCopySocket::setBlocking(bool blocking)
{
    this->_blocking = blocking;
}

void ThreadedZeroCopySocket::clearBuffer()
{
    for (unsigned long i = 0; i < this->_numBlocks; i++)
    {
        // Aquire pointer to current block
        struct block_desc *pbd = (struct block_desc *) this->_ring.rd[this->_currentBlockInd].iov_base;
        // If the block is ready to be processed, flush it
        if ((pbd->h1.block_status & TP_STATUS_USER))
        {
            this->flushBlock(pbd);
            this->_currentBlockInd = (this->_currentBlockInd + 1) % this->_numBlocks;
        }
        else
        {
            // We hit the block being currently filled by the kernel. Stop flushing
            break;
        }
    }
}

const std::string& ThreadedZeroCopySocket::getLastError() const
{
    return this->_lastError;
}

void ThreadedZeroCopySocket::generatePacketFilter(
        const char* filter_string,
        struct sock_fprog** capture_prog_ptr)
{
    int i = -1;
    unsigned int max_mtu = 9000;
    struct bpf_program fcode;
//    int pcap_compile_err = pcap_compile_nopcap(max_mtu, DLT_EN10MB, &fcode, (const char *)filter_string, 1, 0);
//    if (pcap_compile_err == -1)
//    {
//        *capture_prog_ptr = NULL;
//        return;
//    }
    this->_lastError = "";
    pcap_t* dummyHdr = pcap_open_dead(DLT_EN10MB, max_mtu);
    if ( dummyHdr != NULL )
    {
        i = pcap_compile(dummyHdr, &fcode, filter_string, 1, 0);
        if ( i == -1 )
        {
            this->_lastError = pcap_geterr(dummyHdr);
        }
        pcap_close(dummyHdr);
    }
    else
    {
        this->_lastError = pcap_geterr(dummyHdr);
    }
    if (i == -1)
    {
        *capture_prog_ptr = NULL;
        return;
    }
    // Create the sock_filter array from the pcap instructions
    int filter_len = fcode.bf_len;
    struct sock_filter * filter_arr = (struct sock_filter *)(malloc(sizeof(struct sock_filter) * fcode.bf_len));
    memset(filter_arr, 0, sizeof(struct sock_filter));
    struct sock_fprog * capture_prog = (struct sock_fprog *)(malloc(sizeof(struct sock_fprog)));
    memset(capture_prog, 0, sizeof(struct sock_fprog));
    // Copy over instructions
    struct bpf_insn *insn = fcode.bf_insns;
    for (i = 0; i < filter_len; ++insn, ++i)
    {
        // Uncomment to see the filter
        //printf("{ 0x%x, %d, %d, 0x%08x },\n", insn->code, insn->jt, insn->jf, insn->k);

        // Populate Filter (Can this me done via memcpy?  The structs look identical)
        filter_arr[i].code = insn->code;
        filter_arr[i].jt = insn->jt;
        filter_arr[i].jf = insn->jf;
        filter_arr[i].k = insn->k;
    }
    // Assign filter to out capture prog
    capture_prog->len = filter_len;
    capture_prog->filter = filter_arr;
    // Return filter to user (USER MUST CALL free(capture_prog_ptr.filter  and free(capture_prog) )
    *capture_prog_ptr = capture_prog;
}

void ThreadedZeroCopySocket::flushBlock(struct block_desc* pbd)
{
    pbd->h1.block_status = TP_STATUS_KERNEL;
}

void ThreadedZeroCopySocket::doCleanup()
{
    // Socket Cleanup
    if ( this->_sockfd != -1 )
    {
        close(this->_sockfd);
        this->_sockfd = -1;
    }
    // Ring Buffer Cleanup
    // -- MMap
    if ( this->_ring.map != NULL )
        munmap(this->_ring.map, this->_ring.req.tp_block_size * this->_ring.req.tp_block_nr);
    // -- Free buffer memory
    if ( this->_ring.rd != NULL )
        free(this->_ring.rd);
}

