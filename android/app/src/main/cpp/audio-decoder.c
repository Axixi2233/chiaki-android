// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

#include "audio-decoder.h"

#include <jni.h>

#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>

#include <string.h>
#include "chiaki-jni.h"

#define INPUT_BUFFER_TIMEOUT_MS 10

static void *android_chiaki_audio_decoder_output_thread_func(void *user);
static void android_chiaki_audio_decoder_header(ChiakiAudioHeader *header, void *user);
static void android_chiaki_audio_decoder_frame(uint8_t *buf, size_t buf_size, void *user);
static void android_chiaki_audio_haptics_decoder_header(ChiakiAudioHeader *header, void *user);
static void android_chiaki_audio_haptics_decoder_frame(uint8_t *buf, size_t buf_size, void *user);

ChiakiErrorCode android_chiaki_audio_decoder_init(AndroidChiakiAudioDecoder *decoder, ChiakiLog *log)
{
	decoder->log = log;
	memset(&decoder->audio_header, 0, sizeof(decoder->audio_header));
	decoder->codec = NULL;
	decoder->timestamp_cur = 0;

	decoder->cb_user = NULL;
	decoder->settings_cb = NULL;
	decoder->frame_cb = NULL;

	return chiaki_mutex_init(&decoder->codec_mutex, true);
}

void android_chiaki_audio_decoder_shutdown_codec(AndroidChiakiAudioDecoder *decoder)
{
	chiaki_mutex_lock(&decoder->codec_mutex);
	ssize_t codec_buf_index = AMediaCodec_dequeueInputBuffer(decoder->codec, -1);
	if(codec_buf_index >= 0)
	{
		CHIAKI_LOGI(decoder->log, "Audio Decoder sending EOS buffer");
		AMediaCodec_queueInputBuffer(decoder->codec, (size_t)codec_buf_index, 0, 0, decoder->timestamp_cur++, AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
	}
	else
		CHIAKI_LOGE(decoder->log, "Failed to get input buffer for shutting down Audio Decoder!");
	chiaki_mutex_unlock(&decoder->codec_mutex);
	chiaki_thread_join(&decoder->output_thread, NULL);
	AMediaCodec_delete(decoder->codec);
	decoder->codec = NULL;
}

void android_chiaki_audio_decoder_fini(AndroidChiakiAudioDecoder *decoder)
{
	if(decoder->codec)
		android_chiaki_audio_decoder_shutdown_codec(decoder);
	chiaki_mutex_fini(&decoder->codec_mutex);
}

void android_chiaki_audio_decoder_get_sink(AndroidChiakiAudioDecoder *decoder, ChiakiAudioSink *sink)
{
	sink->user = decoder;
	sink->header_cb = android_chiaki_audio_decoder_header;
	sink->frame_cb = android_chiaki_audio_decoder_frame;
}

void android_chiaki_audio_haptics_decoder_get_sink(ChiakiSession *session,AndroidChiakiAudioDecoder *decoder, ChiakiAudioSink *sink)
{
    sink->user = session;
    sink->header_cb = android_chiaki_audio_haptics_decoder_header;
    sink->frame_cb = android_chiaki_audio_haptics_decoder_frame;
}

static void *android_chiaki_audio_decoder_output_thread_func(void *user)
{
	AndroidChiakiAudioDecoder *decoder = user;

	while(1)
	{
		AMediaCodecBufferInfo info;
		ssize_t codec_buf_index = AMediaCodec_dequeueOutputBuffer(decoder->codec, &info, -1);
		if(codec_buf_index >= 0)
		{
			if(decoder->settings_cb)
			{
				size_t codec_buf_size;
				uint8_t *codec_buf = AMediaCodec_getOutputBuffer(decoder->codec, (size_t)codec_buf_index, &codec_buf_size);
				size_t samples_count = info.size / sizeof(int16_t);
				//CHIAKI_LOGD(decoder->log, "Got %llu samples => %f ms of audio", (unsigned long long)samples_count, 1000.0f * (float)(samples_count / 2) / (float)decoder->audio_header.rate);
				decoder->frame_cb((int16_t *)codec_buf, samples_count, decoder->cb_user);
			}
			AMediaCodec_releaseOutputBuffer(decoder->codec, (size_t)codec_buf_index, false);
			if(info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM)
			{
				CHIAKI_LOGI(decoder->log, "AMediaCodec for Audio Decoder reported EOS");
				break;
			}
		}
	}

	CHIAKI_LOGI(decoder->log, "Audio Decoder Output Thread exiting");

	return NULL;
}

static void android_chiaki_audio_haptics_decoder_header(ChiakiAudioHeader *header, void *user){
//    AndroidChiakiAudioDecoder *decoder = user;
//    CHIAKI_LOGI(decoder->log, "axixiLog-触觉反馈-音频解析-header");
}

static void android_chiaki_audio_decoder_header(ChiakiAudioHeader *header, void *user)
{
	AndroidChiakiAudioDecoder *decoder = user;
	chiaki_mutex_lock(&decoder->codec_mutex);
	memcpy(&decoder->audio_header, header, sizeof(decoder->audio_header));

	if(decoder->codec)
	{
		CHIAKI_LOGI(decoder->log, "Audio decoder already initialized, shutting down the old one");
		android_chiaki_audio_decoder_shutdown_codec(decoder);
	}

	const char *mime = "audio/opus";
	decoder->codec = AMediaCodec_createDecoderByType(mime);
	if(!decoder->codec)
	{
		CHIAKI_LOGE(decoder->log, "Failed to create AMediaCodec for mime type %s", mime);
		goto beach;
	}

	AMediaFormat *format = AMediaFormat_new();
	AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, mime);
	AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_CHANNEL_COUNT, header->channels);
	AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_SAMPLE_RATE, header->rate);
	// AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_PCM_ENCODING)

	AMediaCodec_configure(decoder->codec, format, NULL, NULL, 0); // TODO: check result
	AMediaCodec_start(decoder->codec); // TODO: check result

	AMediaFormat_delete(format);

	ChiakiErrorCode err = chiaki_thread_create(&decoder->output_thread, android_chiaki_audio_decoder_output_thread_func, decoder);
	if(err != CHIAKI_ERR_SUCCESS)
	{
		CHIAKI_LOGE(decoder->log, "Failed to create output thread for AMediaCodec");
		AMediaCodec_delete(decoder->codec);
		decoder->codec = NULL;
	}

	uint8_t opus_id_head[0x13];
	memcpy(opus_id_head, "OpusHead", 8);
	opus_id_head[0x8] = 1; // version
	opus_id_head[0x9] = header->channels;
	uint16_t pre_skip = 3840;
	opus_id_head[0xa] = (uint8_t)(pre_skip & 0xff);
	opus_id_head[0xb] = (uint8_t)(pre_skip >> 8);
	opus_id_head[0xc] = (uint8_t)(header->rate & 0xff);
	opus_id_head[0xd] = (uint8_t)((header->rate >> 0x8) & 0xff);
	opus_id_head[0xe] = (uint8_t)((header->rate >> 0x10) & 0xff);
	opus_id_head[0xf] = (uint8_t)(header->rate >> 0x18);
	uint16_t output_gain = 0;
	opus_id_head[0x10] = (uint8_t)(output_gain & 0xff);
	opus_id_head[0x11] = (uint8_t)(output_gain >> 8);
	opus_id_head[0x12] = 0; // channel map
	//AMediaFormat_setBuffer(format, AMEDIAFORMAT_KEY_CSD_0, opus_id_head, sizeof(opus_id_head));
	android_chiaki_audio_decoder_frame(opus_id_head, sizeof(opus_id_head), decoder);

	uint64_t pre_skip_ns = 0;
	uint8_t csd1[8] = { (uint8_t)(pre_skip_ns & 0xff), (uint8_t)((pre_skip_ns >> 0x8) & 0xff), (uint8_t)((pre_skip_ns >> 0x10) & 0xff), (uint8_t)((pre_skip_ns >> 0x18) & 0xff),
						(uint8_t)((pre_skip_ns >> 0x20) & 0xff), (uint8_t)((pre_skip_ns >> 0x28) & 0xff), (uint8_t)((pre_skip_ns >> 0x30) & 0xff), (uint8_t)(pre_skip_ns >> 0x38)};
	android_chiaki_audio_decoder_frame(csd1, sizeof(csd1), decoder);

	uint64_t pre_roll_ns = 0;
	uint8_t csd2[8] = { (uint8_t)(pre_roll_ns & 0xff), (uint8_t)((pre_roll_ns >> 0x8) & 0xff), (uint8_t)((pre_roll_ns >> 0x10) & 0xff), (uint8_t)((pre_roll_ns >> 0x18) & 0xff),
						(uint8_t)((pre_roll_ns >> 0x20) & 0xff), (uint8_t)((pre_roll_ns >> 0x28) & 0xff), (uint8_t)((pre_roll_ns >> 0x30) & 0xff), (uint8_t)(pre_roll_ns >> 0x38)};
	android_chiaki_audio_decoder_frame(csd2, sizeof(csd2), decoder);

	if(decoder->settings_cb)
		decoder->settings_cb(header->channels, header->rate, decoder->cb_user);

