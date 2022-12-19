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

START_TEST(test_dmm_metex14_parser)
{
	size_t test_idx;
	float floatval;
	struct sr_datafeed_analog analog;
	struct sr_analog_encoding encoding;
	struct sr_analog_meaning meaning;
	struct sr_analog_spec spec;
	struct metex14_info info;
	int ret;

	struct {
		const char *desc;
		const char *input_buf;
		float tolerance;
		float expected_value;
		int expected_digits;
	} *test_param, test_params[] = {
		{
			"1234 MOhm",
			"OH  1234 MOhm\r",
			100000.,
			1234000000.,
			-6
		},
		{
			"123.4 MOhm",
			"OH  123.4MOhm\r",
			10000.,
			123400000.,
			-5
		},
		{
			"12.34 MOhm",
			"OH  12.34MOhm\r",
			1000.,
			12340000.,
			-4
		},
		{
			"1.234 MOhm",
			"OH  1.234MOhm\r",
			100.,
			1234000.,
			-3
		},

		{
			"1234 kOhm",
			"OH  1234 kOhm\r",
			100.,
			1234000.,
			-3
		},
		{
			"123.4 kOhm",
			"OH  123.4kOhm\r",
			10.,
			123400.,
			-2
		},
		{
			"12.34 kOhm",
			"OH  12.34kOhm\r",
			1.,
			12340.,
			-1
		},
		{
			"1.234 kOhm",
			"OH  1.234kOhm\r",
			.1,
			1234.,
			0
		},

		{
			"1234 Ohm",
			"OH  1234  Ohm\r",
			.1,
			1234.,
			0
		},
		{
			"123.4 Ohm",
			"OH  123.4 Ohm\r",
			.01,
			123.4,
			1
		},
		{
			"12.34 Ohm",
			"OH  12.34 Ohm\r",
			.001,
			12.34,
			2
		},
		{
			"1.234 Ohm",
			"OH  1.234 Ohm\r",
			.0001,
			1.234,
			3
		},

		{
			"1234 mV",
			"DC  1234   mV\r",
			.0001,
			1.234,
			3
		},
		{
			"123.4 mV",
			"DC  123.4  mV\r",
			.00001,
			.1234,
			4
		},
		{
			"12.34 mV",
			"DC  12.34  mV\r",
			.000001,
			.01234,
			5
		},
		{
			"1.234 mV",
			"DC  1.234  mV\r",
			.0000001,
			.001234,
			6
		},

		{
			"1234 uF",
			"CA  1234   uF\r",
			.0000001,
			.001234,
			6
		},
		{
			"123.4 uF",
			"CA  123.4  uF\r",
			.00000001,
			.0001234,
			7
		},
		{
			"12.34 uF",
			"CA  12.34  uF\r",
			.000000001,
			.00001234,
			8
		},
		{
			"1.234 uF",
			"CA  1.234  uF\r",
			.0000000001,
			.000001234,
			9
		},

		{
			"1234 nF",
			"CA  1234   nF\r",
			.0000000001,
			.000001234,
			9
		},
		{
			"123.4 nF",
			"CA  123.4  nF\r",
			.00000000001,
			.0000001234,
			10
		},
		{
			"12.34 nF",
			"CA  12.34  nF\r",
			.000000000001,
			.00000001234,
			11
		},
		{
			"1.234 nF",
			"CA  1.234  nF\r",
			.0000000000001,
			.000000001234,
			12
		},

		{
			"1234 pF",
			"CA  1234   pF\r",
			.0000000000001,
			.000000001234,
			12
		},
		{
			"123.4 pF",
			"CA  123.4  pF\r",
			.00000000000001,
			.0000000001234,
			13
		},
		{
			"12.34 pF",
			"CA  12.34  pF\r",
			.000000000000001,
			.00000000001234,
			14
		},
		{
			"1.234 pF",
			"CA  1.234  pF\r",
			.0000000000000001,
			.000000000001234,
			15
		},
	};

	for (test_idx = 0; test_idx < ARRAY_SIZE(test_params); test_idx++) {
		test_param = &test_params[test_idx];

		sr_analog_init(&analog, &encoding, &meaning, &spec, -1);
		memset(&info, 0, sizeof(struct metex14_info));

		ret = sr_metex14_parse(
			(const uint8_t *)test_param->input_buf, &floatval, &analog, &info);

		fail_unless(ret == SR_OK,
			"sr_metex14_parse() for '%s' failed with %d.",
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

SR_PRIV void register_tests_metex14(Suite *s)
{
	TCase *tc;

	tc = tcase_create("metex14");
	tcase_add_test(tc, test_dmm_metex14_parser);
	suite_add_tcase(s, tc);
}

#endif
