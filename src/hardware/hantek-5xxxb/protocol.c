/*
 * This file is part of the libsigrok project.
 *
 * This driver is based on the protocol description made by tinman,
 * from the mikrocontroller.net and eevblog.com forums:
 * https://www.mikrocontroller.net/articles/Hantek_Protokoll
 * https://elinux.org/Das_Oszi_Protocol
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

#include <config.h>
#include <math.h>
#include "protocol.h"

//#define HANTEK_DEBUG

static void print_sys_data(struct hantek_5xxxb_sys_data *sys_data)
{
#ifndef HANTEK_DEBUG
	(void)sys_data;
#else
	sr_dbg("SysDATA:");

	/* Vertical Channel 1 */
	sr_dbg("");
	sr_dbg("  vert_ch1_disp = 0x%X", sys_data->vert_ch[0].disp);
	sr_dbg("  vert_ch1_vb = 0x%X", sys_data->vert_ch[0].vb);
	sr_dbg("  vert_ch1_coup = 0x%X", sys_data->vert_ch[0].coup);
	sr_dbg("  vert_ch1_20mhz = 0x%X", sys_data->vert_ch[0].f20mhz);
	sr_dbg("  vert_ch1_fine = 0x%X", sys_data->vert_ch[0].fine);
	sr_dbg("  vert_ch1_probe = 0x%X", sys_data->vert_ch[0].probe);
	sr_dbg("  vert_ch1_rphase = 0x%X", sys_data->vert_ch[0].rphase);
	sr_dbg("  vert_ch1_cnt_fine = 0x%X", sys_data->vert_ch[0].cnt_fine);
	sr_dbg("  vert_ch1_pos = %i", sys_data->vert_ch[0].pos);

	/* Vertical Channel 2 */
	sr_dbg("");
	sr_dbg("  vert_ch2_disp = 0x%X", sys_data->vert_ch[1].disp);
	sr_dbg("  vert_ch2_vb = 0x%X", sys_data->vert_ch[1].vb);
	sr_dbg("  vert_ch2_coup = 0x%X", sys_data->vert_ch[1].coup);
	sr_dbg("  vert_ch2_20mhz = 0x%X", sys_data->vert_ch[1].f20mhz);
	sr_dbg("  vert_ch2_fine = 0x%X", sys_data->vert_ch[1].fine);
	sr_dbg("  vert_ch2_probe = 0x%X", sys_data->vert_ch[1].probe);
	sr_dbg("  vert_ch2_rphase = 0x%X", sys_data->vert_ch[1].rphase);
	sr_dbg("  vert_ch2_cnt_fine = 0x%X", sys_data->vert_ch[1].cnt_fine);
	sr_dbg("  vert_ch2_pos = %i", sys_data->vert_ch[1].pos);

	/* Trigger */
	sr_dbg("");
	sr_dbg("  trig_state = 0x%X", sys_data->trig_state);
	sr_dbg("  trig_type = 0x%X", sys_data->trig_type);
	sr_dbg("  trig_src = 0x%X", sys_data->trig_src);
	sr_dbg("  trig_mode = 0x%X", sys_data->trig_mode);
	sr_dbg("  trig_coup = 0x%X", sys_data->trig_coup);
	sr_dbg("  trig_vpos = %i", sys_data->trig_vpos);
	sr_dbg("  trig_frequency = %lu mHz", sys_data->trig_frequency);
	sr_dbg("  trig_holdtime_min = %lu ps", sys_data->trig_holdtime_min);
	sr_dbg("  trig_holdtime_max = %lu ps", sys_data->trig_holdtime_max);
	sr_dbg("  trig_holdtime = %lu ps", sys_data->trig_holdtime);

	/*
	uint8_t trig_edge_slope;
	uint8_t trig_video_neg;
	uint8_t trig_video_pal;
	uint8_t trig_video_syn;
	uint16_t trig_video_line;
	uint8_t trig_pulse_neg;
	uint8_t trig_pulse_when;
	uint64_t trig_pulse_time;
	uint8_t trig_slope_set;
	uint8_t trig_slope_win;
	uint8_t trig_slope_when;
	uint16_t trig_slope_v1;
	uint16_t trig_slope_v2;
	uint64_t trig_slope_time;

	uint8_t trig_swap_ch1_type;
	uint8_t trig_swap_ch1_mode;
	uint8_t trig_swap_ch1_coup;
	uint8_t trig_swap_ch1_edge_slope;
	uint8_t trig_swap_ch1_video_neg;
	uint8_t trig_swap_ch1_video_pal;
	uint8_t trig_swap_ch1_video_syn;
	uint16_t trig_swap_ch1_video_line;
	uint8_t trig_swap_ch1_pulse_neg;
	uint8_t trig_swap_ch1_pulse_when;
	uint64_t trig_swap_ch1_pulse_time;
	uint8_t trig_swap_ch1_slope_set;
	uint8_t trig_swap_ch1_slope_win;
	uint8_t trig_swap_ch1_slope_when;
	uint16_t trig_swap_ch1_slope_v1;
	uint16_t trig_swap_ch1_slope_v2;
	uint64_t trig_swap_ch1_slope_time;

	uint8_t trig_swap_ch2_type;
	uint8_t trig_swap_ch2_mode;
	uint8_t trig_swap_ch2_coup;
	uint8_t trig_swap_ch2_edge_slope;
	uint8_t trig_swap_ch2_video_neg;
	uint8_t trig_swap_ch2_video_pal;
	uint8_t trig_swap_ch2_video_syn;
	uint16_t trig_swap_ch2_video_line;
	uint8_t trig_swap_ch2_pulse_neg;
	uint8_t trig_swap_ch2_pulse_when;
	uint64_t trig_swap_ch2_pulse_time;
	uint8_t trig_swap_ch2_slope_set;
	uint8_t trig_swap_ch2_slope_win;
	uint8_t trig_swap_ch2_slope_when;
	uint16_t trig_swap_ch2_slope_v1;
	uint16_t trig_swap_ch2_slope_v2;
	uint64_t trig_swap_ch2_slope_time;

	uint8_t trig_overtime_neg;
	uint64_t trig_overtime_time;
	*/

	/* Horizontal */
	sr_dbg("");
	sr_dbg("  horiz_tb = 0x%X", sys_data->horiz_tb);
	sr_dbg("  horiz_win_tb = 0x%X", sys_data->horiz_win_tb);
	sr_dbg("  horiz_win_state = 0x%X", sys_data->horiz_win_state);
	sr_dbg("  horiz_trigtime = %li ps", sys_data->horiz_trigtime);

	/* Math */
	/*
	uint8_t math_disp;
	uint8_t math_mode;
	uint8_t math_fft_src;
	uint8_t math_fft_win;
	uint8_t math_fft_factor;
	uint8_t math_fft_db;
	*/

	/* Display */
	/*
	uint8_t display_mode;
	uint8_t display_persist;
	uint8_t display_format;
	uint8_t display_contrast;
	uint8_t display_maxcontrast;
	uint8_t display_grid_kind;
	uint8_t display_grid_bright;
	uint8_t display_maxgrid_bright;
	*/

	/* Aquire */
	/*
	uint8_t acqurie_mode;
	uint8_t acqurie_avg_cnt;
	uint8_t acqurie_type;
	uint8_t acqurie_store_depth;
	*/

	/* Measure */
	/*
	uint8_t measure_item1_src;
	uint8_t measure_item1;
	uint8_t measure_item2_src;
	uint8_t measure_item2;
	uint8_t measure_item3_src;
	uint8_t measure_item3;
	uint8_t measure_item4_src;
	uint8_t measure_item4;
	uint8_t measure_item5_src;
	uint8_t measure_item5;
	uint8_t measure_item6_src;
	uint8_t measure_item6;
	uint8_t measure_item7_src;
	uint8_t measure_item7;
	uint8_t measure_item8_src;
	uint8_t measure_item8;
	*/

	/* Control */
	sr_dbg("");
	sr_dbg("  control_type = 0x%X", sys_data->control_type);
	sr_dbg("  control_menuid = 0x%X", sys_data->control_menuid);
	sr_dbg("  control_disp_menu = 0x%X", sys_data->control_disp_menu);
	sr_dbg("  control_mul_win = 0x%X", sys_data->control_mul_win);

	/*
	uint16_t trig_swap_ch1_vpos;
	uint8_t trig_swap_ch1_overtime_neg;
	uint64_t trig_swap_ch1_overtime_time;
	uint16_t trig_swap_ch2_vpos;
	uint8_t trig_swap_ch2_overtime_neg;
	uint64_t trig_swap_ch2_overtime_time;

	uint8_t math_fft_base;
	uint8_t math_fft_vrms;
	*/
