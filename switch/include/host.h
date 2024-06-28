// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

#ifndef CHIAKI_HOST_H
#define CHIAKI_HOST_H

#include <netinet/in.h>
#include <map>
#include <string>

#include <chiaki/controller.h>
#include <chiaki/discovery.h>
#include <chiaki/log.h>
#include <chiaki/opusdecoder.h>
#include <chiaki/regist.h>

#include "exception.h"
#include "io.h"
#include "settings.h"

class DiscoveryManager;
static void Discovery(ChiakiDiscoveryHost *, void *);
static void InitAudioCB(unsigned int channels, unsigned int rate, void *user);
static bool VideoCB(uint8_t *buf, size_t buf_size, void *user);
static void AudioCB(int16_t *buf, size_t samples_count, void *user);
static void EventCB(ChiakiEvent *event, void *user);
static void RegistEventCB(ChiakiRegistEvent *event, void *user);

enum RegistError
{
	HOST_REGISTER_OK,
	HOST_REGISTER_ERROR_SETTING_PSNACCOUNTID,
	HOST_REGISTER_ERROR_SETTING_PSNONLINEID
};

class Settings;

class Host
{
	private:
		ChiakiLog *log = nullptr;
		Settings *settings = nullptr;
		//video config
		ChiakiVideoResolutionPreset video_resolution = CHIAKI_VIDEO_RESOLUTION_PRESET_720p;
		ChiakiVideoFPSPreset video_fps = CHIAKI_VIDEO_FPS_PRESET_60;
		std::string host_type;
		// user info
		std::string psn_online_id = "";
		std::string psn_account_id = "";
		// info from regist/settings
		ChiakiRegist regist = {};
		ChiakiRegistInfo regist_info = {};
		std::function<void()> chiaki_regist_event_type_finished_canceled = nullptr;
		std::function<void()> chiaki_regist_event_type_finished_failed = nullptr;
		std::function<void()> chiaki_regist_event_type_finished_success = nullptr;
		std::function<void()> chiaki_event_connected_cb = nullptr;
		std::function<void(bool)> chiaki_even_login_pin_request_cb = nullptr;
		std::function<void(uint8_t, uint8_t)> chiaki_event_rumble_cb = nullptr;
		std::function<void(ChiakiQuitEvent *)> chiaki_event_quit_cb = nullptr;
		std::function<void(ChiakiControllerState *, std::map<uint32_t, int8_t> *)> io_read_controller_cb = nullptr;

		// internal state
		bool discovered = false;
		bool registered = false;
		// rp_key_data is true when rp_key, rp_regist_key, rp_key_type
		bool rp_key_data = false;

		std::string host_name;
		// sony's host_id == mac addr without colon
		std::string host_id;
		std::string host_addr;
		std::string ap_ssid;
		std::string ap_bssid;
		std::string ap_key;
		std::string ap_name;
		std::string server_nickname;
		ChiakiTarget target = CHIAKI_TARGET_PS4_UNKNOWN;
		ChiakiDiscoveryHostState state = CHIAKI_DISCOVERY_HOST_STATE_UNKNOWN;
		ChiakiControllerState controller_state = {0};
		std::map<uint32_t, int8_t> finger_id_touch_id;

		// mac address = 48 bits
		uint8_t server_mac[6] = {0};
		char rp_regist_key[CHIAKI_SESSION_AUTH_SIZE] = {0};
		uint32_t rp_key_type = 0;
		uint8_t rp_key[0x10] = {0};
		// manage stream session
		bool session_init = false;
		ChiakiSession session;
		ChiakiOpusDecoder opus_decoder;
		ChiakiConnectVideoProfile video_profile;
		friend class Settings;
		friend class DiscoveryManager;

	public:
		Host(std::string host_name);
		~Host();
		int Register(int pin);
		int Wakeup();
		int InitSession(IO *);
		int FiniSession();
		void StopSession();
		void StartSession();
		void SendFeedbackState();
		void RegistCB(ChiakiRegistEvent *);
		void ConnectionEventCB(ChiakiEvent *);
		bool GetVideoResolution(int *ret_width, int *ret_height);
		std::string GetHostName();
		std::string GetHostAddr();
		ChiakiTarget GetChiakiTarget();
		void SetChiakiTarget(ChiakiTarget target);
		void SetHostAddr(std::string host_addr);
		void SetRegistEventTypeFinishedCanceled(std::function<void()> chiaki_regist_event_type_finished_canceled);
		void SetRegistEventTypeFinishedFailed(std::function<void()> chiaki_regist_event_type_finished_failed);
		void SetRegistEventTypeFinishedSuccess(std::function<void()> chiaki_regist_event_type_finished_success);
		void SetEventConnectedCallback(std::function<void()> chiaki_event_connected_cb);
		void SetEventLoginPinRequestCallback(std::function<void(bool)> chiaki_even_login_pin_request_cb);
		void SetEventRumbleCallback(std::function<void(uint8_t, uint8_t)> chiaki_event_rumble_cb);
		void SetEventQuitCallback(std::function<void(ChiakiQuitEvent *)> chiaki_event_quit_cb);
		void SetReadControllerCallback(std::function<void(ChiakiControllerState *, std::map<uint32_t, int8_t> *)> io_read_controller_cb);
		bool IsRegistered();
		bool IsDiscovered();
		bool IsReady();
		bool HasRPkey();
		bool IsPS5();
};

#endif
