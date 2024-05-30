// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef __RTP_RECEIVER_TEST_H__
#define __RTP_RECEIVER_TEST_H__
#include "sockutil.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void rtp_receiver_test(socket_t rtp[2], const char* peer, int peerport[2], int payload, const char* encoding);
void* rtp_receiver_tcp_test(uint8_t interleave1, uint8_t interleave2, int payload, const char* encoding);
void rtp_receiver_tcp_input(uint8_t channel, const void* data, uint16_t bytes);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
