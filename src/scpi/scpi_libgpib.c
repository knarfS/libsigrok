/*
 * This file is part of the libsigrok project.
 *
 * Copyright (C) 2014 Martin Ling <martin-sigrok@earth.li>
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

#include <gio/gio.h>
#include <glib/gtypes.h>
#include <glib/gerror.h>

#include <config.h>
#include <gpib/ib.h>
#include <string.h>
#include <libsigrok/libsigrok.h>
#include "libsigrok-internal.h"
#include "scpi.h"

#define LOG_PREFIX "scpi_gpib"

struct scpi_gpib {
	char *name;
	int descriptor;
	int board_descriptor;
	int read_started;
};

/** Closure for the ibwait() call’s parameters. */
typedef struct {
  int ud;
} ibwait_params;


static int scpi_gpib_dev_inst_new(void *priv, struct drv_context *drvc,
		const char *resource, char **params, const char *serialcomm)
{
	struct scpi_gpib *gscpi = priv;

	(void)drvc;
	(void)resource;
	(void)serialcomm;

	if (!params || !params[1])
			return SR_ERR;

	gscpi->name = g_strdup(params[1]);

	return SR_OK;
}

static int scpi_gpib_open(struct sr_scpi_dev_inst *scpi)
{
	int board_desc;
	struct scpi_gpib *gscpi = scpi->priv;

	if ((gscpi->descriptor = ibfind(gscpi->name)) < 0)
		return SR_ERR;

	// Get board/controller index (IbaBNA / minor number) for device.
	ibask(gscpi->descriptor, IbaBNA, &board_desc);
	gscpi->board_descriptor = board_desc;
	sr_spew("scpi_gpib_open(): device_descriptor = %i, board_descriptor = %i",
		gscpi->descriptor, board_desc);

	// Init automatic serial poll (for SRQ) for board
	// TODO: Auto poll doesn't work for HP/Agilent 82357B
	//ibconfig(board_desc, IbcAUTOPOLL, 1);
	ibconfig(board_desc, IbcAUTOPOLL, 0);

	return SR_OK;
}

static int scpi_gpib_source_add(struct sr_session *session, void *priv,
		int events, int timeout, sr_receive_data_callback cb, void *cb_data)
{
	(void) priv;

	/* Hook up a dummy handler to receive data from the device. */
	return sr_session_source_add(session, -1, events, timeout, cb, cb_data);
}

static int scpi_gpib_source_remove(struct sr_session *session, void *priv)
{
	(void) priv;

	return sr_session_source_remove(session, -1);
}

static int scpi_gpib_send(void *priv, const char *command)
{
	struct scpi_gpib *gscpi = priv;
	int len = strlen(command);

	ibwrt(gscpi->descriptor, command, len);

	if (ibsta & ERR)
	{
		sr_err("Error while sending SCPI command: '%s': iberr = %s.",
			command, gpib_error_string(iberr));
		return SR_ERR;
	}

	if (ibcnt < len)
	{
		sr_err("Failed to send all of SCPI command: '%s': "
				"len = %d, ibcnt = %d.", command, len, ibcnt);
		return SR_ERR;
	}

	sr_spew("Successfully sent SCPI command: '%s'.", command);

	return SR_OK;
}

static int scpi_gpib_read_begin(void *priv)
{
	struct scpi_gpib *gscpi = priv;

	gscpi->read_started = 0;

	return SR_OK;
}

static int scpi_gpib_read_data(void *priv, char *buf, int maxlen)
{
	struct scpi_gpib *gscpi = priv;

	ibrd(gscpi->descriptor, buf, maxlen);

	if (ibsta & ERR)
	{
		sr_err("Error while reading SCPI response: "
			"iberr = %s, ibsta = %d.",
			gpib_error_string(iberr), ibsta);
		return SR_ERR;
	}

	gscpi->read_started = 1;

	return ibcnt;
}

static int scpi_gpib_read_complete(void *priv)
{
	struct scpi_gpib *gscpi = priv;

	return gscpi->read_started && (ibsta & END);
}

static void scpi_gpib_waitsrq_data_free(ibwait_params *params)
{
	sr_spew("scpi_gpib_waitsrq_data_free(): Start");
	//g_slice_free(ibwait_params, params);
	g_free(params);
}

