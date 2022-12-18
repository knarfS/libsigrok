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
	struct asycii_info info;
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
			1
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
			2
		},
		{
			"9.999 MOhm",
			"08123408100A0\r\n",
			100,
			1234000,
			3
		},
		{
			"999.9 kOhm (1 Mohm)",
			"08123408200A0\r\n",
			10,
			123400,
			1
		},
		{
			"99.99 kOhm",
			"08123408300A0\r\n",
			1,
			12340,
			2
		},
		{
			"9.999 kOhm",
			"08123408400A0\r\n",
			.1,
			1234,
			3
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
			2
		},
		{
			"999.9 uV",
			"07123408100A0\r\n",
			.00000001,
			.0001234,
			1
		},
		/* mA */
		{
			"9.999 mA",
			"0E123408000A0\r\n",
			.0000001,
			.001234,
			3
		},
		{
			"99.99 mV",
			"0E123408100A0\r\n",
			.000001,
			.01234,
			2
		},
		{
			"399.9 mV",
			"0E123408200A0\r\n",
			.00001,
			.1234,
			1
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
			3
		},
		{
			"99.99 kHz",
			"0F123408300A0\r\n",
			1,
			12340,
			2
		},
		{
			"999.9 kHz",
			"0F123408400A0\r\n",
			10,
			123400,
			1
		},
		{
			"9.999 MHz",
			"0F123408500A0\r\n",
			100,
			1234000,
			3
		},
		{
			"39.99 MHz",
			"0F123408600A0\r\n",
			1000,
			12340000,
			2
		},
		/* Cx */
		{
			"39.99 nF",
			"0B123408000A0\r\n",
			.000000000001,
			.00000001234,
			2
		},
		{
			"399.9 nF",
			"0B123408100A0\r\n",
			.00000000001,
			.0000001234,
			1
		},
		{
			"3.999 uF",
			"0B123408200A0\r\n",
			.0000000001,
			.000001234,
			3
		},
		{
			"39.99 uF",
			"0B123408300A0\r\n",
			.000000001,
			.00001234,
			2
		}
	};

	for (test_idx = 0; test_idx < ARRAY_SIZE(test_params); test_idx++) {
		test_param = &test_params[test_idx];

		srtest_analog_init(&analog, &encoding, &meaning, &spec, -1);
		memset(&info, 0, sizeof(struct fs9922_info));

		ret = meterman_38xr_parse(
			(const uint8_t *)test_param->input_buf, &floatval, &analog, &info);

		fail_unless(ret == SR_OK, "meterman_38xr_parse() failed.");
		ck_assert_float_eq_tol(floatval,
			test_param->expected_value, test_param->tolerance);
		ck_assert_int_eq(
			analog.encoding->digits, test_param->expected_digits);
		ck_assert_int_eq(
			analog.spec->spec_digits, test_param->expected_digits);
	}
}
END_TEST

Suite *suite_dmm_mm38xr(void)
{
	Suite *s;
	TCase *tc;

	s = suite_create("dmm_mm38xr");

	tc = tcase_create("meterman_38xr_parse");
	tcase_add_test(tc, test_value_digits);
	suite_add_tcase(s, tc);

	return s;
}
