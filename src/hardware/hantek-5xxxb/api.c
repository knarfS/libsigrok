/*
 * This file is part of the libsigrok project.
 *
 * Copyright (C) 2018-2021 Frank Stettner <frank-stettner@gmx.net>
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
/*
 * This driver is based on the protocol description made by tinman,
 * from the mikrocontroller.net and eevblog.com forums:
 * https://www.mikrocontroller.net/articles/Hantek_Protokoll
 * https://elinux.org/Das_Oszi_Protocol
 */

#include <config.h>
#include "protocol.h"

static const uint32_t scanopts[] = {
	SR_CONF_CONN,
};

static const uint32_t drvopts[] = {
	SR_CONF_OSCILLOSCOPE,
};

static const uint32_t devopts[] = {
	SR_CONF_CONTINUOUS,
	SR_CONF_CONN | SR_CONF_GET,
	SR_CONF_LIMIT_MSEC | SR_CONF_GET | SR_CONF_SET,
	SR_CONF_LIMIT_SAMPLES | SR_CONF_GET | SR_CONF_SET,
	SR_CONF_LIMIT_FRAMES | SR_CONF_GET | SR_CONF_SET,

	SR_CONF_NUM_VDIV | SR_CONF_GET,
	SR_CONF_NUM_HDIV | SR_CONF_GET,
	SR_CONF_TIMEBASE | SR_CONF_GET | SR_CONF_SET | SR_CONF_LIST,
	SR_CONF_SAMPLERATE | SR_CONF_GET,
	SR_CONF_TRIGGER_SOURCE | SR_CONF_GET | SR_CONF_SET | SR_CONF_LIST,
	SR_CONF_TRIGGER_SLOPE | SR_CONF_GET | SR_CONF_SET | SR_CONF_LIST,
	SR_CONF_TRIGGER_LEVEL | SR_CONF_GET | SR_CONF_SET,
	SR_CONF_HORIZ_TRIGGERPOS | SR_CONF_GET | SR_CONF_SET | SR_CONF_LIST,
	SR_CONF_BUFFERSIZE | SR_CONF_GET | SR_CONF_SET | SR_CONF_LIST,
	SR_CONF_AVERAGING | SR_CONF_GET | SR_CONF_SET,
	SR_CONF_AVG_SAMPLES | SR_CONF_GET | SR_CONF_SET | SR_CONF_LIST,
};

static const uint32_t devopts_cg[] = {
	SR_CONF_ENABLED | SR_CONF_GET | SR_CONF_SET,
	SR_CONF_VDIV | SR_CONF_GET | SR_CONF_SET | SR_CONF_LIST,
	SR_CONF_COUPLING | SR_CONF_GET | SR_CONF_SET | SR_CONF_LIST,
	SR_CONF_FILTER | SR_CONF_GET | SR_CONF_SET | SR_CONF_LIST,
	SR_CONF_PROBE_FACTOR | SR_CONF_GET | SR_CONF_SET | SR_CONF_LIST,
};

static const char *channel_names[] = {
	"CH1", "CH2",
};

static const uint64_t buffersizes[] = {
	(4 * 1000),        /* 4k */
	(40 * 1000),       /* 40k */
	(512 * 1000),      /* 512k */
	(1 * 1000 * 1000), /* 1M */
};