#endif
}

static uint8_t calc_checksum(uint8_t *msg, uint16_t msg_size)
{
	uint16_t i;
	uint16_t sum;
	uint8_t checksum;

	sum = 0;
	for (i=0; i<msg_size; ++i) {
		sum += msg[i];
	}
	checksum = (uint8_t)(0x00FF & sum);

	return checksum;
}

static int send_bulk(const struct sr_dev_inst *sdi,
	uint8_t *out_buf, uint16_t out_size)
{
	struct sr_usb_dev_inst *usb;
	int ret, tmp;

	usb = sdi->conn;

	/*
	sr_err("send_bulk(): out_buff:");
	for (int i=0; i<out_size; ++i)
		printf("0x%X ", out_buf[i]);
	printf("\n");
	*/

	// TODO: Calc checksum here?

	ret = libusb_bulk_transfer(usb->devhdl, HANTEK_5XXXB_USB_EP_OUT,
		out_buf, out_size, &tmp, 500);
	if (ret != 0) {
		sr_err("libusb_bulk_transfer(): Send cmd 0x%X failed with %s",
			out_buf[3], libusb_error_name(ret));
		return SR_ERR;
	}

	return SR_OK;
}

static int receive_bulk(const struct sr_dev_inst *sdi,
	uint8_t *in_buf, int *in_size, uint32_t max_size, uint8_t expected_cmd)
{
	struct sr_usb_dev_inst *usb;
	int ret;
	int package_size;
	uint8_t checksum;

	usb = sdi->conn;

	ret = libusb_bulk_transfer(usb->devhdl, HANTEK_5XXXB_USB_EP_IN,
		in_buf, max_size, in_size, 5000); // TODO: setable timeout
	if (ret != 0) {
		sr_err("libusb_bulk_transfer(): Receive cmd 0x%X failed with %s",
			expected_cmd, libusb_error_name(ret));
		return SR_ERR;
	}

	package_size = (in_buf[2] << 8) | in_buf[1];
	if (*in_size != package_size+3) {
		sr_err("Response size does not match (got %i, expected %i)",
			package_size+3, *in_size);

		sr_err("receive_bulk(): in_size = %i", *in_size);
		sr_err("receive_bulk(): in_buff:");
		for (int i=0; i<*in_size; ++i)
			printf("0x%X ", in_buf[i]);
		printf("\n");

		return SR_ERR;
	}

	checksum = calc_checksum(in_buf, *in_size-1);
	if (in_buf[*in_size-1] != checksum) {
		sr_err("Response checksum does not match (got %u, expected %u)",
			in_buf[*in_size-1], checksum);
		return SR_ERR;
	}

	if (in_buf[3] != expected_cmd + 0x80) {
		sr_err("Response command does not match (got 0x%X, expected 0x%X)",
			in_buf[3], expected_cmd + 0x80);

		sr_err("receive_bulk(): in_size = %i", *in_size);
		sr_err("receive_bulk(): in_buff:");
		for (int i=0; i<*in_size; ++i)
			printf("0x%X ", in_buf[i]);
		printf("\n");

		return SR_ERR;
	}

	return SR_OK;
}

