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

START_TEST(test_dmm_fs9922_parser)
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
		const uint8_t input_buf[13];
		float tolerance;
		float expected_value;
		int expected_digits;
	} *test_param, test_params[] = {
		/* 0 decimal places */
		{
			"0 decimal places with mega prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x30, 0x21, 0x00, 0x10, 0x2e, 0x0d, 0x0a},
			100000,
			4635000000.,
			-6
		},
		{
			"0 decimal places with kilo prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x30, 0x21, 0x00, 0x20, 0x2e, 0x0d, 0x0a},
			100,
			4635000.,
			-3
		},
		{
			"0 decimal places with no prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x30, 0x21, 0x00, 0x00, 0x2e, 0x0d, 0x0a},
			.1,
			4635.,
			0
		},
		{
			"0 decimal places with milli prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x30, 0x21, 0x00, 0x40, 0x2e, 0x0d, 0x0a},
			.0001,
			4.635,
			3
		},
		{
			"0 decimal places with micro prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x30, 0x21, 0x00, 0x80, 0x2e, 0x0d, 0x0a},
			.0000001,
			.004635,
			6
		},
		{
			"0 decimal places with nano prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x30, 0x21, 0x02, 0x00, 0x2e, 0x0d, 0x0a},
			.0000000001,
			.000004635,
			9
		},
		/* 1 decimal place */
		{
			"1 decimal place with mega prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x34, 0x21, 0x00, 0x10, 0x2e, 0x0d, 0x0a},
			10000,
			463500000.,
			-5
		},
		{
			"1 decimal place with kilo prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x34, 0x21, 0x00, 0x20, 0x2e, 0x0d, 0x0a},
			10,
			463500.,
			-2
		},
		{
			"1 decimal place with no prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x34, 0x21, 0x00, 0x00, 0x2e, 0x0d, 0x0a},
			.01,
			463.5,
			1
		},
		{
			"1 decimal place with milli prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x34, 0x21, 0x00, 0x40, 0x2e, 0x0d, 0x0a},
			.00001,
			.4635,
			4
		},
		{
			"1 decimal place with micro prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x34, 0x21, 0x00, 0x80, 0x2e, 0x0d, 0x0a},
			.00000001,
			.0004635,
			7
		},
		{
			"1 decimal place with nano prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x34, 0x21, 0x02, 0x00, 0x2e, 0x0d, 0x0a},
			.00000000001,
			.0000004635,
			10
		},
		/* 2 decimal places */
		{
			"2 decimal places with mega prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x32, 0x21, 0x00, 0x10, 0x2e, 0x0d, 0x0a},
			1000,
			46350000.,
			-4
		},
		{
			"2 decimal places with kilo prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x32, 0x21, 0x00, 0x20, 0x2e, 0x0d, 0x0a},
			1,
			46350.,
			-1
		},
		{
			"2 decimal places with no prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x32, 0x21, 0x00, 0x00, 0x2e, 0x0d, 0x0a},
			.001,
			46.35,
			2
		},
		{
			"2 decimal places with milli prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x32, 0x21, 0x00, 0x40, 0x2e, 0x0d, 0x0a},
			.000001,
			.04635,
			5
		},
		{
			"2 decimal places with micro prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x32, 0x21, 0x00, 0x80, 0x2e, 0x0d, 0x0a},
			.000000001,
			.00004635,
			8
		},
		{
			"2 decimal places with nano prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x32, 0x21, 0x02, 0x00, 0x2e, 0x0d, 0x0a},
			.000000000001,
			.00000004635,
			11
		},
		/* 3 decimal places */
		{
			"3 decimal places with mega prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x31, 0x21, 0x00, 0x10, 0x2e, 0x0d, 0x0a},
			100,
			4635000.,
			-3
		},
		{
			"3 decimal places with kilo prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x31, 0x21, 0x00, 0x20, 0x2e, 0x0d, 0x0a},
			.1,
			4635.,
			0
		},
		{
			"3 decimal places with no prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x31, 0x21, 0x00, 0x00, 0x2e, 0x0d, 0x0a},
			.0001,
			4.635,
			3
		},
		{
			"3 decimal places with milli prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x31, 0x21, 0x00, 0x40, 0x2e, 0x0d, 0x0a},
			.0000001,
			.004635,
			6
		},
		{
			"3 decimal places with micro prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x31, 0x21, 0x00, 0x80, 0x2e, 0x0d, 0x0a},
			.0000000001,
			.000004635,
			9
		},
		{
			"3 decimal places with nano prefix",
			{0x2b, 0x34, 0x36, 0x33, 0x35, 0x20, 0x31, 0x21, 0x02, 0x00, 0x2e, 0x0d, 0x0a},
			.0000000000001,
			.000000004635,
			12
		},
	};

	for (test_idx = 0; test_idx < ARRAY_SIZE(test_params); test_idx++) {
		test_param = &test_params[test_idx];

		sr_analog_init(&analog, &encoding, &meaning, &spec, -1);
		memset(&info, 0, sizeof(struct fs9922_info));

		ret = sr_fs9922_parse(
			test_param->input_buf, &floatval, &analog, &info);

		fail_unless(ret == SR_OK,
			"sr_fs9922_parse() for '%s' failed with %d.",
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

SR_PRIV void register_tests_fs9922(Suite *s)
{
	TCase *tc;

	tc = tcase_create("fs9922");
	tcase_add_test(tc, test_dmm_fs9922_parser);
	suite_add_tcase(s, tc);
}

#endif