static GSList *scan(struct sr_dev_driver *di, GSList *options)
{
	struct drv_context *drvc;
	struct dev_context *devc;
	struct sr_dev_inst *sdi;
	struct sr_usb_dev_inst *usb;
	struct sr_config *src;
	struct sr_channel_group *cg;
	struct sr_channel *ch;
	GSList *l, *devices, *conn_devices;
	struct libusb_device_descriptor des;
	libusb_device **devlist;
	unsigned int i, j;
	const char *conn;
	char connection_id[64];

	drvc = di->context;

	devices = NULL;
	conn_devices = NULL;
	drvc->instances = NULL;
	conn = NULL;

	for (l = options; l; l = l->next) {
		src = l->data;
		if (src->key == SR_CONF_CONN) {
			conn = g_variant_get_string(src->data, NULL);
			break;
		}
	}

	if (conn)
		conn_devices = sr_usb_find(drvc->sr_ctx->libusb_ctx, conn);
	else
		conn_devices = NULL;

	libusb_get_device_list(drvc->sr_ctx->libusb_ctx, &devlist);
	for (i = 0; devlist[i]; i++) {
		if (conn) {
			usb = NULL;
			for (l = conn_devices; l; l = l->next) {
				usb = l->data;
				if (usb->bus == libusb_get_bus_number(devlist[i]) &&
				    usb->address == libusb_get_device_address(devlist[i]))
					break;
			}
			if (!l)
				/*
				 * This device matched none of the ones that
				 * matched the conn specification.
				 */
				continue;
		}

		libusb_get_device_descriptor(devlist[i], &des);

		if (des.idVendor != HANTEK_5XXXB_USB_VENDOR ||
				des.idProduct != HANTEK_5XXXB_USB_PRODUCT)
			continue;
		sr_dbg("Found a 0x%X 0x%X.", des.idVendor, des.idProduct);

		if (usb_get_port_path(devlist[i], connection_id, sizeof(connection_id)) < 0)
			continue;

		sdi = g_malloc0(sizeof(struct sr_dev_inst));
		sdi->status = SR_ST_INACTIVE;
		/* TODO
		sdi->status = SR_ST_INITIALIZING;
		sdi->inst_type = SR_INST_USB;
		*/

		/*
		 * All known Devices with this protocol:
		 *   Hantek DSO5202B/BM/BMV, DSO5102B/BM/BMV, DSO5062B/BM/BMV
		 *   Hantek (Handhelds) DSO1202B/BV, DSO1102B/BV, DSO1062B/BV
		 *   Tekway DST1202B, DST1102B, DST1062B
		 *   Protek 3210, 3110
		 *   Voltcraft DSO-1062D, DSO-3062C
		 *
		 * TODO:
		 * in_sys_data->control_type is containing the actual model:
		 *   0x00 : Tekway DST1202B, Hantek DSO5202B/BM/BMV, Protek 3210
		 *   0x01 : Tekway DST1100
		 *   0x02 : Tekway DST4060
		 *   0x03 : Tekway DST1150
		 *   0x04 : Tekway DST4042
		 *   0x05 : Tekway DST1102B, Hantek DSO5102B/BM/BMV, Protek 3110
		 *   0x06 : Tekway DST4062B, Hantek DSO5062C (?)
		 *   0x07 : Tekway DST1152
		 *   0x08 : Tekway DST3022B
		 *   0x09 : Tekway DST3042B
		 *   0x0A : Tekway DST4062
		 *   0x0B : Tekway DST4102B, Hantek DSO5102C (?)
		 *   0x0C : Tekway DST1062B, Hantek DSO5062B/BM/BMV, Voltcraft DSO-1062D/DSO-3062C
		 */
		sdi->vendor = g_strdup("Voltcraft"); // TODO
		sdi->model = g_strdup("DSO-1062D"); // TODO
		sdi->connection_id = g_strdup(connection_id);
		sdi->conn = sr_usb_dev_inst_new(
			libusb_get_bus_number(devlist[i]),
			libusb_get_device_address(devlist[i]), NULL);

		/*
		 * Add only the real channels. EXT isn't a source of data, only
		 * a trigger source internal to the device.
		 */
		for (j = 0; j < ARRAY_SIZE(channel_names); j++) {
			ch = sr_channel_new(sdi, j, SR_CHANNEL_ANALOG, TRUE, channel_names[j]);
			cg = g_malloc0(sizeof(struct sr_channel_group));
			cg->name = g_strdup(channel_names[j]);
			cg->channels = g_slist_append(cg->channels, ch);
			sdi->channel_groups = g_slist_append(sdi->channel_groups, cg);
		}

		devc = g_malloc0(sizeof(struct dev_context));
		sr_sw_limits_init(&devc->limits);
		g_mutex_init(&devc->rw_mutex);
		devc->dev_state = IDLE;
		devc->in_sys_data = g_malloc0(sizeof(struct hantek_5xxxb_sys_data));
		devc->out_sys_data = g_malloc0(sizeof(struct hantek_5xxxb_sys_data));
		sdi->priv = devc;

		devices = g_slist_append(devices, sdi);
	}

	g_slist_free_full(conn_devices, (GDestroyNotify)sr_usb_dev_inst_free);
	libusb_free_device_list(devlist, 1);

	return std_scan_complete(di, devices);
}