static void send_df_chunk(const struct sr_dev_inst *sdi,
	uint8_t *buf, int num_samples, int channel_idx)
{
	struct sr_datafeed_packet packet;
	struct sr_datafeed_analog analog;
	struct sr_analog_encoding encoding;
	struct sr_analog_meaning meaning;
	struct sr_analog_spec spec;
	GSList *l;
	float vdiv, vdivlog;
	int digits;

	packet.type = SR_DF_ANALOG;
	packet.payload = &analog;

	sr_analog_init(&analog, &encoding, &meaning, &spec, 0);
	analog.num_samples = num_samples;
	analog.meaning->mq = SR_MQ_VOLTAGE;
	analog.meaning->unit = SR_UNIT_VOLT;
	analog.meaning->mqflags = 0;

	l = g_slist_append(NULL, g_slist_nth_data(sdi->channels, channel_idx));
	analog.meaning->channels = l;

	vdiv = hantek_5xxxb_get_volts_per_div(sdi, channel_idx);
	vdivlog = log10f(vdiv * HANTEK_5XXXB_NUM_VDIV / 255);
	//sr_err("send_df_chunk(): vdivlog = %f", vdivlog);
	digits = -(int)vdivlog + (vdivlog < 0.0);
	//sr_err("send_df_chunk(): digits = %i", digits);
	analog.encoding->digits = digits;
	analog.spec->spec_digits = digits;

	analog.data = g_try_malloc(num_samples * sizeof(float));

	for (int i = 0; i < num_samples; i++) {
		((float *)analog.data)[i] = hantek_5xxxb_get_value_from_vert_pos(
			sdi, (int8_t)*(buf + i), vdiv, channel_idx);
	}
	//sr_err("send_df_chunk(): send %i samples", num_samples);
	sr_session_send(sdi, &packet);
	g_slist_free(analog.meaning->channels);

	g_free(analog.data);
}

SR_PRIV uint64_t hantek_5xxxb_get_samplerate(
	const struct hantek_5xxxb_sys_data *sys_data)
{
	uint64_t samplerate;
	size_t sample_rate_array_idx;
	double num_hdiv;
	uint32_t samples;

	sample_rate_array_idx =
		hantek_5xxxb_get_sample_rate_array_index_from_sys_data(
			sys_data->acqurie_store_depth);

	/* This would return the correct sample rate of the scope, but it wouldn't
	 * match the time stride of the actual samples
	samplerate = sample_rate
		[sys_data->horiz_win_tb]
		[sys_data->vert_ch[0].disp & sys_data->vert_ch[1].disp]
		[sample_rate_array_idx];
	*/

	if (sys_data->control_disp_menu)
		num_hdiv = HANTEK_5XXXB_NUM_HDIV_MENU_ON;
	else
		num_hdiv = HANTEK_5XXXB_NUM_HDIV_MENU_OFF;

	samples = sample_count
			[sys_data->horiz_win_tb]
			[sys_data->vert_ch[0].disp & sys_data->vert_ch[1].disp]
			[sys_data->control_disp_menu]
			[sample_rate_array_idx];
	samplerate = (uint64_t)(samples /
			((win_timebase[sys_data->horiz_win_tb][0] /
			(float)win_timebase[sys_data->horiz_win_tb][1]) * num_hdiv));

	return samplerate;
}

