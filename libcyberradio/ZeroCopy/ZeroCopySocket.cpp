#include <LibCyberRadio/ZeroCopy/ZeroCopySocket.hpp>
#include <errno.h>

// Constructor / Destructor
/*****************************************************************************/
ZeroCopySocket::ZeroCopySocket(std::string captureFilter) :
    captureFilter(captureFilter),
    blockTimeout(BLOCK_TIMEOUT),
    pollTimeout(BLOCK_TIMEOUT*2),
    currentBlockInd(0),
    blocking(true)
{
    // this can throw a variety of exceptions.  user should be using try/catch
    this->initSocket();
}

ZeroCopySocket::~ZeroCopySocket()
{
    close(this->sockfd); // Socket Cleanup
    munmap(this->ring.map, this->ring.req.tp_block_size * this->ring.req.tp_block_nr); // MMap Cleanup
    free(this->ring.rd); // Free buffer memory
}

// Public
/*****************************************************************************/
struct block_desc * ZeroCopySocket::holdBlock()
{

    // Set a pointer to the current ring buffer block
    struct block_desc *pbd = (struct block_desc *) this->ring.rd[this->currentBlockInd].iov_base;

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
                pbd = (struct block_desc *) this->ring.rd[this->currentBlockInd].iov_base;
                continue;
            }
        }

        // Clear polling struct
        memset(&pfd, 0, sizeof(pfd));
        pfd.fd = this->sockfd;
        pfd.events = POLLIN | POLLERR;
        pfd.revents = 0;

        poll_res = poll(&pfd, 1, this->pollTimeout);

        if ((poll_res == 0)    && (!this->blocking))
        {
            // User requested timeout occured before block timeout
            return NULL;
        }

        if ((poll_res == 1)    && (!this->blocking) && (pbd->h1.num_pkts == 0))
        {
            // Annoying Ubuntu 12 Kernel Case: Poll says there is data, but the block is empty.
            // The only sane thing to do is treat this as a timeout.
            // The next time the user calls holdBlock(), this block get's flushed
            return NULL;
        }
    }
}

void ZeroCopySocket::releaseBlock()
{
    // Create a pointer to the current ring buffer block
    struct block_desc *pbd = (struct block_desc *) this->ring.rd[this->currentBlockInd].iov_base;

    // Release the block to the kernel
    this->flushBlock(pbd);

    // Update ring buffer counter
    this->currentBlockInd = (this->currentBlockInd + 1) % BLOCK_NUM;
}

/*
* Sets time (milliseconds) to poll an empty block before timeout
* -1 sets for infinite polling time
*/
void ZeroCopySocket::setPollTimeout(int pollTimeout)
{
    if (pollTimeout < 0)
    {
        // The user is requesting infinite timeout.
        // This is handled by the loop, not by poll() itself.
        // You could use -1 in the poll() call, but it breaks the holdBlock function.
        pollTimeout = 2*BLOCK_TIMEOUT;
    }
    this->pollTimeout = pollTimeout;
}

/* Removes all blocks that are currently queued */
void ZeroCopySocket::clearBuffer()
{
    int numBlocks = BLOCK_NUM;
    for (int i = 0; i < numBlocks; i++)
    {
        // Aquire pointer to current block
        struct block_desc *pbd = (struct block_desc *) this->ring.rd[this->currentBlockInd].iov_base;

        // If the block is ready to be processed, flush it
        if ((pbd->h1.block_status & TP_STATUS_USER)) {
            this->flushBlock(pbd);
            this->currentBlockInd = (this->currentBlockInd + 1) % BLOCK_NUM;
        } else {
            // We hit the block being currently filled by the kernel. Stop flushing
            break;
        }
    }

}

void ZeroCopySocket::setBlocking(bool shouldBlock)
{
    this->blocking = shouldBlock;
}

// packetStats()

