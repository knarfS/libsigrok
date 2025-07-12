/*
 * This file is part of the libsigrok project.
 *
 * Copyright (C) 2018 Sven Bursch-Osewold <sb_git@bursch.com>
 * Copyright (C) 2019 King Kévin <kingkevin@cuvoodoo.info>
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

#ifndef LIBSIGROK_HARDWARE_ZKETECH_EBD_USB_PROTOCOL_H
#define LIBSIGROK_HARDWARE_ZKETECH_EBD_USB_PROTOCOL_H

#include <stdint.h>
#include <glib.h>
#include <libsigrok/libsigrok.h>
#include "libsigrok-internal.h"

#define LOG_PREFIX "zketech-ebd-usb"

#define MSG_MAX_LEN 19
#define MSG_FRAME_BEGIN 0xfa
#define MSG_FRAME_END 0xf8

struct dev_context {
	struct sr_sw_limits limits;
	//GMutex rw_mutex;
	gboolean enabled;
	float current_limit;
	float uvc_threshold;
	gboolean acquisition_running;
};

/* Communication via serial. */
SR_PRIV int ebd_read_message(struct sr_serial_dev_inst *serial, size_t length,
	uint8_t *buf);

/* Commands. */
SR_PRIV int ebd_start_measurement(struct sr_serial_dev_inst *serial,
	struct dev_context *devc);
SR_PRIV int ebd_stop_measurement(struct sr_serial_dev_inst *serial,
	struct dev_context *devc);
// TODO: rename to ebd_enable_load()
SR_PRIV int ebd_load_enable(struct sr_serial_dev_inst *serial,
	struct dev_context *devc);
// TODO: rename to ebd_disable_load()
SR_PRIV int ebd_load_disable(struct sr_serial_dev_inst *serial,
	struct dev_context *devc);
SR_PRIV int ebd_receive_data(int fd, int revents, void *cb_data);

/* Configuration. */
SR_PRIV int ebd_get_enabled(const struct sr_dev_inst *sdi, gboolean *enabled);
SR_PRIV int ebd_set_enabled(const struct sr_dev_inst *sdi, gboolean enable);
SR_PRIV int ebd_get_current_limit(const struct sr_dev_inst *sdi, float *current);
SR_PRIV int ebd_set_current_limit(const struct sr_dev_inst *sdi, float current);
SR_PRIV int ebd_get_uvc_threshold(const struct sr_dev_inst *sdi, float *voltage);
SR_PRIV int ebd_set_uvc_threshold(const struct sr_dev_inst *sdi, float voltage);

#endif
