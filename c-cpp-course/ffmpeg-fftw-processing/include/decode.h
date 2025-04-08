#ifndef DECODE_H
#define DECODE_H

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include <stdbool.h>
#include <stdint.h>

#define NOT_STEREO_ERROR 105
#define CANNOT_OPEN_FILE_ERROR 106
#define NO_AUDIO_STREAM 107
#define ALLOC_ERROR (-110)
#define INVALID_FORMAT 111
typedef struct
{
	AVFormatContext* f_context;
	AVCodecContext* codec_context;
	AVFrame* frame;
	AVPacket* packet;
	int32_t audio_stream_index;
	int32_t channels;
	const AVCodec* codec;
} AudioContainerUnits;
typedef struct
{
	double* channels[2];
	int32_t capacities[2];
	int32_t sample_rate;
} Channels;
Channels* init_channels(void);
void free_channels(Channels* AudioChannels);
AudioContainerUnits get_units(void);
int prepare_audio_container(char* name, AudioContainerUnits* Units, int32_t* channel_sample_rate);
int decode_audio(AudioContainerUnits* Units, Channels* AudioChannels, bool is_stereo, int32_t max_sample_rate);

#endif
