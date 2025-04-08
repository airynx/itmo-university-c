#include "return_codes.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

bool is_zero(uint32_t exponent, uint64_t mantissa)
{
	return exponent == 0x0 && mantissa == 0x0;
}
bool is_inf(uint32_t exponent, uint64_t mantissa)
{
	return exponent == 0xFF && mantissa == 0x0;
}
bool is_nan(uint32_t exponent, uint64_t mantissa)
{
	return exponent == 0xFF && mantissa != 0x0;
}
int print_zero_special(int32_t sign, uint64_t mantissa, int32_t exponent, bool is_single)
{
	if (!is_zero(exponent, mantissa))
		return 0;
	char leading_zeroes;
	if (!is_single)
		leading_zeroes = 3;
	else
		leading_zeroes = 6;
	if (sign)
		printf("-");
	printf("0x0.%0*xp%+d\n", leading_zeroes, 0, 0);
	return 1;
}
char print_nan_special(uint64_t mantissa, int32_t exponent)
{
	if (!is_nan(exponent, mantissa))
		return 0;
	printf("nan\n");
	return 1;
}
char print_inf_special(uint32_t sign, uint64_t mantissa, int32_t exponent)
{
	if (!is_inf(exponent, mantissa))
		return 0;
	if (sign)
		printf("-");
	printf("inf\n");
	return 1;
}

void round_mantissa(uint32_t sign, uint64_t *mantissa, uint32_t lost_bits, uint16_t count_lost_bits, bool overflow_last, char round_mode)
{
	if (!lost_bits)
		return;
	switch (round_mode)
	{
		{
		case '1':
		{
			if (lost_bits & ((uint64_t)1 << (count_lost_bits - 1)))
			{
				if ((lost_bits & ~((uint64_t)1 << (count_lost_bits - 1))) || overflow_last)
					(*mantissa)++;
			}
			break;
		}
		case '2':
		{
			if (!sign)
				(*mantissa)++;
			break;
		}
		case '3':
		{
			if (sign)
				(*mantissa)++;
			break;
		}
		}
	}
}
uint64_t normalize_mantissa(uint64_t mantissa, int32_t *exponent, bool is_single, bool *overflow_bit)
{
	uint16_t bias = is_single ? 23 : 10;
	while (mantissa < (1ULL << bias))
	{
		mantissa <<= 1;
		(*exponent)--;
	}
	if (mantissa >= (1ULL << (bias + 1)))
	{
		mantissa >>= 1;
		(*exponent)++;
		*overflow_bit = true;
	}
	return mantissa;
}
void normalize_division(int32_t *result_exponent, uint64_t *result_mantissa, uint16_t *remaining_bits, uint64_t mantissa2, uint64_t normalize_shift)
{
	while (*result_exponent > 0 && normalize_shift > *result_mantissa)
	{
		(*remaining_bits) <<= 1;
		(*result_mantissa) <<= 1;
		(*result_exponent)--;
		if ((*remaining_bits) >= mantissa2)
		{
			(*result_mantissa)++;
			(*remaining_bits) -= mantissa2;
		}
	}
}
int check_special(int32_t sign, int32_t exp, int64_t mantissa, bool is_single)
{
	return print_zero_special(sign, mantissa, exp, is_single) || print_inf_special(sign, mantissa, exp) ||
		   print_nan_special(mantissa, exp);
}
int32_t find_exp(int32_t num, int16_t mantissa_size, uint32_t exponent_mask)
{
	return (num >> mantissa_size) & exponent_mask;
}
int32_t find_sign(int32_t num, int16_t sign_shift)
{
	return (num >> sign_shift) & 1;
}
uint64_t find_mantissa(int32_t num, uint32_t mantissa_mask)
{
	return num & mantissa_mask;
}
typedef struct
{
	int16_t sign_shift;
	int16_t mantissa_size;
	uint32_t mantissa_mask;
	uint32_t exponent_mask;
	uint8_t leading_zeroes;
	int32_t bias;
	uint32_t hidden_one_mask;
	uint32_t count_lost_bits;
	uint64_t lost_bits_mask;
	uint64_t lost_bits;
} NumberValues;
NumberValues initSingleValues()
{
	NumberValues values = {
		.sign_shift = 31,
		.mantissa_size = 23,
		.mantissa_mask = 0b00000000011111111111111111111111,
		.exponent_mask = 0b00000000000000000000000011111111,
		.leading_zeroes = 6,
		.bias = 127,
		.hidden_one_mask = 0b00000000100000000000000000000000
	};
	return values;
}
NumberValues initHalfValues()
{
	NumberValues values = {
		.sign_shift = 15,
		.mantissa_size = 10,
		.mantissa_mask = 0b0000001111111111,
		.exponent_mask = 0b0000000000011111,
		.leading_zeroes = 3,
		.bias = 15,
		.hidden_one_mask = 0b0000010000000000
	};
	return values;
}
void print_num(int32_t num, char round_mode, bool is_single, NumberValues *numberValues)
{
	int32_t sign = find_sign(num, numberValues->sign_shift);
	int32_t exp = find_exp(num, numberValues->mantissa_size, numberValues->exponent_mask);
	uint32_t mantissa = find_mantissa(num, numberValues->mantissa_mask);
	if (check_special(sign, exp, mantissa, is_single))
		return;
	if (round_mode != 'n')
	{
		if (exp)
			mantissa |= numberValues->hidden_one_mask;
		else
			exp++;
		bool is_overflow;
		mantissa = normalize_mantissa(mantissa, &exp, is_single, &is_overflow);
		mantissa &= ~(numberValues->hidden_one_mask);
	}
	if (print_zero_special(sign, mantissa, exp, is_single) || print_nan_special(mantissa, exp) ||
		print_zero_special(sign, mantissa, exp, is_single))
		return;
	mantissa <<= is_single ? 1 : 2;
	if (sign)
		printf("-");
	printf("0x1.%0*xp%+d\n", numberValues->leading_zeroes, mantissa, exp - numberValues->bias);
}

