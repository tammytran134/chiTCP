/*
 *  chiTCP - A simple, testable TCP stack
 *
 *  Type and data structure declarations for implementing the
 *  TCP protocol. See tcp.c for the functions that actually
 *  run the TCP protocol.
 *
 */

/*
 *  Copyright (c) 2013-2014, The University of Chicago
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  - Neither the name of The University of Chicago nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "chitcp/buffer.h"
#include "chitcp/packet.h"
#include "chitcp/multitimer.h"

#ifndef TCP_H_
#define TCP_H_

#define TCP_BUFFER_SIZE (4096)
#define TCP_MSS (536)
#define CLOCK_G (50000000L)
#define MIN_RTO (200000000L)
#define MAX_RTO (60000000000L)
#define BETA (0.25)
#define ALPHA (0.125)
#define FIN_ON (1)
#define ACK_ON (1)
#define SYN_ON (1)
#define FIN_OFF (0)
#define ACK_OFF (0)
#define SYN_OFF (0)

/* TCP events. Roughly correspond to the ones specified in
 * http://tools.ietf.org/html/rfc793#section-3.9 */
typedef enum
{
    APPLICATION_CONNECT = 1,
    APPLICATION_SEND    = 2,
    APPLICATION_RECEIVE = 3,
    APPLICATION_CLOSE   = 4,
    PACKET_ARRIVAL      = 5,
    TIMEOUT_RTX         = 6,
    TIMEOUT_PST         = 7,
    CLEANUP             = 8
} tcp_event_type_t;


typedef enum
{
    RETRANSMISSION      = 0,
    PERSIST             = 1,
} tcp_timer_type_t;

/*  Many values in tcp_data have identifiers from RFC 793, as below     */

/*  From RFC 793 definition of the Transmission Control Block:

    Begin quote:

    Send Sequence Variables

    SND.UNA - send unacknowledged
    SND.NXT - send next
    SND.WND - send window
    SND.UP  - send urgent pointer
    SND.WL1 - segment sequence number used for last window update
    SND.WL2 - segment acknowledgment number used for last window
    update
    ISS     - initial send sequence number

    Receive Sequence Variables

    RCV.NXT - receive next
    RCV.WND - receive window
    RCV.UP  - receive urgent pointer
    IRS     - initial receive sequence number

    End quote.    */

/* SND.UP, SND.WL1, SND.WL2, and RCV.UP are unused */

static char *tcp_event_type_names[] =
{
    "APPLICATION_CONNECT",
    "APPLICATION_SEND",
    "APPLICATION_RECEIVE",
    "APPLICATION_CLOSE",
    "PACKET_ARRIVAL",
    "TIMEOUT_RTX",
    "TIMEOUT_PST",
    "CLEANUP"
};

static inline char *tcp_event_str (tcp_event_type_t evt)
{
    return tcp_event_type_names[evt-1];
}

/* Forward declarations */
typedef struct retransmission_queue retransmission_queue_t;
typedef struct out_of_order_list out_of_order_list_t;

typedef struct retransmission_queue
{
    tcp_packet_t *packet;
    tcp_seq expected_ack_seq;
    //struct timespec *timeout_spec;
    struct timespec *send_start;
    bool_t retransmitted;
    /* double linked list */
    retransmission_queue_t *prev;
    retransmission_queue_t *next;
} retransmission_queue_t;

typedef struct out_of_order_list
{
    tcp_packet_t *packet;
    tcp_seq seq;
    out_of_order_list_t *prev;
    out_of_order_list_t *next;
} out_of_order_list_t;

/* TCP data. Roughly corresponds to the variables and buffers
 * one would expect in a Transmission Control Block (as
 * specified in RFC 793). */
typedef struct tcp_data
{
    /* Queue with pending packets received from the network */
    tcp_packet_list_t *pending_packets;
    pthread_mutex_t lock_pending_packets;
    pthread_cond_t cv_pending_packets;

    /* Transmission control block */

    /* Send sequence variables */
    uint32_t ISS;      /* Initial send sequence number */
    uint32_t SND_UNA;  /* First byte sent but not acknowledged */
    uint32_t SND_NXT;  /* Next sendable byte */
    uint16_t SND_WND;  /* Send Window */

    /* Receive sequence variables */
    uint32_t IRS;      /* Initial receive sequence number */
    uint32_t RCV_NXT;  /* Next byte expected */
    uint16_t RCV_WND;  /* Receive Window */

    /* Buffers */
    circular_buffer_t send;
    circular_buffer_t recv;

    /* Number of bytes sent but unacknowledged in send buffer */
    int unack_bytes;

    /* Has a CLOSE been requested on this socket? */
    bool_t closing;
    /* The state after CLOSE call */
    tcp_state_t state_after_close;

    /* multitimer */
    multi_timer_t *tcp_timer;

    /* Retransmission queue */
    retransmission_queue_t *queue;

    /* Out-of-order list */
    out_of_order_list_t *list;

    /* PST probe packet */
    tcp_packet_t *probe_packet;

    /* RTT retransmission */
    bool_t first_RTT;
    uint64_t RTT;
    uint64_t RTO;
    uint64_t SRTT;
    uint64_t RTTVAR;
    bool_t rtms_timer_on;
} tcp_data_t;

#endif /* TCP_H_ */