static void scpi_gpib_waitsrq_thread_cb(GTask *task, gpointer source_object,
	gpointer task_params, GCancellable *cancellable)
{
	(void)source_object;
	(void)cancellable;
	ibwait_params *params = task_params;
	int status_mask;
	char *status_register;
	int retval;

	/* Handle cancellation. */
	if (g_task_return_error_if_cancelled(task)) {
	  return;
	}

	/* TEST */
	status_register = g_malloc0(10);
	ibrsp(params->ud, status_register);
	if (ibsta & ERR) {
		sr_err("Error while serial polling: iberr = %s, ibsta = %d.",
			gpib_error_string(iberr), ibsta);
		return; // SR_ERR;
	}
	sr_spew("scpi_gpib_waitsrq_thread_cb(): status_register = %s", status_register);
	if (sr_atoi(status_register, &retval) == SR_OK) {
		sr_spew("scpi_gpib_waitsrq_thread_cb(): status_register = %i", retval);
	}
	else {
		sr_spew("scpi_gpib_waitsrq_thread_cb(): sr_atoi failed");
	}
	g_free(status_register);

	/* Run the blocking function. */
	sr_spew("scpi_gpib_waitsrq_thread_cb(): Run the blocking function for ud = %i", params->ud);
	status_mask = RQS; // | CMPL;
	ibwait(params->ud, status_mask);
	sr_spew("scpi_gpib_waitsrq_thread_cb(): Blocking function returned.");
	if (ibsta & ERR) {
		sr_err("Error while waiting for SRQ: iberr = %s, ibsta = %d.",
			gpib_error_string(iberr), ibsta);
		return; // SR_ERR;
	}

	status_register = g_malloc0(10);
	//status_register = NULL;
	ibrsp(params->ud, status_register);
	if (ibsta & ERR) {
		sr_err("Error while serial polling: iberr = %s, ibsta = %d.",
			gpib_error_string(iberr), ibsta);
		return; // SR_ERR;
	}

	if (sr_atoi(status_register, &retval) == SR_OK) {}

	g_free(status_register);

	g_task_return_int(task, retval);
}

void scpi_gpib_waitsrq_async(void *priv,
	GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data)
{
	struct scpi_gpib *gscpi = priv;

	GTask *task = NULL;  /* owned */
	ibwait_params *params = NULL;  /* owned */

	//g_return_if_fail(validate_param(gscpi->descriptor));
	//g_return_if_fail(validate_param(status_mask));
	g_return_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable));

	sr_spew("scpi_gpib_waitsrq_async(): Start");

	task = g_task_new(NULL, cancellable, callback, user_data);
	g_task_set_source_tag(task, scpi_gpib_waitsrq_async);

	/* Cancellation should be handled manually using mechanisms specific to
	* some_blocking_function(). */
	g_task_set_return_on_cancel(task, FALSE);

	/* Set up a closure containing the call’s parameters. Copy them to avoid
	* locking issues between the calling thread and the worker thread. */
	params = g_new0(ibwait_params, 1);
	params->ud = gscpi->descriptor;

	g_task_set_task_data(task, params, (GDestroyNotify)scpi_gpib_waitsrq_data_free);

	/* Run the task in a worker thread and return immediately while that continues
	* in the background. When it’s done it will call @callback in the current
	* thread default main context. */
	g_task_run_in_thread(task, scpi_gpib_waitsrq_thread_cb);

	g_object_unref(task);
}

int scpi_gpib_waitsrq_finish(GAsyncResult *result, GError **error)
{
  g_return_val_if_fail(g_task_is_valid(result, scpi_gpib_waitsrq_async), -1);
  g_return_val_if_fail(error == NULL || *error == NULL, -1);

  return g_task_propagate_int(G_TASK(result), error);
}



static int scpi_gpib_close(struct sr_scpi_dev_inst *scpi)
{
	struct scpi_gpib *gscpi = scpi->priv;

	/* Put device in back local mode to prevent lock-out of front panel. */
	ibloc(gscpi->descriptor);
	/* Now it's safe to close the handle. */
	ibonl(gscpi->descriptor, 0);

	return SR_OK;
}

static void scpi_gpib_free(void *priv)
{
	struct scpi_gpib *gscpi = priv;

	g_free(gscpi->name);
}

SR_PRIV const struct sr_scpi_dev_inst scpi_libgpib_dev = {
	.name = "GPIB",
	.prefix = "libgpib",
	.priv_size = sizeof(struct scpi_gpib),
	.dev_inst_new = scpi_gpib_dev_inst_new,
	.open = scpi_gpib_open,
	.source_add = scpi_gpib_source_add,
	.source_remove = scpi_gpib_source_remove,
	.send = scpi_gpib_send,
	.read_begin = scpi_gpib_read_begin,
	.read_data = scpi_gpib_read_data,
	.read_complete = scpi_gpib_read_complete,
	.close = scpi_gpib_close,
	.free = scpi_gpib_free,
};
