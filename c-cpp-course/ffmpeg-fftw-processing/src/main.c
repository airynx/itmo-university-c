#include "decode.h"
#include "return_codes.h"
#include "xcorr.h"

#include <stdio.h>

static void print_result(int32_t delta, int32_t sample_rate, int32_t delta_time)
{
	printf("delta: %i samples\nsample rate: %i Hz\ndelta time: %i ms\n", delta, sample_rate, delta_time);
}

static void merge_in_channels(Channels* channel_to_merge, Channels* merge_channel)
{
	merge_channel->channels[1] = channel_to_merge->channels[0];
	merge_channel->capacities[1] = channel_to_merge->capacities[0];
	merge_channel->sample_rate = channel_to_merge->sample_rate;
}
static int check_error(int decode_error)
{
	switch (decode_error)
	{
	case NOT_STEREO_ERROR:
		fputs("Your file is not stereo, exiting...\n", stderr);
		return ERROR_FORMAT_INVALID;
	case CANNOT_OPEN_FILE_ERROR:
		fputs("Your file can not be opened.\n", stderr);
		return ERROR_CANNOT_OPEN_FILE;
	case ALLOC_ERROR:
		fputs("Could not allocate enough memory, exiting...\n", stderr);
		return ERROR_NOTENOUGH_MEMORY;
	case NO_AUDIO_STREAM:
		fputs("Your file format is invalid, there are no audiochannels in there.\n", stderr);
		return ERROR_FORMAT_INVALID;
	case INVALID_FORMAT:
		fputs("Current format is invalid, can not process this one. Program works with FLAC, MP2, MP3, Opus, AAC "
			  "extensions.\n",
			  stderr);
		return ERROR_FORMAT_INVALID;
	default:
		fputs("Your data is invalid, maybe the file is corrupted.\n", stderr);
		return ERROR_DATA_INVALID;
	}
}
int main(int argc, char* argv[])
{
	av_log_set_level(AV_LOG_QUIET);

	////
	//  decoding audio
	if (argc != 2 && argc != 3)
	{
		fputs("Usage:\n <file>\n <file1> <file2>\n", stderr);
		return ERROR_ARGUMENTS_INVALID;
	}

	Channels* channels = init_channels();
	AudioContainerUnits Units = get_units();
	int32_t max_sample_rate;
	int return_code;

	return_code = prepare_audio_container(argv[1], &Units, &channels->sample_rate);
	if (return_code != SUCCESS)
		return check_error(return_code);

	if (argc == 2)
	{
		max_sample_rate = channels->sample_rate;
		return_code = decode_audio(&Units, channels, true, max_sample_rate);
		if (return_code != SUCCESS)
		{
			free_channels(channels);
			return check_error(return_code);
		}
	}
	else
	{
		Channels* channels2 = init_channels();
		AudioContainerUnits Units2 = get_units();

		return_code = prepare_audio_container(argv[2], &Units2, &channels2->sample_rate);
		if (return_code != SUCCESS)
			return check_error(return_code);

		max_sample_rate = channels->sample_rate > channels2->sample_rate ? channels->sample_rate : channels2->sample_rate;
		return_code = decode_audio(&Units, channels, false, max_sample_rate);
		if (return_code != SUCCESS)
		{
			free_channels(channels);
			return check_error(return_code);
		}

		return_code = decode_audio(&Units2, channels2, false, max_sample_rate);
		if (return_code != SUCCESS)
		{
			free_channels(channels2);
			return check_error(return_code);
		}
		merge_in_channels(channels2, channels);
	}
	////
	// x-corr
	int32_t corr_sample_rate = x_corr(channels, &return_code);

	if (return_code)
	{
		free_channels(channels);
		return check_error(return_code);
	}

	////
	// output
	print_result(corr_sample_rate, max_sample_rate, corr_sample_rate * 1000 / max_sample_rate);

	////
	// cleaning up
	free_channels(channels);
	return SUCCESS;
}
