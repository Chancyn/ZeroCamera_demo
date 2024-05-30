#include "rtsp-server-aio.h"
#include "aio-transport.h"
#include "rtp-over-rtsp.h"
#include "sys/sock.h"
#include "sys/atomic.h"
#include "sys/system.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#define TIMEOUT_RECV 65000
#define TIMEOUT_SEND 10000

struct rtsp_session_t
{
	socket_t socket;
	aio_transport_t* aio;
	struct rtp_over_rtsp_t rtp;
	int rtsp_need_more_data;
	uint8_t buffer[4 * 1024];

	struct rtsp_server_t *rtsp;
	struct sockaddr_storage addr;
	socklen_t addrlen;

	void (*onerror)(void* param, rtsp_server_t* rtsp, int code);
    void (*onerror2)(void* param, rtsp_server_t* rtsp, int code, void *ptr);
	void (*onrtp)(void* param, uint8_t channel, const void* data, uint16_t bytes);
	void (*onrtp2)(void* param, uint8_t channel, const void* data, uint16_t bytes, void *ptr);
	void* param;
};

static void rtsp_session_ondestroy(void* param)
{
	struct rtsp_session_t *session;
	session = (struct rtsp_session_t *)param;

	// user call rtsp_server_destroy
	if (session->rtsp)
	{
		rtsp_server_destroy(session->rtsp);
		session->rtsp = NULL;
	}

	if (session->rtp.data)
	{
		assert(session->rtp.capacity > 0);
		free(session->rtp.data);
		session->rtp.data = NULL;
		session->rtp.capacity = 0;
	}

#if defined(_DEBUG) || defined(DEBUG)
	memset(session, 0xCC, sizeof(*session));
#endif
	free(session);
}

static void rtsp_session_onrecv(void* param, int code, size_t bytes)
{
	size_t remain;
	const uint8_t* p, *end;
	struct rtsp_session_t *session;
	session = (struct rtsp_session_t *)param;

	if (0 == code && 0 == bytes)
		code = ECONNRESET;

	if (0 == code)
	{
		p = session->buffer;
		end = session->buffer + bytes;
		do
		{
			if (0 == session->rtsp_need_more_data && ('$' == *p || 0 != session->rtp.state))
			{
				p = rtp_over_rtsp(&session->rtp, p, end);
			}
			else
			{
				remain = end - p;
				code = rtsp_server_input(session->rtsp, p, &remain);
				session->rtsp_need_more_data = code;
				if (0 == code)
				{
					// TODO: pipeline remain data
					assert(bytes > remain);
					assert(0 == remain || '$' == *(end - remain));
				}
				p = end - remain;
			}
		} while (p < end && 0 == code);

		if (code >= 0)
		{
			// need more data
			code = aio_transport_recv(session->aio, session->buffer, sizeof(session->buffer));
		}
	}

	// error or peer closed
	if (0 != code || 0 == bytes)
	{
		session->onerror(session->param, session->rtsp, code ? code : ECONNRESET);
		aio_transport_destroy(session->aio);
	}
}

static void rtsp_session_onsend(void* param, int code, size_t bytes)
{
	struct rtsp_session_t *session;
	session = (struct rtsp_session_t *)param;
//	session->server->onsend(session, code, bytes);
	if (0 != code)
	{
		session->onerror(session->param, session->rtsp, code);
		aio_transport_destroy(session->aio);
	}
	(void)bytes;
}

static int rtsp_session_send(void* ptr, const void* data, size_t bytes)
{
	struct rtsp_session_t *session;
	session = (struct rtsp_session_t *)ptr;
	//return aio_tcp_transport_send(session->aio, data, bytes);

	// TODO: send multiple rtp packet once time
	return bytes == socket_send(session->socket, data, bytes, 0) ? 0 : -1;
}

static void rtsp_session_onrtp(void *param, uint8_t channel, const void *data, uint16_t bytes)
{
    struct rtsp_session_t* session = (struct rtsp_session_t*)param;
    if (session)
    {
        if (session->onrtp2)
        {
            printf("rtsp_session_onrtp onrtp2 session:%p, param:%p\n", session, session->param);
            // session->param:rtsp_transport_tcp_create regitster param ptr, return with session ptr
            return session->onrtp2(session->param, channel, data, bytes, session);
        }
        else if (session->onrtp)
        {
            printf("rtsp_session_onrtp onrtp session:%p\n", session);
            return session->onrtp(session->param, channel, data, bytes);
        }
    }
    return;
}

static void rtsp_session_onerror(void* param, rtsp_server_t* rtsp, int code, void *ptr)
{
    struct rtsp_session_t* session = (struct rtsp_session_t*)param;
    if (session)
    {
        if (session->onerror2)
        {
            printf("rtsp_session_onrtp onerror2 session:%p, param:%p\n", session, session->param);
            // session->param:rtsp_transport_tcp_create regitster param ptr, return with session ptr
            return session->onerror2(session->param, rtsp, code, session);
        }
        else if (session->onerror)
        {
            printf("rtsp_session_onrtp onerror session:%p\n", session);
            return session->onerror(session->param, rtsp, code);
        }
    }
    return;
}

int rtsp_transport_tcp_create(socket_t socket, const struct sockaddr* addr, socklen_t addrlen, struct aio_rtsp_handler_t* handler, void* param)
{
	char ip[65];
	unsigned short port;
	struct rtsp_session_t *session;
	struct rtsp_handler_t rtsphandler;
	struct aio_transport_handler_t h;

	memset(&h, 0, sizeof(h));
	h.ondestroy = rtsp_session_ondestroy;
	h.onrecv = rtsp_session_onrecv;
	h.onsend = rtsp_session_onsend;

	memcpy(&rtsphandler, &handler->base, sizeof(rtsphandler));
	rtsphandler.send = rtsp_session_send;

	session = (struct rtsp_session_t*)calloc(1, sizeof(*session));
	if (!session) return -ENOMEM;

	session->socket = socket;
	socket_addr_to(addr, addrlen, ip, &port);
	assert(addrlen <= sizeof(session->addr));
	session->addrlen = addrlen < sizeof(session->addr) ? addrlen : sizeof(session->addr);
	memcpy(&session->addr, addr, session->addrlen); // save client ip/port

	session->param = param;
	session->onerror = handler->onerror;
	session->aio = aio_transport_create(socket, &h, session);
	session->rtsp = rtsp_server_create(ip, port, &rtsphandler, param, session); // reuse-able, don't need create in every link
	if (!session->rtsp || !session->aio)
	{
		rtsp_session_ondestroy(session);
		return -ENOMEM;
	}

    // version2 regiester
	if (handler->onrtp2)
	{
        printf("create session:%p, param:%p\n", session, param);
		session->rtp.param = session;               // must use seesion ptr
		session->rtp.onrtp = rtsp_session_onrtp;    // call onrtp2,callback with seesion ptr
		session->onrtp2 = handler->onrtp2;
        session->onrtp = handler->onrtp;
        // session->rtp.onerror = rtsp_session_onerror;  // call onrtp2,callback with seesion ptr
        // session->onerror2 = handler->onerror2;
        // session->onerror = handler->onerror;
	}else {
		session->rtp.param = param;
		session->rtp.onrtp = handler->onrtp;
	}

	aio_transport_set_timeout(session->aio, TIMEOUT_RECV, TIMEOUT_SEND);
	if (0 != aio_transport_recv(session->aio, session->buffer, sizeof(session->buffer)))
	{
		rtsp_session_ondestroy(session);
		return -ENOTCONN;
	}

	return 0;
}
