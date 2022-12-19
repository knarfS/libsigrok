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

/**
 * DMM packet parser test suite.
 * Can be considered part of the tests/ source code.
 * Gets implemented here because an internal library API is tested.
 *
 * Implement a public routine which is accessible to the tests/main.c
 * application code. While keeping sr_*_parse() routines private to the
 * library. Cover DMM packet parsing with unit tests.
 */
SR_API Suite *suite_dmm_packet_parsers(void)
{
	Suite *s;

	s = suite_create("dmm_packet_parsers");

	register_tests_fs9922(s);

	return s;
}

#endif /* HAVE_CHECK */
