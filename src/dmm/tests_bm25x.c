/*
 * This file is part of the libsigrok project.
 *
 * Copyright (C) 2022 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#if defined HAVE_CHECK && HAVE_CHECK

#include <check.h>
#include <libsigrok/libsigrok.h>
#include "libsigrok-internal.h"

START_TEST(test_dmm_bm25x_parser)
{
	size_t test_idx;
	float floatval;
	struct sr_datafeed_analog analog;
	struct sr_analog_encoding encoding;
	struct sr_analog_meaning meaning;
	struct sr_analog_spec spec;
	struct bm25x_info info;
	int ret;

	struct {
		const char *desc;
		const uint8_t input_buf[15];
		float tolerance;
		float expected_value;
		int expected_digits;
	} *test_param, test_params[] = {
		/* 0 decimal places */
		{
			"0 decimal places with mega prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x50, 0x6a, 0x78, 0x8f, 0x9e, 0xa7, 0xb2, 0xc0, 0xd0, 0xe5},
			100000,
			5136000000.,
			-6
		},
		{
			"0 decimal places with kilo prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x50, 0x6a, 0x78, 0x8f, 0x9e, 0xa7, 0xb1, 0xc0, 0xd0, 0xe5},
			100,
			5136000.,
			-3
		},
		{
			"0 decimal places with no prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x50, 0x6a, 0x78, 0x8f, 0x9e, 0xa7, 0xb0, 0xc0, 0xd0, 0xe5},
			.1,
			5136.,
			0
		},
		{
			"0 decimal places with milli prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x50, 0x6a, 0x78, 0x8f, 0x9e, 0xa7, 0xb0, 0xc0, 0xd1, 0xe5},
			.0001,
			5.136,
			3
		},
		{
			"0 decimal places with micro prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x50, 0x6a, 0x78, 0x8f, 0x9e, 0xa7, 0xb0, 0xc0, 0xd2, 0xe5},
			.0000001,
			.005136,
			6
		},
		{
			"0 decimal places with nano prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x50, 0x6a, 0x78, 0x8f, 0x9e, 0xa7, 0xb0, 0xc1, 0xd0, 0xe5},
			.0000000001,
			.000005136,
			9
		},
		/* 1 decimal place */
		{
			"1 decimal place with mega prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x50, 0x6a, 0x78, 0x8f, 0x9f, 0xa7, 0xb2, 0xc0, 0xd0, 0xe5},
			10000.,
			513600000.,
			-5
		},
		{
			"1 decimal place with kilo prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x50, 0x6a, 0x78, 0x8f, 0x9f, 0xa7, 0xb1, 0xc0, 0xd0, 0xe5},
			10.,
			513600.,
			-2
		},
		// TODO
		/* 2 decimal places */
		{
			"2 decimal places with mega prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x50, 0x6a, 0x79, 0x8f, 0x9e, 0xa7, 0xb2, 0xc0, 0xd0, 0xe5},
			1000.,
			51360000.,
			-4
		},
		{
			"2 decimal places with kilo prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x50, 0x6a, 0x79, 0x8f, 0x9e, 0xa7, 0xb1, 0xc0, 0xd0, 0xe5},
			1.,
			51360.,
			-1
		},
		{
			"2 decimal places with no prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x50, 0x6a, 0x79, 0x8f, 0x9e, 0xa7, 0xb0, 0xc0, 0xd0, 0xe5},
			.001,
			51.36,
			2
		},
		{
			"2 decimal places with milli prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x50, 0x6a, 0x79, 0x8f, 0x9e, 0xa7, 0xb0, 0xc0, 0xd1, 0xe5},
			.000001,
			.05136,
			5
		},
		{
			"2 decimal places with micro prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x50, 0x6a, 0x79, 0x8f, 0x9e, 0xa7, 0xb0, 0xc0, 0xd2, 0xe5},
			.000000001,
			.00005136,
			8
		},
		{
			"2 decimal places with nano prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x50, 0x6a, 0x79, 0x8f, 0x9e, 0xa7, 0xb0, 0xc1, 0xd0, 0xe5},
			.000000000001,
			.00000005136,
			11
		},
		/* 3 decimal places */
		{
			"3 decimal places with mega prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x51, 0x6a, 0x78, 0x8f, 0x9e, 0xa7, 0xb2, 0xc0, 0xd0, 0xe5},
			100,
			5136000.,
			-3
		},
		{
			"3 decimal places with kilo prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x51, 0x6a, 0x78, 0x8f, 0x9e, 0xa7, 0xb1, 0xc0, 0xd0, 0xe5},
			.1,
			5136.,
			0
		},
		{
			"3 decimal places with no prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x51, 0x6a, 0x78, 0x8f, 0x9e, 0xa7, 0xb0, 0xc0, 0xd0, 0xe5},
			.0001,
			5.136,
			3
		},
		{
			"3 decimal places with milli prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x51, 0x6a, 0x78, 0x8f, 0x9e, 0xa7, 0xb0, 0xc0, 0xd1, 0xe5},
			.0000001,
			.005136,
			6
		},
		{
			"3 decimal places with micro prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x51, 0x6a, 0x78, 0x8f, 0x9e, 0xa7, 0xb0, 0xc0, 0xd2, 0xe5},
			.0000000001,
			.000005136,
			9
		},
		{
			"3 decimal places with nano prefix",
			{0x02, 0x1a, 0x20, 0x3c, 0x47, 0x51, 0x6a, 0x78, 0x8f, 0x9e, 0xa7, 0xb0, 0xc1, 0xd0, 0xe5},
			.0000000000001,
			.000000005136,
			12
		},
	};

	for (test_idx = 0; test_idx < ARRAY_SIZE(test_params); test_idx++) {
		test_param = &test_params[test_idx];

		sr_analog_init(&analog, &encoding, &meaning, &spec, -1);
		memset(&info, 0, sizeof(struct bm25x_info));

		ret = sr_brymen_bm25x_parse(
			test_param->input_buf, &floatval, &analog, &info);

		fail_unless(ret == SR_OK,
			"sr_brymen_bm25x_parse() for '%s' failed with %d.",
			test_param->desc, ret);
		fail_unless(sr_check_float_eq_tol(floatval,
				test_param->expected_value,
				test_param->tolerance),
			"For '%s' expected value %f does not match %f",
			test_param->desc, test_param->expected_value, floatval);
		fail_unless(
			analog.encoding->digits == test_param->expected_digits,
			"For '%s' expected encoding.digits %d does not match %d",
			test_param->desc, test_param->expected_digits,
			analog.encoding->digits);
		fail_unless(
			analog.spec->spec_digits == test_param->expected_digits,
			"For '%s' expected spec.spec_digits %d does not match %d",
			test_param->desc, test_param->expected_digits,
			analog.spec->spec_digits);
	}
}
END_TEST

SR_PRIV void register_tests_bm25x(Suite *s)
{
	TCase *tc;

	tc = tcase_create("bm25x");
	tcase_add_test(tc, test_dmm_bm25x_parser);
	suite_add_tcase(s, tc);
}

#endif