void sum_subtract(char format, int32_t num1, int32_t num2, char round_mode, bool is_sum, NumberValues *numberValues)
{
	int32_t min_exp = 1 - numberValues->bias;
	int32_t sign1 = find_sign(num1, numberValues->sign_shift);
	int32_t sign2 = find_sign(num2, numberValues->sign_shift);
	uint64_t mantissa1 = find_mantissa(num1, numberValues->mantissa_mask);
	uint64_t mantissa2 = find_mantissa(num2, numberValues->mantissa_mask);
	int32_t exp1 = find_exp(num1, numberValues->mantissa_size, numberValues->exponent_mask);
	int32_t exp2 = find_exp(num2, numberValues->mantissa_size, numberValues->exponent_mask);

	if (print_nan_special(mantissa1, exp1) || print_nan_special(mantissa1, exp1))
		return;
	if (is_zero(exp1, mantissa1))
	{
		if (!print_zero_special(0, mantissa1, exp1, format == 'f'))
			print_num(num2, round_mode, format == 'f', numberValues);
		return;
	}
	else if (is_zero(exp2, mantissa2))
	{
		print_num(num1, round_mode, format == 'f', numberValues);
		return;
	}
	if (print_inf_special(sign1, mantissa1, exp1) || print_inf_special(sign2, mantissa2, exp2))
		return;
	mantissa1 |= numberValues->hidden_one_mask;
	mantissa2 |= numberValues->hidden_one_mask;
	exp1 -= numberValues->bias;
	exp2 -= numberValues->bias;
	int32_t max_exp;
	int32_t sign_of_sum;
	uint64_t counted_mantissa;

	if (!is_sum)
		sign2 ^= 1;

	numberValues->count_lost_bits = exp1 < exp2 ? exp2 - exp1 : exp1 - exp2;
	max_exp = exp1 < exp2 ? exp2 : exp1;
	numberValues->lost_bits_mask = ((uint64_t)1 << (numberValues->count_lost_bits)) - 1;
	numberValues->lost_bits = (exp1 < exp2 ? mantissa1 : mantissa2) & numberValues->lost_bits_mask;
	if (exp1 < exp2)
		mantissa1 >>= (exp2 - exp1);
	else
		mantissa2 >>= (exp1 - exp2);
	if (sign1 == sign2)
	{
		sign_of_sum = sign1;
		counted_mantissa = mantissa1 + mantissa2;
	}
	else if (mantissa1 < mantissa2)
	{
		sign_of_sum = sign2;
		counted_mantissa = mantissa2 - mantissa1;
	}
	else
	{
		sign_of_sum = sign1;
		counted_mantissa = mantissa1 - mantissa2;
	}
	bool overflow_last = false;
	counted_mantissa = normalize_mantissa(counted_mantissa, &max_exp, format == 'f', &overflow_last);
	round_mantissa(sign_of_sum, &counted_mantissa, numberValues->lost_bits, numberValues->count_lost_bits, overflow_last, round_mode);
	if (max_exp >= numberValues->bias)
		print_inf_special(sign_of_sum, 0x0, 0xff);
	else if (max_exp + numberValues->bias <= min_exp)
		print_zero_special(sign_of_sum, 0x0, 0x0, format == 'f');
	else
		print_num(
			(sign_of_sum << numberValues->sign_shift) | ((max_exp + numberValues->bias) << numberValues->mantissa_size) |
				(counted_mantissa & numberValues->mantissa_mask),
			'n',
			format == 'f',
			numberValues);
}

