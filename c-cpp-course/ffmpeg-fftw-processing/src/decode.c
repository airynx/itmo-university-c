#include "decode.h"

#include <libavutil/audio_fifo.h>
#include <libavutil/avutil.h>
#include <libavutil/frame.h>
#include <libswresample/swresample.h>

#define NOT_STEREO_ERROR 105
#define CANNOT_OPEN_FILE_ERROR 106
#define NO_AUDIO_STREAM 107
#define CODEC_NOT_FOUND 108
#define CODEC_NOT_SUPPORTED 109
#define ALLOC_ERROR (-110)
#define FRAME_READING_ERROR 111
#define FRAME_RECEIVING_ERROR 112
#define FILE_CORRUPTED_ERROR 113
#define NEGATIVE_INDEX (-1)
#define GOOD 0

////
// На вход подается аудиоконтейнер. Функция decode извлекает аудиоканалы.
////

AudioContainerUnits get_units(void)
{
	AudioContainerUnits Units = { .f_context = NULL, .codec_context = NULL, .frame = av_frame_alloc(), .packet = av_packet_alloc() };
	return Units;
}

Channels* init_channels(void)
{
	Channels* AudioChannels = malloc(sizeof(Channels));
	AudioChannels->channels[0] = calloc(1, sizeof(double));
	AudioChannels->channels[1] = calloc(1, sizeof(double));
	AudioChannels->capacities[0] = 0;
	AudioChannels->capacities[1] = 0;
	AudioChannels->sample_rate = 0;
	return AudioChannels;
}
void free_channels(Channels* AudioChannels)
{
	free(AudioChannels->channels[0]);
	free(AudioChannels->channels[1]);
	free(AudioChannels);
}
static int32_t realloc_array(double** array, int32_t current_len, int32_t needed_capacity)
{
	if (current_len >= needed_capacity)
		return current_len;
	current_len = (needed_capacity > 1000000 ? 2 : 1.3) * needed_capacity;
	double* temp_buffer = realloc(*array, current_len * sizeof(double));
	if (!temp_buffer)
		return NEGATIVE_INDEX;
	*array = temp_buffer;
	return current_len;
}
static int parse_av_error(int av_error_code)
{
	switch (av_error_code)
	{
	case AVERROR(ENOMEM):
		return ALLOC_ERROR;
	case AVERROR(ENOENT):
		return CANNOT_OPEN_FILE_ERROR;
	default:
		return FILE_CORRUPTED_ERROR;
	}
}
static void free_codec_context(AVCodecContext** codec_context_ptr)
{
	if (codec_context_ptr && *codec_context_ptr)
	{
		avcodec_close(*codec_context_ptr);
		avcodec_free_context(codec_context_ptr);
		*codec_context_ptr = NULL;
	}
}
static void free_format_context(AVFormatContext** f_context)
{
	if (f_context && *f_context)
	{
		avformat_close_input(f_context);
		*f_context = NULL;
	}
}
static void free_frame(AVFrame** frame)
{
	if (frame && *frame)
	{
		av_frame_free(frame);
		*frame = NULL;
	}
}
static void free_packet(AVPacket** packet)
{
	if (packet && *packet)
	{
		av_packet_free(packet);
		*packet = NULL;
	}
}
static void free_resources(AudioContainerUnits* Units)
{
	free_frame(&Units->frame);
	free_packet(&Units->packet);
	free_codec_context(&Units->codec_context);
	free_format_context(&Units->f_context);
}
static int add_samples(Channels* AudioChannels, int32_t* array_size1, double** data, int frame_count, int ch)
{
	*array_size1 = realloc_array(&AudioChannels->channels[ch], *array_size1, AudioChannels->capacities[ch] + frame_count);
	if (*array_size1 == NEGATIVE_INDEX)
		return ALLOC_ERROR;

	memcpy(AudioChannels->channels[ch] + AudioChannels->capacities[ch], data[ch], frame_count * sizeof(double));
	AudioChannels->capacities[ch] += frame_count;
	return GOOD;
}
static int extract_audio(AudioContainerUnits* Units, Channels* AudioChannels, bool is_stereo, int32_t max_sample_rate)
{
	int32_t array_size1 = 0;
	int32_t array_size2 = 0;
	int return_code = GOOD;

	struct SwrContext* swr_context = NULL;
	swr_alloc_set_opts2(
		&swr_context,
		&Units->codec_context->ch_layout,
		AV_SAMPLE_FMT_DBLP,
		max_sample_rate,
		&Units->codec_context->ch_layout,
		Units->codec_context->sample_fmt,
		Units->codec_context->sample_rate,
		0,
		NULL);
	if (!swr_context)
		return ALLOC_ERROR;
	return_code = swr_init(swr_context);
	if (return_code < 0)
		return parse_av_error(return_code);
	AVFrame* output_frame = av_frame_alloc();
	if (!output_frame)
	{
		swr_free(&swr_context);
		return ALLOC_ERROR;
	}
	while ((return_code = av_read_frame(Units->f_context, Units->packet)) != AVERROR_EOF)
	{
		if (return_code)
		{
			return_code = FRAME_READING_ERROR;
			break;
		}
		if (Units->packet->stream_index != Units->audio_stream_index)
		{
			av_packet_unref(Units->packet);
			continue;
		}
		return_code = avcodec_send_packet(Units->codec_context, Units->packet);
		if (return_code != GOOD)
		{
			return_code = parse_av_error(return_code);
			break;
		}

		while (!(return_code = avcodec_receive_frame(Units->codec_context, Units->frame)))
		{
			int64_t out_nb_samples = av_rescale_rnd(
				swr_get_delay(swr_context, Units->codec_context->sample_rate) + Units->frame->nb_samples,
				max_sample_rate,
				Units->codec_context->sample_rate,
				AV_ROUND_UP);

			return_code = av_samples_alloc(output_frame->data, NULL, (is_stereo ? 2 : 1), out_nb_samples, AV_SAMPLE_FMT_DBLP, 0);
			if (return_code < 0)
				return ALLOC_ERROR;

			int32_t frame_count = swr_convert(
				swr_context,
				output_frame->data,
				out_nb_samples,
				(const uint8_t**)Units->frame->data,
				Units->frame->nb_samples);
			double** data = (double**)output_frame->data;

			return_code = add_samples(AudioChannels, &array_size1, data, frame_count, 0);
			if (return_code)
				break;

			if (is_stereo)
			{
				return_code = add_samples(AudioChannels, &array_size2, data, frame_count, 1);
				if (return_code)
					break;
			}
			av_freep(&output_frame->data[0]);
			av_frame_unref(Units->frame);
		}
		av_packet_unref(Units->packet);
		if (return_code != AVERROR(EAGAIN))
		{
			return_code = FRAME_RECEIVING_ERROR;
			break;
		}
	}
	av_frame_free(&output_frame);
	swr_free(&swr_context);
	if (return_code == AVERROR_EOF)
	{
		AudioChannels->channels[0] = realloc(AudioChannels->channels[0], AudioChannels->capacities[0] * sizeof(double));
		if (is_stereo)
			AudioChannels->channels[1] = realloc(AudioChannels->channels[1], AudioChannels->capacities[1] * sizeof(double));
		return GOOD;
	}
	return return_code;
}
static int deploy_codec(AudioContainerUnits* Units)
{
	Units->codec = avcodec_find_decoder(Units->f_context->streams[Units->audio_stream_index]->codecpar->codec_id);
	if (Units->codec == NULL)
		return CODEC_NOT_FOUND;

	Units->codec_context = avcodec_alloc_context3(Units->codec);
	if (Units->codec_context == NULL)
		return ALLOC_ERROR;

	if (avcodec_parameters_to_context(Units->codec_context, Units->f_context->streams[Units->audio_stream_index]->codecpar))
		return CODEC_NOT_SUPPORTED;

	Units->channels = Units->f_context->streams[Units->audio_stream_index]->codecpar->ch_layout.nb_channels;

	if (avcodec_open2(Units->codec_context, Units->codec, NULL) < 0)
		return CODEC_NOT_SUPPORTED;

	return GOOD;
}
static int set_props_to_container(AudioContainerUnits* Units)
{
	int return_code;
	return_code = avformat_find_stream_info(Units->f_context, NULL);
	if (return_code < 0)
		return parse_av_error(return_code);

	Units->audio_stream_index = av_find_best_stream(Units->f_context, AVMEDIA_TYPE_AUDIO, -1, -1, &Units->codec, 0);
	if (Units->audio_stream_index == AVERROR_STREAM_NOT_FOUND)
		return NO_AUDIO_STREAM;

	return_code = deploy_codec(Units);
	if (return_code)
		return return_code;
	return GOOD;
}
int prepare_audio_container(char* name, AudioContainerUnits* Units, int32_t* channel_sample_rate)
{
	int return_code;
	if (Units->packet == NULL || Units->frame == NULL)
	{
		free_packet(&Units->packet);
		free_frame(&Units->frame);
		return ALLOC_ERROR;
	}

	return_code = avformat_open_input(&Units->f_context, name, NULL, NULL);
	if (return_code)
	{
		free_packet(&Units->packet);
		free_frame(&Units->frame);
		return parse_av_error(return_code);
	}

	return_code = set_props_to_container(Units);
	if (Units->audio_stream_index >= 0)
	{
		int format = Units->f_context->streams[Units->audio_stream_index]->codecpar->format;
		if (format != AV_SAMPLE_FMT_S16 && format != AV_SAMPLE_FMT_S16P && format != AV_SAMPLE_FMT_S32 &&
			format != AV_SAMPLE_FMT_S32P && format != AV_SAMPLE_FMT_FLT && format != AV_SAMPLE_FMT_FLTP)
			return_code = INVALID_FORMAT;
	}

	if (return_code)
	{
		free_resources(Units);
		return return_code;
	}

	*channel_sample_rate = Units->codec_context->sample_rate;
	return GOOD;
}
int decode_audio(AudioContainerUnits* Units, Channels* AudioChannels, bool is_stereo, int32_t max_sample_rate)
{
	if (is_stereo && Units->channels != 2)
		return NOT_STEREO_ERROR;
	int return_code = extract_audio(Units, AudioChannels, is_stereo, max_sample_rate);

	free_resources(Units);

	if (return_code)
		return return_code;

	return GOOD;
}