beach:
	chiaki_mutex_unlock(&decoder->codec_mutex);
}


static void android_chiaki_audio_haptics_decoder_frame(uint8_t *buf, size_t buf_size, void *user){
    if(buf_size < 25){return;}
    ChiakiSession *session = user;
    CHIAKI_LOGI(session->log, "axixiLog-触觉反馈-音频解析-frame");
    ChiakiEvent event = { 0 };
    event.type = CHIAKI_EVENT_RUMBLE;
    event.rumble.unknown = buf[0];
    event.rumble.left = buf[1];
    event.rumble.right = buf[2];
    session->event_cb(&event,session->event_cb_user);
}
static void android_chiaki_audio_decoder_frame(uint8_t *buf, size_t buf_size, void *user)
{
	AndroidChiakiAudioDecoder *decoder = user;
	chiaki_mutex_lock(&decoder->codec_mutex);

	if(!decoder->codec)
	{
		CHIAKI_LOGE(decoder->log, "Received audio data, but decoder is not initialized!");
		goto beach;
	}

	while(buf_size > 0)
	{
		ssize_t codec_buf_index = AMediaCodec_dequeueInputBuffer(decoder->codec, INPUT_BUFFER_TIMEOUT_MS * 1000);
		if(codec_buf_index < 0)
		{
			CHIAKI_LOGE(decoder->log, "Failed to get input audio buffer");
			return;
		}

		size_t codec_buf_size;
		uint8_t *codec_buf = AMediaCodec_getInputBuffer(decoder->codec, (size_t)codec_buf_index, &codec_buf_size);
		size_t codec_sample_size = buf_size;
		if(codec_sample_size > codec_buf_size)
		{
			CHIAKI_LOGD(decoder->log, "Sample is bigger than audio buffer, splitting");
			codec_sample_size = codec_buf_size;
		}
		memcpy(codec_buf, buf, codec_sample_size);
		AMediaCodec_queueInputBuffer(decoder->codec, (size_t)codec_buf_index, 0, codec_sample_size, decoder->timestamp_cur++, 0); // timestamp just raised by 1 for maximum realtime
		buf += codec_sample_size;
		buf_size -= codec_sample_size;
	}

beach:
	chiaki_mutex_unlock(&decoder->codec_mutex);
}