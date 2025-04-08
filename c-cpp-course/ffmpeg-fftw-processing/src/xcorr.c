#include "decode.h"

#include <fftw3.h>

static size_t find_sample_delay(const double* backward_xcorr, int32_t n)
{
	size_t max_ind = 0;
	for (size_t i = 1; i < n; i++)
	{
		if (backward_xcorr[i] > backward_xcorr[max_ind])
			max_ind = i;
	}
	return max_ind;
}
static void complex_conjugate(double* complex_number, double re1, double re2, double im1, double im2)
{
	complex_number[0] = re1 * re2 + im1 * im2;
	complex_number[1] = im1 * re2 - re1 * im2;
}
static void destroy_fftw_plans(fftw_plan plan_to_destroy1, fftw_plan plan_to_destroy2, fftw_plan out_plan)
{
	if (plan_to_destroy1)
		fftw_destroy_plan(plan_to_destroy1);
	if (plan_to_destroy2)
		fftw_destroy_plan(plan_to_destroy2);
	if (out_plan)
		fftw_destroy_plan(out_plan);
}
int32_t x_corr(Channels* AudioChannels, int* return_code)
{
	int32_t n =
		AudioChannels->capacities[0] > AudioChannels->capacities[1] ? AudioChannels->capacities[0] : AudioChannels->capacities[1];	  // ?? size_t but n is int32_t
	double* real_signals = fftw_alloc_real(sizeof(double) * 3 * n);
	double* ext_signal1 = real_signals;
	double* ext_signal2 = real_signals + n;
	double* backward_xcorr = real_signals + 2 * n;
	if (!ext_signal1 || !ext_signal2 || !backward_xcorr)
	{
		fftw_free(real_signals);
		*return_code = ALLOC_ERROR;
		return ALLOC_ERROR;
	}

	fftw_complex* complex_signals = fftw_alloc_complex(sizeof(fftw_complex) * 3 * n);
	fftw_complex* forward_out1 = complex_signals;
	fftw_complex* forward_out2 = complex_signals + n;
	fftw_complex* xcorr = complex_signals + 2 * n;
	if (!forward_out1 || !forward_out2 || !xcorr)
	{
		fftw_free(real_signals);
		fftw_free(complex_signals);
		*return_code = ALLOC_ERROR;
		return ALLOC_ERROR;
	}

	memcpy(ext_signal1, AudioChannels->channels[0], AudioChannels->capacities[0] * sizeof(double));
	memset(ext_signal1 + AudioChannels->capacities[0], 0, (n - AudioChannels->capacities[0]) * sizeof(double));

	memcpy(ext_signal2, AudioChannels->channels[1], AudioChannels->capacities[1] * sizeof(double));
	memset(ext_signal2 + AudioChannels->capacities[1], 0, (n - AudioChannels->capacities[1]) * sizeof(double));

	fftw_plan forward_plan1 = fftw_plan_dft_r2c_1d(n, ext_signal1, forward_out1, FFTW_ESTIMATE);
	fftw_plan forward_plan2 = fftw_plan_dft_r2c_1d(n, ext_signal2, forward_out2, FFTW_ESTIMATE);
	fftw_plan backward_plan = fftw_plan_dft_c2r_1d(n, xcorr, backward_xcorr, FFTW_ESTIMATE);
	if (!forward_plan1 || !forward_plan2 || !backward_plan)
	{
		fftw_free(real_signals);
		fftw_free(complex_signals);
		destroy_fftw_plans(forward_plan1, forward_plan2, backward_plan);
		*return_code = ALLOC_ERROR;
		return ALLOC_ERROR;
	}

	fftw_execute(forward_plan1);
	fftw_execute(forward_plan2);

	for (size_t i = 0; i < n; i++)
		complex_conjugate(xcorr[i], forward_out1[i][0], forward_out2[i][0], forward_out1[i][1], forward_out2[i][1]);

	fftw_execute(backward_plan);

	size_t not_centred_delay = find_sample_delay(backward_xcorr, n);
	int32_t sample_delay = (not_centred_delay > n / 2) ? not_centred_delay - n : not_centred_delay;

	fftw_free(real_signals);
	fftw_free(complex_signals);
	destroy_fftw_plans(forward_plan1, forward_plan2, backward_plan);

	return sample_delay;
}