static int dev_open(struct sr_dev_inst *sdi)
{
	struct drv_context *drvc;
	struct sr_usb_dev_inst *usb;
	struct libusb_device_descriptor des;
	libusb_device **devlist;
	char connection_id[64];
	int ret;
	int i;

	drvc = sdi->driver->context;
	usb = sdi->conn;

	libusb_get_device_list(drvc->sr_ctx->libusb_ctx, &devlist);
	for (i = 0; devlist[i]; i++) {
		libusb_get_device_descriptor(devlist[i], &des);

		if (des.idVendor != HANTEK_5XXXB_USB_VENDOR ||
				des.idProduct != HANTEK_5XXXB_USB_PRODUCT)
			continue;

		if ((sdi->status == SR_ST_INITIALIZING) ||
				(sdi->status == SR_ST_INACTIVE)) {
			/* Check device by its physical USB bus/port address. */
			if (usb_get_port_path(devlist[i], connection_id, sizeof(connection_id)) < 0)
				continue;

			if (strcmp(sdi->connection_id, connection_id))
				/* This is not the one. */
				continue;
		}

		if (!(ret = libusb_open(devlist[i], &usb->devhdl))) {
			sdi->status = SR_ST_ACTIVE;
			sr_info("Opened device on %d.%d (logical) / "
				"%s (physical) interface %d.",
				usb->bus, usb->address,
				sdi->connection_id, HANTEK_5XXXB_USB_INTERFACE);
		} else
			sr_err("Failed to open device: %s.", libusb_error_name(ret));

		/* If we made it here, we handled the device (somehow). */
		break;
	}
	libusb_free_device_list(devlist, 1);

	if (sdi->status != SR_ST_ACTIVE) {
		sr_err("Unable to open device.");
		return SR_ERR;
	}

	if (libusb_kernel_driver_active(usb->devhdl, HANTEK_5XXXB_USB_INTERFACE) == 1) {
		ret = libusb_detach_kernel_driver(usb->devhdl, HANTEK_5XXXB_USB_INTERFACE);
		if (ret < 0) {
			sr_err("Failed to detach kernel driver: %s.",
				libusb_error_name(ret));
			libusb_close(usb->devhdl);
			return SR_ERR;
		}
	}

	ret = libusb_claim_interface(usb->devhdl, HANTEK_5XXXB_USB_INTERFACE);
	if (ret != 0) {
		sr_err("Unable to claim interface: %s.", libusb_error_name(ret));
		return SR_ERR;
	}

	libusb_reset_device(usb->devhdl);

	return SR_OK;
}

static int dev_close(struct sr_dev_inst *sdi)
{
	struct dev_context *devc;
	struct sr_usb_dev_inst *usb;

	devc = (sdi) ? sdi->priv : NULL;
	if (devc)
		g_mutex_clear(&devc->rw_mutex);

	usb = sdi->conn;
	if (!usb->devhdl)
		return SR_OK;

	sr_info("Closing device on %d.%d (logical) / %s (physical) interface %d.",
		usb->bus, usb->address, sdi->connection_id, HANTEK_5XXXB_USB_INTERFACE);
	libusb_release_interface(usb->devhdl, HANTEK_5XXXB_USB_INTERFACE);
	libusb_close(usb->devhdl);
	usb->devhdl = NULL;
	sdi->status = SR_ST_INACTIVE;

	return SR_OK;
}