// Private
/*****************************************************************************/
void ZeroCopySocket::initSocket()
{
    int err, v = TPACKET_V3;
    unsigned int i;
    unsigned int blocksiz = BLOCK_SIZE, framesiz = FRAME_SIZE;
    unsigned int blocknum = BLOCK_NUM;

    // Create a raw socket
    this->sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (this->sockfd < 0) {
        perror("");
        throw "Could not create socket";
    }

    // Set filter on socket (Doing so before allocating packet_mmap avoids junk in RX Buffer)
    struct sock_fprog * capture_prog;
    this->generatePacketFilter(this->captureFilter.c_str(), &capture_prog);
    // Attach filter to socket
    if ((setsockopt(this->sockfd, SOL_SOCKET, SO_ATTACH_FILTER, capture_prog, sizeof(struct sock_fprog))) != 0)
    {
        std::cerr << "Invalid TCPDUMP Filter" << std::endl;
        perror("setsockopt[filter]");
        throw "Invalid TCPDUMP Filter";
    }
    // Free filters
    free(capture_prog->filter);
    free(capture_prog);

    // Use TPACKET_V3 for packet version (in hopes of data packing efficiency)
    err = setsockopt(this->sockfd, SOL_PACKET, PACKET_VERSION, &v, sizeof(v));
    if (err < 0) {
        perror("setsockopt");
        throw "Could not create socket";
    }

    // Set the ring buffer size (to get the correct size: total_bytes = blocksiz*blocknum), framesiz doesn't seem to matter
    memset(&this->ring.req, 0, sizeof(this->ring.req));
    this->ring.req.tp_block_size = blocksiz; // Size of a block
    this->ring.req.tp_frame_size = framesiz; // Size of a frame
    this->ring.req.tp_block_nr = blocknum; // Number of blocks
    this->ring.req.tp_frame_nr = (blocksiz * blocknum) / framesiz; // Number of frames
    this->ring.req.tp_retire_blk_tov = BLOCK_TIMEOUT; // Millisecond timeout for block (0 lets kernel decide)
    this->ring.req.tp_feature_req_word = TP_FT_REQ_FILL_RXHASH;

    // Set the socket to recognize the RX Ring patameters
    err = setsockopt(this->sockfd, SOL_PACKET, PACKET_RX_RING, &this->ring.req, sizeof(this->ring.req));
    if (err < 0) {
        perror("setsockopt");
        throw "Could not create socket";
    }

    // Allocate the rx ring and mmap it to the socket (socket will now use this space for its rx buffer)
    this->ring.map = (uint8_t *)(mmap(NULL, this->ring.req.tp_block_size * this->ring.req.tp_block_nr,
             PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, this->sockfd, 0));
    if (this->ring.map == MAP_FAILED) {
        perror("mmap");
        throw "Could not create socket";
    }

    // Create an IO Vector for each block in the ring (this allows a possible readv for reading multiple blocks at a time)
    this->ring.rd = (struct iovec *)(malloc(this->ring.req.tp_block_nr * sizeof(struct iovec)));
    // Assign I/O vectors to ring blocks
    for (i = 0; i < this->ring.req.tp_block_nr; ++i) {
        this->ring.rd[i].iov_base = this->ring.map + (i * this->ring.req.tp_block_size);
        this->ring.rd[i].iov_len = this->ring.req.tp_block_size;
    }
}

// Tells the kernel that the block described by pbd is ok to overwrite
void ZeroCopySocket::flushBlock(struct block_desc *pbd)
{
    pbd->h1.block_status = TP_STATUS_KERNEL;
}

// Creates an assembly packet filter that only allows data specified by filter_string
// This uses the same syntax as tcpdump
void ZeroCopySocket::generatePacketFilter(const char * filter_string, struct sock_fprog ** capture_prog_ptr)
{
    int i;
    unsigned int max_mtu = 9000;
    struct bpf_program fcode;
    int pcap_compile_err = pcap_compile_nopcap(max_mtu, DLT_EN10MB, &fcode, (const char *)filter_string, 1, 0);
    if (pcap_compile_err == -1)
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
