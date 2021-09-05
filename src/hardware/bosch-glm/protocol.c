/*
 * This file is part of the libsigrok project.
 *
 * Copyright (C) 2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <config.h>
#include "protocol.h"

static uint8_t bosch_glm_crc(const uint8_t *buffer, int len)
{
	uint8_t crc, poly, b, x;
	size_t i;

	crc = 0xaa;
	poly = 0xa6;

	if (!buffer || len < 0)
		return crc;

	while (len--) {
		b = *buffer++;
		for (i = 0; i < 8; i++) {
			x = ((crc >> 7) ^ (b >> (7-i))) & 1;
			crc = (crc << 1) & 0xFF;
			if (x)
				crc ^= poly;
		}
	}

	return crc;
}

SR_PRIV int bosch_glm_receive_data(int fd, int revents, void *cb_data)
{
	struct sr_dev_inst *sdi;
	struct dev_context *devc;
	uint8_t out_buf[4];
	uint8_t in_buf[16];
	ssize_t len;
	struct sr_datafeed_packet packet;
	struct sr_datafeed_analog analog;
	struct sr_analog_encoding encoding;
	struct sr_analog_meaning meaning;
	struct sr_analog_spec spec;

	(void)fd;
	(void)revents;

	if (!(sdi = cb_data))
		return TRUE;

	if (!(devc = sdi->priv))
		return TRUE;

	out_buf[0] = 0xC0;
	out_buf[1] = 0x40;
	out_buf[2] = 0x00;
	out_buf[3] = bosch_glm_crc(out_buf, 3);
	len = sr_bt_write(sdi->conn, ARRAY_AND_SIZE(out_buf));
	if (len < 0)
		return SR_ERR_IO;

	len = sr_bt_read(sdi->conn, in_buf, sizeof(in_buf));
	if (len <= 0) // 7
		return SR_ERR_IO;

	if (in_buf[0] != 0) {
		switch (in_buf[0]) {
		case 1:
			sr_err("Communication timeout");
			break;
		case 3:
			sr_err("Checksum error");
			break;
		case 4:
			sr_err("Unknown command");
			break;
		case 5:
			sr_err("Invalid access level");
			break;
		case 8:
			sr_err("Hardware error");
			break;
		case 10:
			sr_err("Device not ready");
			break;
		default:
			sr_err("Unknown error");
		}
		return SR_ERR_IO;
	}
	//for (size_t i=0; i<len; i++) {
	//	sr_err("bosch_glm_receive_data(): BUF[%zu] = %d", i, in_buf[i]);
	//}
	uint8_t crc = bosch_glm_crc(in_buf, 6);
	if (in_buf[6] != crc)
		sr_err("Wrong checksum: Got %x but should be %x", in_buf[6], crc);

	float fvalue = (float)read_u32le(in_buf + 2) * 0.00005;

	packet.type = SR_DF_ANALOG;
	packet.payload = &analog;

	sr_analog_init(&analog, &encoding, &meaning, &spec, 3);

	analog.num_samples = 1;
	analog.data = &fvalue;

	encoding.unitsize = sizeof(float);
	encoding.is_float = TRUE;
	encoding.digits = 3;

	meaning.mq = SR_MQ_VOLTAGE;
	//meaning.mqflags = devc->acquisition_mq_flags;
	meaning.unit = SR_UNIT_VOLT;
	meaning.channels = sdi->channels;

	spec.spec_digits = 3;

	sr_session_send(sdi, &packet);

	sr_sw_limits_update_samples_read(&devc->limits, 1);
	if (sr_sw_limits_check(&devc->limits))
		sr_dev_acquisition_stop(sdi);

	return TRUE;
}