static int config_get(uint32_t key, GVariant **data,
	const struct sr_dev_inst *sdi, const struct sr_channel_group *cg)
{
	struct dev_context *devc;
	struct sr_usb_dev_inst *usb;
	gboolean locked;
	int ch_idx;
	int ret;
	uint8_t vb, probe_idx;
	float vdiv, trigger_lvl, trigger_hpos;

	if (!sdi)
		return SR_ERR_ARG;

	if (!sdi->conn)
		return SR_ERR_ARG;

	devc = sdi->priv;
	usb = sdi->conn;

	/* Handle config keys that don't need the SysDATA */
	switch (key) {
	case SR_CONF_LIMIT_SAMPLES:
	case SR_CONF_LIMIT_FRAMES:
	case SR_CONF_LIMIT_MSEC:
		return sr_sw_limits_config_get(&devc->limits, key, data);
	case SR_CONF_CONN:
		*data = g_variant_new_printf("%d.%d", usb->bus, usb->address);
		return SR_OK;
	}

	/*
	 * Don't get the SysDATA when acquisition is running. SysDATA will be
	 * catched by acquision anyways.
	 */
	if (devc->dev_state != CAPTURE) {
		g_mutex_lock(&devc->rw_mutex);
		locked = TRUE;
		hantek_5xxxb_get_sys_data(sdi, devc->in_sys_data);
	} else
		locked = FALSE;

	if (!cg) {
		switch (key) {
		case SR_CONF_NUM_HDIV:
			/*
			 * 16 DIV (640 pixels) with menu visible and 19.2 DIV (768 pixels)
			 * without menu.
			 */
			if (devc->in_sys_data->control_disp_menu)
				*data = g_variant_new_int32(HANTEK_5XXXB_NUM_HDIV_MENU_ON);
			else
				*data = g_variant_new_int32(HANTEK_5XXXB_NUM_HDIV_MENU_OFF_INT);
			break;
		case SR_CONF_NUM_VDIV:
			*data = g_variant_new_int32(HANTEK_5XXXB_NUM_VDIV_INT);
			break;
		case SR_CONF_TIMEBASE:
			*data = g_variant_new("(tt)",
				win_timebase[devc->in_sys_data->horiz_win_tb][0],
				win_timebase[devc->in_sys_data->horiz_win_tb][1]);
			break;
		case SR_CONF_SAMPLERATE:
			*data = g_variant_new_uint64(
				hantek_5xxxb_get_samplerate(devc->in_sys_data));
			break;
		case SR_CONF_TRIGGER_SOURCE:
			*data = g_variant_new_string(
				trigger_source[devc->in_sys_data->trig_src]);
			break;
		case SR_CONF_TRIGGER_SLOPE:
			*data = g_variant_new_string(
				trigger_slope[devc->in_sys_data->trig_edge_slope]);
			break;
		case SR_CONF_TRIGGER_LEVEL:
			ch_idx = devc->in_sys_data->trig_src;
			if (ch_idx > 1) {
				ret = SR_ERR_NA;
				goto done;
			}
			vdiv = hantek_5xxxb_get_volts_per_div(sdi, ch_idx);
			trigger_lvl = hantek_5xxxb_get_value_from_vert_pos(sdi,
				devc->in_sys_data->trig_vpos, vdiv, ch_idx);
			*data = g_variant_new_double(trigger_lvl);
			break;
		case SR_CONF_HORIZ_TRIGGERPOS:
			/* Trigger pos is in ps (1e12) */
			trigger_hpos = (double)devc->in_sys_data->horiz_trigtime / 1e12;
			*data = g_variant_new_double(trigger_hpos);
			break;
		case SR_CONF_BUFFERSIZE:
			*data = g_variant_new_uint64(
				hantek_5xxxb_get_memory_depth_from_sys_data(
					devc->in_sys_data->acqurie_store_depth));
			break;
		case SR_CONF_AVERAGING:
			*data = g_variant_new_boolean(
				devc->in_sys_data->acqurie_mode == ACQ_MODE_AVG);
			break;
		case SR_CONF_AVG_SAMPLES:
			*data = g_variant_new_uint64(
				average_count[devc->in_sys_data->acqurie_avg_cnt]);
			break;
		default:
			ret = SR_ERR_NA;
			goto done;
		}
	} else {
		if (sdi->channel_groups->data == cg)
			ch_idx = 0;
		else if (sdi->channel_groups->next->data == cg)
			ch_idx = 1;
		else {
			ret = SR_ERR_ARG;
			goto done;
		}

		switch (key) {
		case SR_CONF_ENABLED:
			*data = g_variant_new_boolean(
				devc->in_sys_data->vert_ch[ch_idx].disp);
			break;
		case SR_CONF_FILTER:
			*data = g_variant_new_boolean(
				devc->in_sys_data->vert_ch[ch_idx].f20mhz);
			break;
		case SR_CONF_VDIV:
			vb = devc->in_sys_data->vert_ch[ch_idx].vb;
			probe_idx = devc->in_sys_data->vert_ch[ch_idx].probe;
			*data = g_variant_new("(tt)",
				ch_vdiv[probe_idx][vb][0], ch_vdiv[probe_idx][vb][1]);
			break;
		case SR_CONF_COUPLING:
			*data = g_variant_new_string(
				ch_coupling[devc->in_sys_data->vert_ch[ch_idx].coup]);
			break;
		case SR_CONF_PROBE_FACTOR:
			*data = g_variant_new_uint64(
				probe_factor[devc->in_sys_data->vert_ch[ch_idx].probe]);
			break;
		default:
			ret = SR_ERR_NA;
			goto done;
		}
	}

	ret = SR_OK;

done:
	if (locked)
		g_mutex_unlock(&devc->rw_mutex);
	return ret;
}