SR_PRIV void hantek_5xxxb_set_timebase(const struct sr_dev_inst *sdi,
	int timebase_idx)
{
	struct dev_context *devc;
	uint64_t samplerate;
	size_t sample_rate_array_idx;

	devc = sdi->priv;

	sample_rate_array_idx =
		hantek_5xxxb_get_sample_rate_array_index_from_sys_data(
			devc->in_sys_data->acqurie_store_depth);
	/* Check if the current memory depth is valide for the new timebase. */
	samplerate = sample_rate
		[devc->in_sys_data->horiz_win_tb]
		[devc->in_sys_data->vert_ch[0].disp & devc->in_sys_data->vert_ch[1].disp]
		[sample_rate_array_idx];
	if (samplerate == 0) {
		/* Current memory depth is not valide, set to fixed 512k TODO */
		devc->out_sys_data->acqurie_store_depth =
			hantek_5xxxb_get_store_depth_from_sample_rate_array_index(
				sample_rate_array_idx - 1);
	}

	devc->out_sys_data->horiz_win_tb = (uint8_t)timebase_idx;
}

SR_PRIV uint64_t hantek_5xxxb_get_memory_depth_from_sys_data(
	uint8_t store_depth)
{
	for (size_t i=0; i<ARRAY_SIZE(memory_depth_mapper); i++) {
		if (memory_depth_mapper[i].sys_data_store_depth_map == store_depth)
			return memory_depth_mapper[i].memory_depth;
	}
	return 0;
}

SR_PRIV uint8_t hantek_5xxxb_get_store_depth_from_memory_depth(
	uint64_t memory_depth)
{
	for (size_t i=0; i<ARRAY_SIZE(memory_depth_mapper); i++) {
		if (memory_depth_mapper[i].memory_depth == memory_depth)
			return memory_depth_mapper[i].sys_data_store_depth_map;
	}
	return 0; // TODO
}

SR_PRIV uint8_t hantek_5xxxb_get_store_depth_from_sample_rate_array_index(
	size_t sample_rate_array_index)
{
	for (size_t i=0; i<ARRAY_SIZE(memory_depth_mapper); i++) {
		if (memory_depth_mapper[i].sample_rate_array_index_map == sample_rate_array_index)
			return memory_depth_mapper[i].sys_data_store_depth_map;
	}
	return 0; // TODO
}

SR_PRIV size_t hantek_5xxxb_get_sample_rate_array_index_from_sys_data(
	uint8_t store_depth)
{
	for (size_t i=0; i<ARRAY_SIZE(memory_depth_mapper); i++) {
		if (memory_depth_mapper[i].sys_data_store_depth_map == store_depth)
			return memory_depth_mapper[i].sample_rate_array_index_map;
	}
	return 0; // TODO
}

SR_PRIV float hantek_5xxxb_get_volts_per_div(const struct sr_dev_inst *sdi,
	int channel_idx)
{
	struct dev_context *devc;
	uint8_t vb, probe;
	float vdiv;

	devc = sdi->priv;

	vb = devc->in_sys_data->vert_ch[channel_idx].vb;
	probe = devc->in_sys_data->vert_ch[channel_idx].probe;

	vdiv = ch_vdiv[probe][vb][0] / (float)ch_vdiv[probe][vb][1];
	if (devc->in_sys_data->vert_ch[channel_idx].fine) {
		if (vb == VB_5MV || vb == VB_50MV || vb == VB_500MV || vb == VB_5V) {
			/* 0x01 to 0x4A (75 steps) when between "factor 2.5x" levels */
			vdiv -= devc->in_sys_data->vert_ch[channel_idx].cnt_fine *
				((vdiv - (vdiv / 2.5)) / 75);
		} else {
			/* 0x00 to 0x31 (49 steps) when between "factor 2x" level */
			vdiv -= devc->in_sys_data->vert_ch[channel_idx].cnt_fine *
				((vdiv / 2) / 50);
		}
	}
	//vdiv *= probe_factor[devc->in_sys_data->vert_ch[channel_idx].probe]; // TODO: kann weg?

	return vdiv;
}

/**
 * The sample data / trigger position are post-processed from the image memory.
 * The span is 10.2 DIV vertical (-127 to 127, 510 pixels), that equals
 * 25 steps per DIV.
 */
