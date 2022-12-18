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
		const uint8_t input_buf[13];
		float tolerance;
		float expected_value;
		int expected_digits;
	} *test_param, test_params[] = {
		/* A600 */
		{
			"600.0 A",
			{0x55, 0x00, 0x00, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.01,
			123.4,
			1
		},
		/* A60 */
		{
			"60.00 A",
			{0x55, 0x01, 0x00, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.001,
			12.34,
			2
		},
		/* V */
		{
			"600.0 mV",
			{0x55, 0x02, 0x00, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.00001,
			.1234,
			1
		},
		{
			"6.000 V",
			{0x55, 0x02, 0x01, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.0001,
			1.234,
			3
		},
		{
			"60.00 V",
			{0x55, 0x02, 0x02, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.001,
			12.34,
			2
		},
		{
			"600.0 V",
			{0x55, 0x02, 0x03, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.01,
			123.4,
			1
		},
		{
			"1000 V",
			{0x55, 0x02, 0x04, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.1,
			1234,
			0
		},
		/* Diode */
		{
			"2.500 V",
			{0x55, 0x0b, 0x00, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.0001,
			1.234,
			3
		},
		/* Ohm */
		{
			"600.0 Ohm",
			{0x55, 0x04, 0x00, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.01,
			123.4,
			1
		},
		{
			"6.000 kOhm",
			{0x55, 0x04, 0x01, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.1,
			1234,
			3
		},
		{
			"60.00 kOhm",
			{0x55, 0x04, 0x02, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			1,
			12340,
			2
		},
		{
			"600.0 kOhm",
			{0x55, 0x04, 0x03, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			10,
			123400,
			1
		},
		{
			"6.000 MOhm",
			{0x55, 0x04, 0x04, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			100,
			1234000,
			3
		},
		{
			"60.00 MOhm",
			{0x55, 0x04, 0x05, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			1000,
			12340000,
			2
		},
		/* C */
		{
			"6.000 nF",
			{0x55, 0x05, 0x00, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.0000000000001,
			.000000001234,
			3
		},
		{
			"60.00 nF",
			{0x55, 0x05, 0x01, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.000000000001,
			.00000001234,
			2
		},
		{
			"600.0 nF",
			{0x55, 0x05, 0x02, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.00000000001,
			.0000001234,
			1
		},
		{
			"6.000 uF",
			{0x55, 0x05, 0x03, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.0000000001,
			.000001234,
			3
		},
		{
			"60.00 uF",
			{0x55, 0x05, 0x04, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.000000001,
			.00001234,
			2
		},
		{
			"600.0 uF",
			{0x55, 0x05, 0x05, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.00000001,
			.0001234,
			1
		},
		{
			"6.000 mF",
			{0x55, 0x05, 0x06, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.0000001,
			.001234,
			3
		},
		{
			"60.00 mF",
			{0x55, 0x05, 0x07, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.000001,
			.01234,
			2
		},
		/* Hz */
		{
			"60.00 Hz",
			{0x55, 0x06, 0x00, 0x00, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.001,
			12.34,
			2
		},
		{
			"600.0 Hz",
			{0x55, 0x06, 0x00, 0x01, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.01,
			123.4,
			1
		},
		{
			"6.000 kHz",
			{0x55, 0x06, 0x00, 0x02, 0xd2, 0x04, 0x00, 0x00, 0x01},
			.1,
			1234,
			3
		},
		{
			"60.00 kHz",
			{0x55, 0x06, 0x00, 0x03, 0xd2, 0x04, 0x00, 0x00, 0x01},
			1,
			12340,
			2
		},
		{
			"600.0 kHz",
			{0x55, 0x06, 0x00, 0x04, 0xd2, 0x04, 0x00, 0x00, 0x01},
			10,
			123400,
			1
		},
		{
			"6.000 MHz",
			{0x55, 0x06, 0x00, 0x05, 0xd2, 0x04, 0x00, 0x00, 0x01},
			100,
			1234000,
			3
		},
		{
			"60.00 MHz",
			{0x55, 0x06, 0x00, 0x06, 0xd2, 0x04, 0x00, 0x00, 0x01},
			1000,
			12340000,
			2
		},
	};

	for (test_idx = 0; test_idx < ARRAY_SIZE(test_params); test_idx++) {
		test_param = &test_params[test_idx];

		srtest_analog_init(&analog, &encoding, &meaning, &spec, -1);
		memset(&info, 0, sizeof(struct fs9922_info));

		ret = sr_ms2115b_parse(
			test_param->input_buf, &floatval, &analog, &info);

		fail_unless(ret == SR_OK, "sr_ms2115b_parse() failed.");
		ck_assert_float_eq_tol(floatval,
			test_param->expected_value, test_param->tolerance);
		ck_assert_int_eq(
			analog.encoding->digits, test_param->expected_digits);
		ck_assert_int_eq(
			analog.spec->spec_digits, test_param->expected_digits);
	}
}
END_TEST

Suite *suite_dmm_ms2115b(void)
{
	Suite *s;
	TCase *tc;

	s = suite_create("dmm_ms2115b");

	tc = tcase_create("sr_ms2115b_parse");
	tcase_add_test(tc, test_value_digits);
	suite_add_tcase(s, tc);

	return s;
}
