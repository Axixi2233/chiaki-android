// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

#ifndef CHIAKI_CTRL_H
#define CHIAKI_CTRL_H

#include "common.h"
#include "thread.h"
#include "stoppipe.h"

#include <stdint.h>
#include <stdbool.h>

#if _WIN32
#include <winsock2.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct chiaki_ctrl_message_queue_t ChiakiCtrlMessageQueue;

typedef struct chiaki_ctrl_t
{
	struct chiaki_session_t *session;
	ChiakiThread thread;

	bool should_stop;
	bool login_pin_entered;
	uint8_t *login_pin;
	size_t login_pin_size;
	ChiakiCtrlMessageQueue *msg_queue;
	ChiakiStopPipe notif_pipe;
	ChiakiMutex notif_mutex;

	bool login_pin_requested;

	chiaki_socket_t sock;

#ifdef __GNUC__
	__attribute__((aligned(__alignof__(uint32_t))))
#endif
	uint8_t recv_buf[512];

	size_t recv_buf_size;
	uint64_t crypt_counter_local;
	uint64_t crypt_counter_remote;
	uint32_t keyboard_text_counter;
} ChiakiCtrl;

CHIAKI_EXPORT ChiakiErrorCode chiaki_ctrl_init(ChiakiCtrl *ctrl, struct chiaki_session_t *session);
CHIAKI_EXPORT ChiakiErrorCode chiaki_ctrl_start(ChiakiCtrl *ctrl);
CHIAKI_EXPORT void chiaki_ctrl_stop(ChiakiCtrl *ctrl);
CHIAKI_EXPORT ChiakiErrorCode chiaki_ctrl_join(ChiakiCtrl *ctrl);
CHIAKI_EXPORT void chiaki_ctrl_fini(ChiakiCtrl *ctrl);
CHIAKI_EXPORT ChiakiErrorCode chiaki_ctrl_send_message(ChiakiCtrl *ctrl, uint16_t type, const uint8_t *payload, size_t payload_size);
CHIAKI_EXPORT void chiaki_ctrl_set_login_pin(ChiakiCtrl *ctrl, const uint8_t *pin, size_t pin_size);
CHIAKI_EXPORT ChiakiErrorCode chiaki_ctrl_goto_bed(ChiakiCtrl *ctrl);
CHIAKI_EXPORT ChiakiErrorCode chiaki_ctrl_keyboard_set_text(ChiakiCtrl *ctrl, const char* text);
CHIAKI_EXPORT ChiakiErrorCode chiaki_ctrl_keyboard_accept(ChiakiCtrl *ctrl);
CHIAKI_EXPORT ChiakiErrorCode chiaki_ctrl_keyboard_reject(ChiakiCtrl *ctrl);

#ifdef __cplusplus
}
#endif

#endif // CHIAKI_CTRL_H