SR_PRIV float hantek_5xxxb_get_value_from_vert_pos(
	const struct sr_dev_inst *sdi, int16_t pos, float vdiv, int channel_idx)
{
	struct dev_context *devc;
	int16_t raw_value;
	float value;

	devc = sdi->priv;

	/* pos - offset */
	raw_value = pos - devc->in_sys_data->vert_ch[channel_idx].pos;
	value = raw_value * (vdiv / 25);

	return value;
}

SR_PRIV int16_t hantek_5xxxb_get_vert_pos_from_value(
	const struct sr_dev_inst *sdi, float value, float vdiv, int channel_idx)
{
	struct dev_context *devc;
	int16_t pos;

	devc = sdi->priv;

	//sr_err("hantek_5xxxb_get_value_from_vert_pos(): vdiv = %f", vdiv);
	//sr_err("hantek_5xxxb_get_value_from_vert_pos(): value = %f", value);
	pos = (int8_t)(value / (vdiv / 25));
	//sr_err("hantek_5xxxb_get_value_from_vert_pos(): pos = %i", pos);
	pos += devc->in_sys_data->vert_ch[channel_idx].pos;
	//sr_err("hantek_5xxxb_get_value_from_vert_pos(): offset = %i", devc->in_sys_data->vert_ch[channel_idx].pos);
	//sr_err("hantek_5xxxb_get_value_from_vert_pos(): pos = %i", pos);

	return pos;
}

SR_PRIV int hantek_5xxxb_lock_panel(const struct sr_dev_inst *sdi,
	gboolean lock)
{
	uint8_t out_buf[7], in_buf[128];
	int in_size, ret;

	out_buf[0] = HANTEK_5XXXB_USB_NORM_MSG;
	out_buf[1] = 4;
	out_buf[2] = 0;
	out_buf[3] = HANTEK_5XXXB_CMD_LOCK;
	out_buf[4] = HANTEK_5XXXB_CMD_SUB_LOCK;
	out_buf[5] = lock;
	out_buf[6] = calc_checksum(out_buf, sizeof(out_buf)-1);
	ret = send_bulk(sdi, out_buf, sizeof(out_buf));
	if (ret != SR_OK)
		return ret;

	ret = receive_bulk(sdi, in_buf, &in_size, sizeof(in_buf),
		HANTEK_5XXXB_CMD_LOCK);
	if (ret != SR_OK)
		return ret;

	if (in_buf[4] != HANTEK_5XXXB_CMD_SUB_LOCK) {
		sr_err("Response sub command does not match (got 0x%X, expected 0x%X)",
			   in_buf[4], HANTEK_5XXXB_CMD_SUB_LOCK);
		return SR_ERR;
	}

	if (in_buf[5] != lock) {
		sr_err("Response is wrong (got 0x%X, expected 0x%X)", in_buf[5], lock);
		return SR_ERR;
	}

	return SR_OK;
}

SR_PRIV int hantek_5xxxb_get_sys_data(const struct sr_dev_inst *sdi,
	struct hantek_5xxxb_sys_data *sys_data)
{
	uint8_t out_buf[5], in_buf[512];
	int in_size, ret;

	/* Always lock panel before getting SysDATA */
	ret = hantek_5xxxb_lock_panel(sdi, TRUE);
	if (ret != SR_OK)
		return ret;

	out_buf[0] = HANTEK_5XXXB_USB_NORM_MSG;
	out_buf[1] = 2;
	out_buf[2] = 0;
	out_buf[3] = HANTEK_5XXXB_CMD_RD_SYSDATA;
	out_buf[4] = calc_checksum(out_buf, sizeof(out_buf)-1);
	ret = send_bulk(sdi, out_buf, sizeof(out_buf));
	if (ret != SR_OK) {
		/* Try to unlock panel */
		hantek_5xxxb_lock_panel(sdi, FALSE);
		return ret;
	}

	ret = receive_bulk(sdi, in_buf, &in_size, sizeof(in_buf),
		HANTEK_5XXXB_CMD_RD_SYSDATA);
	if (ret != SR_OK) {
		/* Try to unlock panel */
		hantek_5xxxb_lock_panel(sdi, FALSE);
		return ret;
	}

	/* Unlock panel */
	ret = hantek_5xxxb_lock_panel(sdi, FALSE);
	if (ret != SR_OK)
		return ret;

	memcpy(sys_data, in_buf+4, sizeof(struct hantek_5xxxb_sys_data));
	print_sys_data(sys_data);

	return SR_OK;
}

