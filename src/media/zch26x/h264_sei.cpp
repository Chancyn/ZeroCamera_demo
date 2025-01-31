//
// Created by monktan on 2020/10/21.
//


#include "bs.h"
#include "h264_stream.h"
#include "zc_macros.h"
#include "h264_sei.h"

#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // memset

sei_t* sei_new()
{
    sei_t* s = (sei_t*)malloc(sizeof(sei_t));
    memset(s, 0, sizeof(sei_t));
    s->payload = NULL;
    return s;
}

void sei_free(sei_t* s)
{
    if ( s->payload != NULL ) free(s->payload);
    free(s);
}

void read_sei_end_bits(h264_stream_t* h, bs_t* b )
{
    // if the message doesn't end at a byte border
    if ( !bs_byte_aligned( b ) )
    {
        if ( !bs_read_u1( b ) ) fprintf(stderr, "WARNING: bit_equal_to_one is 0!!!!\n");
        while ( ! bs_byte_aligned( b ) )
        {
            if ( bs_read_u1( b ) ) fprintf(stderr, "WARNING: bit_equal_to_zero is 1!!!!\n");
        }
    }

    read_rbsp_trailing_bits(h, b);
}

static void read_user_data_unregistered(h264_stream_t* h, bs_t* b, int payloadSize)
{
    sei_t* s = h->sei;

    s->payload = (uint8_t*)malloc(payloadSize);

    int i;

    // uuid_iso_iec_11578 todo...
    for (i = 0; i < 16; i++)
        s->payload[i] = bs_read_u(b, 8);
    for (i = 16; i < payloadSize; i++)
        s->payload[i] = bs_read_u(b, 8);
}

// D.1 SEI payload syntax
void read_sei_payload(h264_stream_t* h, bs_t* b, int payloadType, int payloadSize)
{
    ZC_UNUSED sei_t* s = h->sei;

    switch (payloadType)
    {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            break;
        case 5:
            read_user_data_unregistered(h, b, payloadSize);
            break;
        default:
            break;
    }

    read_sei_end_bits(h, b);
}

// D.1 SEI payload syntax
void write_sei_payload(h264_stream_t* h, bs_t* b, int payloadType, int payloadSize)
{
    sei_t* s = h->sei;

    int i;
    for ( i = 0; i < s->payloadSize; i++ )
        bs_write_u(b, 8, s->payload[i]);
}
