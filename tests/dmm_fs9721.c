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

#include <check.h>
#include <libsigrok/libsigrok.h>
#include "lib.h"
#include "libsigrok-internal.h"

START_TEST(test_value_digits)
{
	size_t test_idx;
	float floatval;
	struct sr_datafeed_analog analog;
	struct sr_analog_encoding encoding;
	struct sr_analog_meaning meaning;
	struct sr_analog_spec spec;
	struct fs9922_info info;
	int ret;

	struct {
		const char *desc;
		const uint8_t input_buf[14];
		float tolerance;
		float expected_value;
		int expected_digits;
	} *test_param, test_params[] = {
		/* 3 decimal places */
		{
			"3 decimal places with mega prefix",
			{0x10, 0x22, 0x37, 0x4f, 0x5e, 0x61, 0x7f, 0x83, 0x9e, 0xa0, 0xb2, 0xc0, 0xd0, 0xe0},
			100,
			4635000.,
			3
		},
		{
			"3 decimal places with kilo prefix",
			{0x10, 0x22, 0x37, 0x4f, 0x5e, 0x61, 0x7f, 0x83, 0x9e, 0xa2, 0xb0, 0xc0, 0xd0, 0xe0},
			.1,
			4635.,
			3
		},
		{
			"3 decimal places with no prefix",
			{0x10, 0x22, 0x37, 0x4f, 0x5e, 0x61, 0x7f, 0x83, 0x9e, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0},
			.0001,
			4.635,
			3
		},{
			"3 decimal places with milli prefix",
			{0x10, 0x22, 0x37, 0x4f, 0x5e, 0x61, 0x7f, 0x83, 0x9e, 0xa0, 0xb8, 0xc0, 0xd0, 0xe0},
			.0000001,
			.004635,
			3
		},
		{
			"3 decimal places with micro prefix",
			{0x10, 0x22, 0x37, 0x4f, 0x5e, 0x61, 0x7f, 0x83, 0x9e, 0xa8, 0xb0, 0xc0, 0xd0, 0xe0},
			.0000000001,
			.000004635,
			3
		},
		{
			"3 decimal places with nano prefix",
			{0x10, 0x22, 0x37, 0x4f, 0x5e, 0x61, 0x7f, 0x83, 0x9e, 0xa4, 0xb0, 0xc0, 0xd0, 0xe0},
			.0000000000001,
			.000000004635,
			3
		},
		// /* 2 decimal places */
		{
			"2 decimal places with mega prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x69, 0x7f, 0x83, 0x9e, 0xa0, 0xb2, 0xc0, 0xd0, 0xe0},
			1000,
			46350000.,
			2
		},
		{
			"2 decimal places with kilo prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x69, 0x7f, 0x83, 0x9e, 0xa2, 0xb0, 0xc0, 0xd0, 0xe0},
			1,
			46350.,
			2
		},
		{
			"2 decimal places with no prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x69, 0x7f, 0x83, 0x9e, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0},
			.001,
			46.35,
			2
		},
		{
			"2 decimal places with milli prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x69, 0x7f, 0x83, 0x9e, 0xa0, 0xb8, 0xc0, 0xd0, 0xe0},
			.000001,
			.04635,
			2
		},
		{
			"2 decimal places with micro prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x69, 0x7f, 0x83, 0x9e, 0xa8, 0xb0, 0xc0, 0xd0, 0xe0},
			.000000001,
			.00004635,
			2
		},
		{
			"2 decimal places with nano prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x69, 0x7f, 0x83, 0x9e, 0xa4, 0xb0, 0xc0, 0xd0, 0xe0},
			.000000000001,
			.00000004635,
			2
		},
		/* 1 decimal place */
		{
			"1 decimal place with mega prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x61, 0x7f, 0x8b, 0x9e, 0xa0, 0xb2, 0xc0, 0xd0, 0xe0},
			10000,
			463500000.,
			1
		},
		{
			"1 decimal place with kilo prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x61, 0x7f, 0x8b, 0x9e, 0xa2, 0xb0, 0xc0, 0xd0, 0xe0},
			10,
			463500.,
			1
		},
		{
			"1 decimal place with no prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x61, 0x7f, 0x8b, 0x9e, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0},
			.01,
			463.5,
			1
		},
		{
			"1 decimal place with milli prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x61, 0x7f, 0x8b, 0x9e, 0xa0, 0xb8, 0xc0, 0xd0, 0xe0},
			.00001,
			.4635,
			1
		},
		{
			"1 decimal place with micro prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x61, 0x7f, 0x8b, 0x9e, 0xa8, 0xb0, 0xc0, 0xd0, 0xe0},
			.00000001,
			.0004635,
			1
		},
		{
			"1 decimal place with nano prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x61, 0x7f, 0x8b, 0x9e, 0xa4, 0xb0, 0xc0, 0xd0, 0xe0},
			.00000000001,
			.0000004635,
			1
		},
		/* 0 decimal places */
		{
			"0 decimal places with mega prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x61, 0x7f, 0x83, 0x9e, 0xa0, 0xb2, 0xc0, 0xd0, 0xe0},
			100000,
			4635000000.,
			0
		},
		{
			"0 decimal places with kilo prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x61, 0x7f, 0x83, 0x9e, 0xa2, 0xb0, 0xc0, 0xd0, 0xe0},
			100,
			4635000.,
			0
		},
		{
			"0 decimal places with no prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x61, 0x7f, 0x83, 0x9e, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0},
			.1,
			4635.,
			0
		},
		{
			"0 decimal places with milli prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x61, 0x7f, 0x83, 0x9e, 0xa0, 0xb8, 0xc0, 0xd0, 0xe0},
			.0001,
			4.635,
			0
		},
		{
			"0 decimal places with micro prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x61, 0x7f, 0x83, 0x9e, 0xa8, 0xb0, 0xc0, 0xd0, 0xe0},
			.0000001,
			.004635,
			0
		},
		{
			"0 decimal places with nano prefix",
			{0x10, 0x22, 0x37, 0x47, 0x5e, 0x61, 0x7f, 0x83, 0x9e, 0xa4, 0xb0, 0xc0, 0xd0, 0xe0},
			.0000000001,
			.000004635,
			0
		}
	};

	for (test_idx = 0; test_idx < ARRAY_SIZE(test_params); test_idx++) {
		test_param = &test_params[test_idx];

		srtest_analog_init(&analog, &encoding, &meaning, &spec, -1);
		memset(&info, 0, sizeof(struct fs9922_info));

		ret = sr_fs9721_parse(
			test_param->input_buf, &floatval, &analog, &info);

		fail_unless(ret == SR_OK, "sr_fs9721_parse() failed.");
		ck_assert_float_eq_tol(floatval,
			test_param->expected_value, test_param->tolerance);
		ck_assert_int_eq(
			analog.encoding->digits, test_param->expected_digits);
		ck_assert_int_eq(
			analog.spec->spec_digits, test_param->expected_digits);
	}
}
END_TEST

Suite *suite_dmm_fs9721(void)
{
	Suite *s;
	TCase *tc;

	s = suite_create("dmm_fs9721");

	tc = tcase_create("sr_fs9721_parse");
	tcase_add_test(tc, test_value_digits);
	suite_add_tcase(s, tc);

	return s;
}