SR_PRIV int hantek_5xxxb_set_sys_data(const struct sr_dev_inst *sdi)
{
	struct dev_context *devc;
	uint8_t *out_buf, in_buf[32];
	int sys_data_size, in_size, ret;

	devc = sdi->priv;

	print_sys_data(devc->out_sys_data);

	sys_data_size = sizeof(struct hantek_5xxxb_sys_data);
	out_buf = g_malloc0(sys_data_size + 5);
	*(out_buf + 0) = HANTEK_5XXXB_USB_NORM_MSG;
	*(out_buf + 1) = sys_data_size + 2;
	*(out_buf + 2) = 0;
	*(out_buf + 3) = HANTEK_5XXXB_CMD_WR_SYSDATA;
	memcpy(out_buf + 4, devc->out_sys_data, sys_data_size);
	*(out_buf + 4 + sys_data_size) = calc_checksum(out_buf, sys_data_size+4);
	ret = send_bulk(sdi, out_buf, sys_data_size+5);
	if (ret != SR_OK)
		return ret;

	ret = receive_bulk(sdi, in_buf, &in_size, sizeof(in_buf),
		HANTEK_5XXXB_CMD_WR_SYSDATA);
	if (ret != SR_OK)
		return ret;

	/* Check return status byte */
	if (in_buf[4] != 0) {
		sr_err("Writing SysDATA failed with status %u", in_buf[4]);
		return SR_ERR;
	}

	return SR_OK;
}

SR_PRIV int hantek_5xxxb_get_sample_data(const struct sr_dev_inst *sdi,
	int channel_idx)
{
	struct dev_context *devc;
	struct sr_datafeed_packet packet;
	uint8_t out_buf[7], in_buf[10007];
	int in_size, ret;
	uint32_t sample_length, samples_read;

	devc = sdi->priv;

	if (!devc->in_sys_data->vert_ch[channel_idx].disp)
		return SR_OK;

	out_buf[0] = HANTEK_5XXXB_USB_NORM_MSG;
	out_buf[1] = 4;
	out_buf[2] = 0;
	out_buf[3] = HANTEK_5XXXB_CMD_RD_SAMPLEDATA;
	out_buf[4] = HANTEK_5XXXB_CMD_SUB_RD_SAMPLEDATA;
	out_buf[5] = channel_idx;
	out_buf[6] = calc_checksum(out_buf, sizeof(out_buf)-1);
	ret = send_bulk(sdi, out_buf, sizeof(out_buf));
	if (ret != SR_OK)
		return ret;

	/* Read sample data (status) packet */
	ret = receive_bulk(sdi, in_buf, &in_size, 128,
		HANTEK_5XXXB_CMD_RD_SAMPLEDATA);
	if (ret != SR_OK)
		return ret;

	if (in_buf[4] == 0x03) {
		sr_err("New data is not available for channel %i", channel_idx);
		return SR_OK;
	} else if (in_buf[4] != 0x00) {
		sr_err("Response sub command does not match (got 0x%X, expected 0x%X)",
			in_buf[4], 0x00);
		return SR_ERR;
	}

	/* Little endian */
	sample_length = in_buf[7] << 16 | in_buf[6] << 8 | in_buf[5];
	samples_read = 0;

	/* Tell the frontend a new frame is on the way. */
	packet.type = SR_DF_FRAME_BEGIN;
	sr_err("send_df_chunk(): send SR_DF_FRAME_BEGIN");
	sr_session_send(sdi, &packet);

	while (TRUE) {
		ret = receive_bulk(sdi, in_buf, &in_size, sizeof(in_buf),
			HANTEK_5XXXB_CMD_RD_SAMPLEDATA);
		if (ret != SR_OK)
			return ret;

		if (in_buf[4] == 0x02)
			break;
		else if (in_buf[4] == 0x03) {
			sr_err("Error while reading data for channel %i", channel_idx);
			continue; // TODO: or break?
		} else if (in_buf[4] != 0x01) {
			sr_err("Response sub command don't match (got 0x%X, expected 0x%X)",
				in_buf[4], 0x01);
			break;
			//return SR_ERR;
		}

		samples_read += in_size-7;
		send_df_chunk(sdi, in_buf+6, in_size-7, channel_idx);
	}

	if (sample_length != samples_read) {
		sr_err("hantek_5xxxb_get_sample_data(): sample_length != samples_read (%u != %u)",
			sample_length, samples_read);
	}
	//sr_err("hantek_5xxxb_get_sample_data(): samples_read = %u", samples_read);

	/* Mark the end of this frame. */
	packet.type = SR_DF_FRAME_END;
	sr_session_send(sdi, &packet);

	sr_sw_limits_update_samples_read(&devc->limits, samples_read);
	sr_sw_limits_update_frames_read(&devc->limits, 1);

	return SR_OK;
}

