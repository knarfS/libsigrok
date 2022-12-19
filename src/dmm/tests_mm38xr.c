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

START_TEST(test_dmm_mm38xr_parser)
{
	size_t test_idx;
	float floatval;
	struct sr_datafeed_analog analog;
	struct sr_analog_encoding encoding;
	struct sr_analog_meaning meaning;
	struct sr_analog_spec spec;
	int ret;

	struct {
		const char *desc;
		const char *input_buf;
		float tolerance;
		float expected_value;
		int expected_digits;
	} *test_param, test_params[] = {
		/* DCV */
		{
			"999.9 mV",
			"0C123408000A0\r\n",
			.00001,
			.1234,
			4
		},
		{
			"9.999 V",
			"0C123408100A0\r\n",
			.0001,
			1.234,
			3
		},
		{
			"99.99 V",
			"0C123408200A0\r\n",
			.001,
			12.34,
			2
		},
		{
			"999.9 V",
			"0C123408300A0\r\n",
			.01,
			123.4,
			1
		},
		/* Ohm */
		{
			"40 MOhm",
			"08123408000A0\r\n",
			1000,
			12340000,
			-4
		},
		{
			"9.999 MOhm",
			"08123408100A0\r\n",
			100,
			1234000,
			-3
		},
		{
			"999.9 kOhm (1 Mohm)",
			"08123408200A0\r\n",
			10,
			123400,
			-2
		},
		{
			"99.99 kOhm",
			"08123408300A0\r\n",
			1,
			12340,
			-1
		},
		{
			"9.999 kOhm",
			"08123408400A0\r\n",
			.1,
			1234,
			0
		},
		{
			"999.9 Ohm",
			"08123408500A0\r\n",
			.01,
			123.4,
			1
		},
		/* Continuity */
		{
			"40 MOhm, continuity",
			"08123408000A8\r\n",
			.1,
			1.,
			0
		},
		{
			"999.9 Ohm, continuity",
			"08123408500A8\r\n",
			.1,
			1.,
			0
		},
		/* uA */
		{
			"99.99 uA",
			"07123408000A0\r\n",
			.000000001,
			.00001234,
			8
		},
		{
			"999.9 uV",
			"07123408100A0\r\n",
			.00000001,
			.0001234,
			7
		},
		/* mA */
		{
			"9.999 mA",
			"0E123408000A0\r\n",
			.0000001,
			.001234,
			6
		},
		{
			"99.99 mV",
			"0E123408100A0\r\n",
			.000001,
			.01234,
			5
		},
		{
			"399.9 mV",
			"0E123408200A0\r\n",
			.00001,
			.1234,
			4
		},
		/* A */
		{
			"9.999 A",
			"0A123408000A0\r\n",
			.0001,
			1.234,
			3
		},
		/* Hz */
		{
			"99.99 Hz",
			"0F123408000A0\r\n",
			.001,
			12.34,
			2
		},
		{
			"999.9 Hz",
			"0F123408100A0\r\n",
			.01,
			123.4,
			1
		},
		{
			"9.999 kHz",
			"0F123408200A0\r\n",
			.1,
			1234,
			0
		},
		{
			"99.99 kHz",
			"0F123408300A0\r\n",
			1,
			12340,
			-1
		},
		{
			"999.9 kHz",
			"0F123408400A0\r\n",
			10,
			123400,
			-2
		},
		{
			"9.999 MHz",
			"0F123408500A0\r\n",
			100,
			1234000,
			-3
		},
		{
			"39.99 MHz",
			"0F123408600A0\r\n",
			1000,
			12340000,
			-4
		},
		/* Cx */
		{
			"39.99 nF",
			"0B123408000A0\r\n",
			.000000000001,
			.00000001234,
			11
		},
		{
			"399.9 nF",
			"0B123408100A0\r\n",
			.00000000001,
			.0000001234,
			10
		},
		{
			"3.999 uF",
			"0B123408200A0\r\n",
			.0000000001,
			.000001234,
			9
		},
		{
			"39.99 uF",
			"0B123408300A0\r\n",
			.000000001,
			.00001234,
			8
		}
	};

	for (test_idx = 0; test_idx < ARRAY_SIZE(test_params); test_idx++) {
		test_param = &test_params[test_idx];

		sr_analog_init(&analog, &encoding, &meaning, &spec, -1);

		ret = meterman_38xr_parse(
			(const uint8_t *)test_param->input_buf, &floatval, &analog, NULL);

		fail_unless(ret == SR_OK,
			"meterman_38xr_parse() for '%s' failed with %d.",
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

SR_PRIV void register_tests_mm38xr(Suite *s)
{
	TCase *tc;

	tc = tcase_create("mm38xr");
	tcase_add_test(tc, test_dmm_mm38xr_parser);
	suite_add_tcase(s, tc);
}

#endif
