/*
 * This file is part of the libsigrok project.
 *
 * Copyright (C) 2012 Alexandru Gagniuc <mr.nuke.me@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBSIGROK_HARDWARE_SERIAL_DMM_PROTOCOL_H
#define LIBSIGROK_HARDWARE_SERIAL_DMM_PROTOCOL_H

#define LOG_PREFIX "serial-dmm"

#define DMM_BUFSIZE 256

struct dev_context {
	struct sr_sw_limits limits;

	uint8_t buf[DMM_BUFSIZE];
	size_t buflen;

	/**
	 * The timestamp [µs] to send the next request.
	 * Used only if device needs polling.
	 */
	uint64_t req_next_at;
};

SR_PRIV int req_packet(struct sr_dev_inst *sdi);
SR_PRIV int receive_data(int fd, int revents, void *cb_data);

#endif
