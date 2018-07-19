/**
 * \file ThreadedZeroCopySocket.hpp
 * \brief Defines a thread-aware version of the
 *    zero-copy socket.
 * \author JM
 * \author DA
 * \copyright Copyright (c) 2018 CyberRadio Solutions, Inc.
 *    All rights reserved.
 *
 * \note Requires C++11 compiler support.
 */

#ifndef LIBCYBERRADIO_THREADEDZEROCOPYSOCKET_HPP
#define LIBCYBERRADIO_THREADEDZEROCOPYSOCKET_HPP

#include "LibCyberRadio/Common/Debuggable.h"
#include <linux/if_packet.h>
#include <sys/uio.h>
#include <cstdint>
#include <mutex>
#include <string>
#include <cstdint>


/**
 * \brief Block descriptor.
 */
struct block_desc
{
    uint32_t version;
    uint32_t offset_to_priv;
    struct tpacket_hdr_v1 h1;
};

/**
 * \internal
 * \brief Ring buffer.
 */
struct ring
{
    struct iovec *rd;
    uint8_t *map;
    struct tpacket_req3 req;
};


/**
 * \brief Thread-aware zero-copy socket.
 *
 * Zero-copy sockets use the Berkeley Packet Filter (BPF) system.
 *
 * ThreadedZeroCopySocket objects are neither copyable nor
 * assignable.
 *
 */
class ThreadedZeroCopySocket : public LibCyberRadio::Debuggable
{
    public:
        /**
         * \brief Constructs a ThreadedZeroCopySocket object.
         * \param captureFilter BPF capture filter string.
         * \param blocking Whether the socket is in blocking mode.
         * \param blockSize Size of each block, in bytes
         * \param numBlocks Number of blocks in the socket's ring buffer.
         * \param frameSize Size of a frame.  Must be a power of 2.
         * \param blockTimeout Timeout (in milliseconds) before a block
         *     expires.  Setting this to 0 uses the kernel's default.
         * \param pollTimeout Timout (in milliseconds) before a polling
         *     attempt times out.
         * \param debug Whether this object produces debug output.
         * \param debugName Name of this object for debugging purposes.
         */
        ThreadedZeroCopySocket(
                const std::string& captureFilter,
                bool blocking = false,
                unsigned long blockSize = 4194304L,
                unsigned long numBlocks = 256L,
                unsigned long frameSize = 2048L,
                unsigned long blockTimeout = 250L,
                unsigned long pollTimeout = 500L,
                bool debug = false,
                const std::string& debugName = "ThreadedZeroCopySocket"
            );
        /**
         * \brief Destroys a ThreadedZeroCopySocket object.
         */
        virtual ~ThreadedZeroCopySocket();
        // Copy constructor is explicitly disabled
        ThreadedZeroCopySocket(const ThreadedZeroCopySocket&) = delete;
        // Assignment operator is explicitly disabled
        ThreadedZeroCopySocket& operator=(const ThreadedZeroCopySocket&) = delete;
        /**
         * \brief Initializes the socket.
         * \returns True if successful, false otherwise.
         */
        virtual bool initialize();
        /**
         * \brief Gets the next available block from the socket's ring buffer.
         * \note If the socket is in blocking mode, this call will block until
         *    data is available.
         * \returns A pointer to the block if data is available, NULL if data is
         *    not availble or if a timeout occurs.
         */
        virtual struct block_desc* holdBlock();
        /**
         * \brief Passes block access to kernel and increments to the next block.
         * \returns A pointer to the block if available, NULL if not.
         */
        virtual void releaseBlock();
        /**
         * \brief Sets the polling timeout for retrieving blocks.
         * \param pollTimeout Timeout (in milliseconds) before polling
         *     expires.  If this is negative, use twice the block
         *     timeout.
         */
        virtual void setPollTimeout(long pollTimeout);
        /**
         * \brief Sets whether the socket is in blocking mode.
         * \param blocking Whether the socket is in blocking mode.
         */
        virtual void setBlocking(bool blocking);
        /**
         * \brief Clears the socket's ring buffer.
         */
        virtual void clearBuffer();
        /**
         * \brief Gets a message describing the last error encountered.
         * \returns The error message, as a string.
         */
        virtual const std::string& getLastError() const;

    protected:
        /**
         * \internal
         * \brief Generates a packet filter from a BPF capture filter string.
         * \param filter_string BPF capture filter string.
         * \param[out] capture_prog_ptr Pointer to the compiled capture
         *     filter program.  If the compilation fails, this returns NULL.
         */
        virtual void generatePacketFilter(
                const char* filter_string,
                struct sock_fprog** capture_prog_ptr
            );
        /**
         * \internal
         * \brief Flushes a block (that is, tells the kernel that the block
         *     is OK to overwrite).
         * \param pbd Pointer to the block being flushed.
         */
        virtual void flushBlock(struct block_desc *pbd);
        /**
         * \internal
         * \brief Cleans up buffer and other allocated objects.
         */
        virtual void doCleanup();

    protected:
        /**
         * \internal
         * \brief Shared mutex that protects against concurrent access to the BPF
         *     compile engine.
         * \note Needed because libpcap is not thread-safe for versions prior to
         *     1.8.0.
         */
        static std::mutex sharedMtx;

    protected:
        // Capture filter (BPF format)
        std::string _captureFilter;
        // Blocking mode
        bool _blocking;
        // Block size
        unsigned long _blockSize;
        // Number of blocks in the buffer
        unsigned long _numBlocks;
        // Frame size (must be a power of 2)
        unsigned long _frameSize;
        // Block timeout (ms)
        unsigned long _blockTimeout;
        // Polling timeout (ms)
        unsigned long _pollTimeout;
        // Socket File Descriptor
        int _sockfd;
        // Ring buffer
        struct ring _ring;
        // Current block index
        unsigned long _currentBlockInd;
        // Last error
        std::string _lastError;
};


#endif /* LIBCYBERRADIO_THREADEDZEROCOPYSOCKET_HPP */
