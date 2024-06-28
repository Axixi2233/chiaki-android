// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

#ifndef CHIAKI_HTTP_H
#define CHIAKI_HTTP_H

#include "common.h"
#include "stoppipe.h"

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct chiaki_http_header_t
{
	const char *key;
	const char *value;
	struct chiaki_http_header_t *next;
} ChiakiHttpHeader;

typedef struct chiaki_http_response_t
{
	int code;
	ChiakiHttpHeader *headers;
} ChiakiHttpResponse;

CHIAKI_EXPORT void chiaki_http_header_free(ChiakiHttpHeader *header);
CHIAKI_EXPORT ChiakiErrorCode chiaki_http_header_parse(ChiakiHttpHeader **header, char *buf, size_t buf_size);

CHIAKI_EXPORT void chiaki_http_response_fini(ChiakiHttpResponse *response);
CHIAKI_EXPORT ChiakiErrorCode chiaki_http_response_parse(ChiakiHttpResponse *response, char *buf, size_t buf_size);

/**
 * @param stop_pipe optional
 * @param timeout_ms only used if stop_pipe is not NULL
 */
CHIAKI_EXPORT ChiakiErrorCode chiaki_recv_http_header(int sock, char *buf, size_t buf_size, size_t *header_size, size_t *received_size, ChiakiStopPipe *stop_pipe, uint64_t timeout_ms);


#ifdef __cplusplus
}
#endif

#endif // CHIAKI_HTTP_H