static int config_set(uint32_t key, GVariant *data,
	const struct sr_dev_inst *sdi, const struct sr_channel_group *cg)
{
	struct dev_context *devc;
	int ret;
	int ch_idx, idx;
	uint8_t probe_idx;
	float vdiv;

	if (!sdi)
		return SR_ERR_ARG;
	devc = sdi->priv;

	/* Handle config keys that don't need the SysDATA */
	switch (key) {
	case SR_CONF_LIMIT_SAMPLES:
	case SR_CONF_LIMIT_FRAMES:
	case SR_CONF_LIMIT_MSEC:
		return sr_sw_limits_config_set(&devc->limits, key, data);
	}

	/*
	 * TODO: See reLoad  Pro
	 * Don't get the SysDATA when acquisition is running. SysDATA will be
	 * catched by acquision anyways.
	 */
	g_mutex_lock(&devc->rw_mutex);
	if (devc->dev_state != CAPTURE)
		hantek_5xxxb_get_sys_data(sdi, devc->in_sys_data);
	memcpy(devc->out_sys_data, devc->in_sys_data,
		sizeof(struct hantek_5xxxb_sys_data));

	if (!cg) {
		switch (key) {
		case SR_CONF_TIMEBASE:
			if ((idx = std_u64_tuple_idx(data, ARRAY_AND_SIZE(win_timebase))) < 0) {
				ret = SR_ERR_ARG;
				goto done;
			}
			hantek_5xxxb_set_timebase(sdi, idx);
			break;
		case SR_CONF_TRIGGER_SOURCE:
			if ((idx = std_str_idx(data, ARRAY_AND_SIZE(trigger_source))) < 0) {
				ret = SR_ERR_ARG;
				goto done;
			}
			devc->out_sys_data->trig_src = (uint8_t)idx;
			break;
		case SR_CONF_TRIGGER_SLOPE:
			if ((idx = std_str_idx(data, ARRAY_AND_SIZE(trigger_slope))) < 0) {
				ret = SR_ERR_ARG;
				goto done;
			}
			devc->out_sys_data->trig_edge_slope = (uint8_t)idx;
			break;
		case SR_CONF_TRIGGER_LEVEL:
			ch_idx = devc->in_sys_data->trig_src;
			if (ch_idx > 1) {
				ret = SR_ERR_ARG;
				goto done;
			}
			vdiv = hantek_5xxxb_get_volts_per_div(sdi, ch_idx);
			devc->out_sys_data->trig_vpos = hantek_5xxxb_get_vert_pos_from_value(
				sdi, g_variant_get_double(data), vdiv, ch_idx);
			break;
		case SR_CONF_HORIZ_TRIGGERPOS:
			/* horiz_trigtime is in ps (1e12) */
			/* TODO: check min (max memory / timebase * 10) / max (10s) */
			devc->out_sys_data->horiz_trigtime =
				g_variant_get_double(data) * 1e12;
			break;
		case SR_CONF_BUFFERSIZE:
			devc->out_sys_data->acqurie_store_depth =
				hantek_5xxxb_get_store_depth_from_memory_depth(
					g_variant_get_uint64(data));
			break;
		case SR_CONF_AVERAGING:
			/* TODO: Check for SR_CONF_BUFFERSIZE */
			devc->out_sys_data->acqurie_mode =
				g_variant_get_boolean(data) ? ACQ_MODE_AVG : ACQ_MODE_NORMAL;
			break;
		case SR_CONF_AVG_SAMPLES:
			/* TODO: Check for SR_CONF_BUFFERSIZE */
			if ((idx = std_u64_idx(data, ARRAY_AND_SIZE(average_count))) < 0) {
				ret = SR_ERR_ARG;
				goto done;
			}
			devc->out_sys_data->acqurie_avg_cnt = (uint8_t)idx;
			break;
		default:
			ret = SR_ERR_ARG;
			goto done;
		}
	} else {
		if (sdi->channel_groups->data == cg)
			ch_idx = 0;
		else if (sdi->channel_groups->next->data == cg)
			ch_idx = 1;
		else {
			ret = SR_ERR_ARG;
			goto done;
		}

		switch (key) {
		case SR_CONF_ENABLED:
			devc->out_sys_data->vert_ch[ch_idx].disp =
				g_variant_get_boolean(data);
			break;
		case SR_CONF_FILTER:
			devc->out_sys_data->vert_ch[ch_idx].f20mhz =
				g_variant_get_boolean(data);
			break;
		case SR_CONF_VDIV:
			probe_idx = devc->in_sys_data->vert_ch[ch_idx].probe;
			if ((idx = std_u64_tuple_idx(data, ARRAY_AND_SIZE(ch_vdiv[probe_idx]))) < 0) {
				ret = SR_ERR_ARG;
				goto done;
			}
			devc->out_sys_data->vert_ch[ch_idx].vb = (uint8_t)idx;
			break;
		case SR_CONF_COUPLING:
			if ((idx = std_str_idx(data, ARRAY_AND_SIZE(ch_coupling))) < 0) {
				ret = SR_ERR_ARG;
				goto done;
			}
			devc->out_sys_data->vert_ch[ch_idx].coup = (uint8_t)idx;
			break;
		case SR_CONF_PROBE_FACTOR:
			if ((idx = std_u64_idx(data, ARRAY_AND_SIZE(probe_factor))) < 0) {
				ret = SR_ERR_ARG;
				goto done;
			}
			devc->out_sys_data->vert_ch[ch_idx].probe = (uint8_t)idx;
			break;
		default:
			ret = SR_ERR_NA;
			goto done;
		}
	}

	ret = hantek_5xxxb_set_sys_data(sdi);

done:
	g_mutex_unlock(&devc->rw_mutex);
	return ret;
}