void multiply_divide(char format, int32_t num1, int32_t num2, char round_mode, bool is_multiply, NumberValues *numberValues)
{
	int32_t sign1 = find_sign(num1, numberValues->sign_shift);
	int32_t sign2 = find_sign(num2, numberValues->sign_shift);
	uint64_t mantissa1 = find_mantissa(num1, numberValues->mantissa_mask);
	uint64_t mantissa2 = find_mantissa(num2, numberValues->mantissa_mask);
	int32_t exp1 = find_exp(num1, numberValues->mantissa_size, numberValues->exponent_mask);
	int32_t exp2 = find_exp(num2, numberValues->mantissa_size, numberValues->exponent_mask);
	int32_t result_sign = sign1 ^ sign2;
	if (print_nan_special(mantissa1, exp1), print_nan_special(mantissa2, exp2))
		return;
	if (!is_multiply)
	{
		if ((is_zero(exp1, mantissa1) && is_zero(exp2, mantissa2)) || is_nan(exp1, mantissa1) ||
			is_nan(exp2, mantissa2) || ((is_inf(exp1, mantissa1) && is_inf(exp2, mantissa2))))
		{
			printf("nan");
			return;
		}
		if (is_zero(exp2, mantissa2) || (is_inf(exp1, mantissa1)))
		{
			if (result_sign)
				printf("-");
			printf("inf\n");
			return;
		}
		if (is_zero(exp1, mantissa1))
		{
			if (is_inf(exp2, mantissa2))
				print_zero_special(result_sign, 0x0, 0x0, format == 'f');
			return;
		}
	}
	else if (is_nan(exp1, mantissa1) || is_nan(exp2, mantissa2) || (is_zero(exp1, mantissa1) && is_inf(exp2, mantissa2)) ||
			 (is_inf(exp1, mantissa1) && is_zero(exp2, mantissa2)))
	{
		printf("nan");
		return;
	}
	else if ((is_zero(exp1, mantissa1) && is_zero(exp2, mantissa2)))
	{
		print_zero_special(result_sign, 0x0, 0x0, format == 'f');
		return;
	}
	else if (is_inf(exp1, mantissa1) && is_inf(exp2, mantissa2))
	{
		if (result_sign)
			printf("-");
		printf("inf\n");
		return;
	}
	mantissa1 |= numberValues->hidden_one_mask;
	mantissa2 |= numberValues->hidden_one_mask;
	exp1 -= numberValues->bias;
	exp2 -= numberValues->bias;
	int32_t result_exponent = is_multiply ? exp1 + exp2 : exp1 - exp2;
	uint16_t remaining_bits;
	uint64_t shifted_mantissa;
	if (!is_multiply)
	{
		shifted_mantissa = (uint64_t)mantissa1 << (numberValues->mantissa_size);
		remaining_bits = shifted_mantissa % mantissa2;
	}
	uint64_t result_mantissa = is_multiply ? (uint64_t)mantissa1 * (uint64_t)mantissa2 : shifted_mantissa / mantissa2;
	bool was_overflow_last_one = false;
	if (is_multiply)
	{
		numberValues->count_lost_bits = numberValues->mantissa_size + 1;
		numberValues->lost_bits = result_mantissa & (format == 'f' ? 0xffffff : 0x7ff);
		result_mantissa >>= numberValues->count_lost_bits;
	}
	else
	{
		normalize_division(&result_exponent, &result_mantissa, &remaining_bits, mantissa2, 1ULL << numberValues->mantissa_size);
		while (result_mantissa >= (1ULL << numberValues->mantissa_size + 1))
		{
			result_mantissa >>= 1;
			result_exponent++;
		}
		result_exponent--;
	}
	result_exponent++;
	result_mantissa = normalize_mantissa(result_mantissa, &result_exponent, format == 'f', &was_overflow_last_one);
	round_mantissa(result_sign, &result_mantissa, numberValues->lost_bits, numberValues->count_lost_bits, was_overflow_last_one, round_mode);
	print_num(
		(result_sign << numberValues->sign_shift) | ((result_exponent + numberValues->bias) << numberValues->mantissa_size) |
			(result_mantissa & numberValues->mantissa_mask),
		'n',
		format == 'f',
		numberValues);
}
bool valid_round_mode(char round_mode)
{
	return '0' <= round_mode && round_mode <= '3';
}
bool valid_format(char format)
{
	return (format == 'h' || format == 'f');
}
bool arguments_check(int argc)
{
	return argc == 4 || argc == 6;
}
bool is_scanf_wrong(uint8_t count_scanf, char *arg)
{
	if (!count_scanf)
		fprintf(stderr, "Input number is incorrect %s\n", arg);
	return count_scanf == 0;
}
int main(int argc, char *argv[])
{
	if (!arguments_check(argc))
	{
		fprintf(stderr, "Wrong arguments. Usage:\n<format> <rounding_code> <number>\n<format> <rounding_code> <number1> <operation> <number2>\n");
		return ERROR_ARGUMENTS_INVALID;
	}
	char format = argv[1][0];
	char round_mode = argv[2][0];
	if (argv[1][1] || !valid_format(format))
	{
		fprintf(stderr, "This type of format is unsupported: %s\n", argv[1]);
		return ERROR_ARGUMENTS_INVALID;
	}
	if (argv[2][1] || !valid_round_mode(argv[2][0]))
	{
		fprintf(stderr, "This rounding code is unsupported: %s\n", argv[2]);
		return ERROR_ARGUMENTS_INVALID;
	}
	int32_t num1;
	uint8_t count_scanf1 = sscanf(argv[3], "%x", &num1);
	if (is_scanf_wrong(count_scanf1, argv[3]))
		return ERROR_ARGUMENTS_INVALID;
	NumberValues numberValues = format == 'f' ? initSingleValues() : initHalfValues();
	switch (argc)
	{
	case 4:
	{
		print_num(num1, format, format == 'f', &numberValues);
		break;
	}
	case 6:
	{
		int32_t num2;
		uint8_t count_scanf2 = sscanf(argv[5], "%x", &num2);
		if (is_scanf_wrong(count_scanf2, argv[5]))
			return ERROR_ARGUMENTS_INVALID;
		switch (argv[4][0])
		{
		case '+':
			sum_subtract(format, num1, num2, round_mode, true, &numberValues);
			break;
		case '-':
			sum_subtract(format, num1, num2, round_mode, false, &numberValues);
			break;
		case '*':
			multiply_divide(format, num1, num2, round_mode, true, &numberValues);
			break;
		case '/':
			multiply_divide(format, num1, num2, round_mode, false, &numberValues);
			break;
		default:
			fprintf(stderr, "This operation is not supported: %s\n", argv[4]);
			return ERROR_ARGUMENTS_INVALID;
		}
	}
	}
	return SUCCESS;
}