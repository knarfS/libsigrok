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

static struct sr_dev_driver bosch_glm_driver_info;

static const uint32_t scanopts[] = {
	SR_CONF_CONN,
};

static const uint32_t drvopts[] = {
	SR_CONF_MULTIMETER, // TODO
};

static const uint32_t devopts[] = {
	SR_CONF_CONTINUOUS,
	SR_CONF_LIMIT_SAMPLES | SR_CONF_SET | SR_CONF_LIST,
	SR_CONF_LIMIT_MSEC | SR_CONF_SET | SR_CONF_LIST,
};


static GSList *scan(struct sr_dev_driver *di, GSList *options)
{
	struct sr_bt_desc *desc;
	const char *conn;
	struct sr_config *src;
	GSList *l;
	int ret;
	struct dev_context *devc;

	conn = NULL;
	for (l = options; l; l = l->next) {
		src = l->data;
		switch (src->key) {
		case SR_CONF_CONN:
			conn = g_variant_get_string(src->data, NULL);
			break;
		}
	}

	if (!conn)
		return NULL;

	//desc = sr_bt_desc_new();
	//if (!desc)
	//	return NULL;
    //
	//ret = sr_bt_config_addr_remote(desc, conn);
	//if (ret < 0)
	//	goto err;
    //
	///*
	// * GLM 50C:  0x0005
	// * GLM 100C: 0x0001
	// */
	//ret = sr_bt_config_rfcomm(desc, 0x0005);
	//if (ret < 0)
	//	goto err;
    //
	//ret = sr_bt_connect_rfcomm(desc);
	//if (ret < 0)
	//	goto err;
	//sr_bt_disconnect(desc);

	struct sr_dev_inst *sdi = g_malloc0(sizeof(*sdi)); // TODO
	devc = g_malloc0(sizeof(*devc));

	sdi->priv = devc;
	//sdi->inst_type = SR_INST_USER;
	sdi->status = SR_ST_INITIALIZING;
	sdi->connection_id = g_strdup(conn);
	sdi->conn = desc;
	sdi->vendor = g_strdup("Bosch");
	sdi->model = g_strdup("GLM 50C"); // TODO

	sr_sw_limits_init(&devc->limits);
	devc->rfcomm_channel = 0x0005; // TODO

	sr_channel_new(sdi, 0, SR_CHANNEL_ANALOG, TRUE, "CH");

	return std_scan_complete(di, g_slist_prepend(NULL, sdi));

err:
	sr_bt_desc_free(desc);
	return NULL;
}

static int dev_open(struct sr_dev_inst *sdi)
{
	struct dev_context *devc;
	struct sr_bt_desc *desc;
	int ret;

	devc = sdi->priv;
	desc = sdi->conn;

	desc = sr_bt_desc_new();
	if (!desc)
		return SR_ERR;

	ret = sr_bt_config_addr_remote(desc, "00:13:43:B5:37:89");
	if (ret < 0)
		return SR_ERR;

	ret = sr_bt_config_rfcomm(desc, devc->rfcomm_channel);
	if (ret < 0)
		return SR_ERR;

	ret = sr_bt_connect_rfcomm(desc);
	if (ret < 0)
		return SR_ERR;

	sdi->status = SR_ST_ACTIVE;
	sdi->conn = desc;

	return SR_OK;
}

static int dev_close(struct sr_dev_inst *sdi)
{
	struct sr_bt_desc *desc;

	desc = sdi->conn;

	sr_bt_disconnect(desc);

	return SR_OK;
}

static int config_get(uint32_t key, GVariant **data,
	const struct sr_dev_inst *sdi, const struct sr_channel_group *cg)
{
	struct dev_context *devc;

	(void)cg;

	devc = sdi->priv;

	return sr_sw_limits_config_get(&devc->limits, key, data);
}

static int config_set(uint32_t key, GVariant *data,
	const struct sr_dev_inst *sdi, const struct sr_channel_group *cg)
{
	struct dev_context *devc;

	(void)cg;

	devc = sdi->priv;

	return sr_sw_limits_config_set(&devc->limits, key, data);
}

static int config_list(uint32_t key, GVariant **data,
	const struct sr_dev_inst *sdi, const struct sr_channel_group *cg)
{
	return STD_CONFIG_LIST(key, data, sdi, cg, scanopts, drvopts, devopts);
}

static int dev_acquisition_start(const struct sr_dev_inst *sdi)
{
	struct dev_context *devc;
	int ret;

	devc = sdi->priv;

	sr_sw_limits_acquisition_start(&devc->limits);
	std_session_send_df_header(sdi);

	if ((ret = sr_session_source_add(sdi->session, -1, 0, 500,
			bosch_glm_receive_data, (void *)sdi)) != SR_OK)
		return ret;

	//if ((ret = sr_scpi_source_add(sdi->session, scpi, G_IO_IN, 10,
	//		bosch_glm_receive_data, (void *)sdi)) != SR_OK)
	//	return ret;

	return SR_OK;
}

static int dev_acquisition_stop(struct sr_dev_inst *sdi)
{
	sr_session_source_remove(sdi->session, -1);

	std_session_send_df_end(sdi);

	return SR_OK;
}

static struct sr_dev_driver bosch_glm_driver_info = {
	.name = "bosch-glm",
	.longname = "Bosch GLM",
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
SR_REGISTER_DEV_DRIVER(bosch_glm_driver_info);