static int config_list(uint32_t key, GVariant **data,
	const struct sr_dev_inst *sdi, const struct sr_channel_group *cg)
{
	struct dev_context *devc;
	int ch_idx;
	uint8_t probe_idx;
	gboolean locked;

	devc = sdi ? sdi->priv : NULL;

	if (!cg) {
		switch (key) {
		case SR_CONF_SCAN_OPTIONS:
		case SR_CONF_DEVICE_OPTIONS:
			return STD_CONFIG_LIST(key, data, sdi, cg, scanopts, drvopts, devopts);
		case SR_CONF_TIMEBASE:
			*data = std_gvar_tuple_array(ARRAY_AND_SIZE(win_timebase));
			break;
		case SR_CONF_TRIGGER_SOURCE:
			*data = g_variant_new_strv(ARRAY_AND_SIZE(trigger_source));
			break;
		case SR_CONF_TRIGGER_SLOPE:
			*data = g_variant_new_strv(ARRAY_AND_SIZE(trigger_slope));
			break;
		case SR_CONF_HORIZ_TRIGGERPOS:
			/* TODO: This are estimated values! */
			*data = std_gvar_min_max_step(-1, 1, 1e-9);
			break;
		case SR_CONF_BUFFERSIZE:
			/* TODO: Depending on SR_CONF_AVERAGING and SR_CONF_TIMEBASE */
			*data = std_gvar_array_u64(ARRAY_AND_SIZE(buffersizes));
			break;
		case SR_CONF_AVG_SAMPLES:
			/* TODO: Depending on SR_CONF_BUFFERSIZE */
			*data = std_gvar_array_u64(ARRAY_AND_SIZE(average_count));
			break;
		default:
			return SR_ERR_NA;
		}
	} else {
		if (sdi->channel_groups->data == cg)
			ch_idx = 0;
		else if (sdi->channel_groups->next->data == cg)
			ch_idx = 1;
		else
			return SR_ERR_ARG;

		switch (key) {
		case SR_CONF_DEVICE_OPTIONS:
			*data = std_gvar_array_u32(ARRAY_AND_SIZE(devopts_cg));
			break;
		case SR_CONF_COUPLING:
			*data = g_variant_new_strv(ARRAY_AND_SIZE(ch_coupling));
			break;
		case SR_CONF_VDIV:
			/*
			 * TODO: See reLoad  Pro
			 * Don't get the SysDATA when acquisition is running. SysDATA will be
			 * catched by acquision anyways.
			 */
			/* TODO: I don't like... */
			if (devc->dev_state != CAPTURE) {
				g_mutex_lock(&devc->rw_mutex);
				locked = TRUE;
				hantek_5xxxb_get_sys_data(sdi, devc->in_sys_data);
			} else
				locked = FALSE;

			probe_idx = devc->in_sys_data->vert_ch[ch_idx].probe;
			*data = std_gvar_tuple_array(ARRAY_AND_SIZE(ch_vdiv[probe_idx]));
			if (locked)
				g_mutex_unlock(&devc->rw_mutex);
			break;
		case SR_CONF_PROBE_FACTOR:
			*data = std_gvar_array_u64(ARRAY_AND_SIZE(probe_factor));
			break;
		default:
			return SR_ERR_NA;
		}
	}

	return SR_OK;
}