SR_PRIV int hantek_5xxxb_receive_data(int fd, int revents, void *cb_data)
{
	struct sr_dev_inst *sdi;
	struct dev_context *devc;
	struct sr_dev_driver *di;
	struct drv_context *drvc;
	struct hantek_5xxxb_sys_data *sys_data;
	int ret, cmp_result, ch_idx;
	uint8_t vb_idx, probe_idx;
	double vdiv, trigger_lvl;
	double trigger_hpos;
	uint64_t buffersize;

	(void)fd;
	(void)revents;

	if (!(sdi = cb_data))
		return TRUE;

	if (!(devc = sdi->priv))
		return TRUE;

	if (devc->dev_state == STOPPING) {
		di = sdi->driver;
		drvc = di->context;

		sr_dbg("Stopping acquisition.");
		usb_source_remove(sdi->session, drvc->sr_ctx);
		std_session_send_df_end(sdi);
		devc->dev_state = IDLE;
		return TRUE;
	}

	//if (revents != G_IO_IN)
	//	return TRUE;

	sys_data = g_malloc0(sizeof(struct hantek_5xxxb_sys_data));

	g_mutex_lock(&devc->rw_mutex);

	ret = hantek_5xxxb_get_sys_data(sdi, sys_data);
	if (ret != SR_OK) {
		g_mutex_unlock(&devc->rw_mutex);
		return FALSE;
	}

	/* Check if some of the common settings have changed. */
	cmp_result = memcmp(sys_data, devc->in_sys_data, sizeof(struct hantek_5xxxb_sys_data));
	if (cmp_result != 0) {
		sr_err("hantek_5xxxb_receive_data(): SysData has changed! pos = %i", cmp_result);

		/* Check channel vertical settings */
		for (ch_idx=0; ch_idx<2; ++ch_idx) {
			/* TODO: channel_group / configurable is missing for all sr_session_send_meta()! */
			if (sys_data->vert_ch[ch_idx].disp != devc->in_sys_data->vert_ch[ch_idx].disp) {
				sr_err("hantek_5xxxb_receive_data(): vert_ch[%i].disp = 0x%X", ch_idx, sys_data->vert_ch[ch_idx].disp);
				sr_session_send_meta(sdi, SR_CONF_ENABLED,
					g_variant_new_boolean(sys_data->vert_ch[ch_idx].disp));
			} else if (sys_data->vert_ch[ch_idx].vb != devc->in_sys_data->vert_ch[ch_idx].vb) {
				vb_idx = sys_data->vert_ch[ch_idx].vb;
				probe_idx = sys_data->vert_ch[ch_idx].probe;
				sr_err("hantek_5xxxb_receive_data(): vert_ch[%i].vb = 0x%X", ch_idx, sys_data->vert_ch[ch_idx].vb);
				sr_session_send_meta(sdi, SR_CONF_VDIV,
					g_variant_new("(tt)", ch_vdiv[probe_idx][vb_idx][0], ch_vdiv[probe_idx][vb_idx][1]));
			} else if (sys_data->vert_ch[ch_idx].coup != devc->in_sys_data->vert_ch[ch_idx].coup) {
				sr_err("hantek_5xxxb_receive_data(): vert_ch[%i].coup = 0x%X", ch_idx, sys_data->vert_ch[ch_idx].coup);
				sr_session_send_meta(sdi, SR_CONF_COUPLING,
					g_variant_new_string(ch_coupling[sys_data->vert_ch[ch_idx].coup]));
			} else if (sys_data->vert_ch[ch_idx].f20mhz != devc->in_sys_data->vert_ch[ch_idx].f20mhz) {
				sr_err("hantek_5xxxb_receive_data(): vert_ch[%i].f20mhz = 0x%X", ch_idx, sys_data->vert_ch[ch_idx].f20mhz);
				sr_session_send_meta(sdi, SR_CONF_FILTER,
					g_variant_new_boolean(sys_data->vert_ch[ch_idx].f20mhz));
			} else if (sys_data->vert_ch[ch_idx].probe != devc->in_sys_data->vert_ch[ch_idx].probe) {
				vb_idx = sys_data->vert_ch[ch_idx].vb;
				probe_idx = sys_data->vert_ch[ch_idx].probe;
				sr_err("hantek_5xxxb_receive_data(): vert_ch[%i].probe = 0x%X", ch_idx, sys_data->vert_ch[ch_idx].probe);
				sr_session_send_meta(sdi, SR_CONF_PROBE_FACTOR,
					g_variant_new_uint64(probe_factor[probe_idx]));
				sr_session_send_meta(sdi, SR_CONF_VDIV,
					g_variant_new("(tt)", ch_vdiv[probe_idx][vb_idx][0], ch_vdiv[probe_idx][vb_idx][1]));
			}
		}

		/* Check trigger settings. */
		if (sys_data->trig_src != devc->in_sys_data->trig_src) {
			sr_err("hantek_5xxxb_receive_data(): trig_src = 0x%X", sys_data->trig_src);
			sr_session_send_meta(sdi, SR_CONF_TRIGGER_SOURCE,
				g_variant_new_string(trigger_source[sys_data->trig_src]));
		} else if (sys_data->trig_edge_slope != devc->in_sys_data->trig_edge_slope) {
			sr_err("hantek_5xxxb_receive_data(): trig_edge_slope = 0x%X", sys_data->trig_edge_slope);
			sr_session_send_meta(sdi, SR_CONF_TRIGGER_SLOPE,
				g_variant_new_string(trigger_slope[sys_data->trig_edge_slope]));
		} else if (sys_data->trig_vpos != devc->in_sys_data->trig_vpos) {
			ch_idx = sys_data->trig_src;
			/* TODO
			if (ch_idx > 1) {
				ret = SR_ERR_NA;
				goto done;
			}
			*/
			vdiv = hantek_5xxxb_get_volts_per_div(sdi, ch_idx);
			trigger_lvl = hantek_5xxxb_get_value_from_vert_pos(sdi,
				sys_data->trig_vpos, vdiv, ch_idx);
			sr_err("hantek_5xxxb_receive_data(): trig_vpos = 0x%X", sys_data->trig_vpos);
			sr_session_send_meta(sdi, SR_CONF_TRIGGER_LEVEL,
				g_variant_new_double(trigger_lvl));
		} else if (sys_data->horiz_trigtime != devc->in_sys_data->horiz_trigtime) {
			sr_err("hantek_5xxxb_receive_data():  = %li", sys_data->horiz_trigtime);
			/* Trigger pos is in ps (1e12) */
			trigger_hpos = (double)sys_data->horiz_trigtime / 1e12;
			sr_session_send_meta(sdi, SR_CONF_HORIZ_TRIGGERPOS,
				g_variant_new_double(trigger_hpos));
		}

		/* Check horizontal settings. */
		if (sys_data->horiz_tb != devc->in_sys_data->horiz_tb ||
			sys_data->horiz_win_tb != devc->in_sys_data->horiz_win_tb) {
			sr_err("hantek_5xxxb_receive_data(): horiz_tb = 0x%X, horiz_win_tb = 0x%X", sys_data->horiz_tb, sys_data->horiz_win_tb);
			sr_session_send_meta(sdi, SR_CONF_TIMEBASE,
				g_variant_new("(tt)",
					win_timebase[sys_data->horiz_win_tb][0],
					win_timebase[sys_data->horiz_win_tb][1]));
			sr_session_send_meta(sdi, SR_CONF_SAMPLERATE,
				g_variant_new_uint64(hantek_5xxxb_get_samplerate(sys_data)));
		} else if (sys_data->control_disp_menu != devc->in_sys_data->control_disp_menu) {
			/*
			 * 16 DIV (640 pixels) with menu visible and 19.2 DIV (768 pixels)
			 * without menu.
			 */
			sr_err("hantek_5xxxb_receive_data(): control_disp_menu = 0x%X", sys_data->control_disp_menu);
			sr_session_send_meta(sdi, SR_CONF_NUM_HDIV,
				g_variant_new_int32(sys_data->control_disp_menu
					? HANTEK_5XXXB_NUM_HDIV_MENU_ON : HANTEK_5XXXB_NUM_HDIV_MENU_OFF_INT));
			// TODO: samplerate??
			sr_session_send_meta(sdi, SR_CONF_SAMPLERATE,
				g_variant_new_uint64(hantek_5xxxb_get_samplerate(sys_data)));
		} else if (sys_data->acqurie_store_depth != devc->in_sys_data->acqurie_store_depth) {
			sr_err("hantek_5xxxb_receive_data(): acqurie_store_depth = 0x%X", sys_data->acqurie_store_depth);

			/* TODO combine with function hantek_5xxxb_get_memory_depth_from_sys_data() */
			buffersize = 0;
			for (size_t i=0; i<ARRAY_SIZE(memory_depth_mapper); i++) {
				if (memory_depth_mapper[i].sys_data_store_depth_map == sys_data->acqurie_store_depth) {
					buffersize = memory_depth_mapper[i].memory_depth;
					break;
				}
			}
			sr_session_send_meta(sdi, SR_CONF_BUFFERSIZE,
				g_variant_new_uint64(buffersize));
			// TODO: samplerate??
		} else if (sys_data->acqurie_mode != devc->in_sys_data->acqurie_mode) {
			sr_session_send_meta(sdi, SR_CONF_AVERAGING,
				g_variant_new_boolean(sys_data->acqurie_mode == ACQ_MODE_AVG));
		} else if (sys_data->acqurie_avg_cnt != devc->in_sys_data->acqurie_avg_cnt) {
			sr_session_send_meta(sdi, SR_CONF_AVG_SAMPLES,
				g_variant_new_uint64(average_count[sys_data->acqurie_avg_cnt]));
		}

		/* Set changed SysDATA to device instance */
		// TODO: Free? memcpy? mutex when get while acqu?
		devc->in_sys_data = sys_data;
	}

	ret = hantek_5xxxb_get_sample_data(sdi, 0);
	if (ret != SR_OK) {
		g_mutex_unlock(&devc->rw_mutex);
		return FALSE;
	}

	ret = hantek_5xxxb_get_sample_data(sdi, 1);
	if (ret != SR_OK) {
		g_mutex_unlock(&devc->rw_mutex);
		return FALSE;
	}

	g_mutex_unlock(&devc->rw_mutex);

	if (sr_sw_limits_check(&devc->limits))
		sr_dev_acquisition_stop(sdi);

	return TRUE;
}
