
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
	struct sr_dev_driver *driver;
	struct sr_dev_dmm_driver *dmm;
	size_t test_idx;
	struct sr_datafeed_analog analog;
	struct sr_analog_encoding encoding;
	struct sr_analog_meaning meaning;
	struct sr_analog_spec spec;
	struct asycii_info info;
	float floatval;
	int ret;

	driver = srtest_driver_get("metrix-mx56c");
	dmm = (struct sr_dev_dmm_driver *)driver;

	struct {
		const char *desc;
		const uint8_t input_buf[16];
		float tolerance;
		float expected_value;
		int expected_digits;
	} *test_param, test_params[] = {
		/* Buffer samples: https://sigrok.org/wiki/Metrix_MX56C */
		/* 2 decimal places */
		{
			"2 decimal places with nano prefix",
			{0x20, 0x20, 0x31, 0x32, 0x2e, 0x33, 0x34, 0x6e, 0x46, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x0d},
			.000000000001,
			.00000001234,
			11
		},
		/* 3 decimal places */
		{
			"3 decimal places with mega prefix",
			{0x20, 0x31, 0x32, 0x2e, 0x33, 0x34, 0x35, 0x4d, 0x6f, 0x68, 0x6d, 0x20, 0x20, 0x20, 0x20, 0x0d},
			100,
			12345000.,
			-3
		},
		/* 4 decimal places */
		{
			"4 decimal places with no prefix",
			{0x20, 0x31, 0x2e, 0x32, 0x33, 0x34, 0x35, 0x20, 0x56, 0x64, 0x63, 0x20, 0x20, 0x20, 0x20, 0x0d},
			.00001,
			1.2345,
			4
		},
	};

	for (test_idx = 0; test_idx < ARRAY_SIZE(test_params); test_idx++) {
		test_param = &test_params[test_idx];

		srtest_analog_init(&analog, &encoding, &meaning, &spec, -1);
		memset(&info, 0, sizeof(struct fs9922_info));

		ret = dmm->packet_parse(
			test_param->input_buf, &floatval, &analog, &info);

		fail_unless(ret == SR_OK,
			"sr_asycii_parse() for '%s' failed with %d.",
			test_param->desc, ret);
		srtest_assert_float_eq_tol(floatval, test_param->expected_value,
			test_param->tolerance, test_param->desc);
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

Suite *suite_dmm_asycii(void)
{
	Suite *s;
	TCase *tc;

	s = suite_create("dmm_asycii");

	tc = tcase_create("sr_asycii_parse");
	tcase_add_checked_fixture(tc, srtest_setup, srtest_teardown);
	tcase_add_test(tc, test_value_digits);
	suite_add_tcase(s, tc);

	return s;
}