static int dev_acquisition_start(const struct sr_dev_inst *sdi)
{
	struct dev_context *devc;
	struct sr_dev_driver *di = sdi->driver;
	struct drv_context *drvc = di->context;

	devc = sdi->priv;

	devc->dev_state = CAPTURE;

	sr_sw_limits_acquisition_start(&devc->limits);
	std_session_send_df_header(sdi);

	usb_source_add(sdi->session, drvc->sr_ctx, 1000,
		hantek_5xxxb_receive_data, (void *)sdi);

	return SR_OK;
}

static int dev_acquisition_stop(struct sr_dev_inst *sdi)
{
	struct dev_context *devc;

	devc = sdi->priv;
	devc->dev_state = STOPPING;

	return SR_OK;
}

static struct sr_dev_driver hantek_5xxxb_driver_info = {
	.name = "hantek-5xxxb",
	.longname = "Hantek 5xxxB",
	.api_version = 1,
	.init = std_init,
	.cleanup = std_cleanup,
	.scan = scan,
	.dev_list = std_dev_list,
	.dev_clear = std_dev_clear,
	.config_get = config_get,
	.config_set = config_set,
	.config_list = config_list,
	.dev_open = dev_open,
	.dev_close = dev_close,
	.dev_acquisition_start = dev_acquisition_start,
	.dev_acquisition_stop = dev_acquisition_stop,
	.context = NULL,
};

SR_REGISTER_DEV_DRIVER(hantek_5xxxb_driver_info);
