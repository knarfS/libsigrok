#include <glib.h>
#include <libsigrok/libsigrok.h>
#include "libsigrok-internal.h"

/** @cond PRIVATE */
#define LOG_PREFIX "command-queue"
/** @endcond */


SR_PRIV struct sr_cmd_queue *sr_cmd_queue_new(struct sr_dev_inst *sdi)
{
	struct sr_cmd_queue *queue;

	queue = g_malloc0(sizeof(struct sr_cmd_queue));
	queue->queue = g_queue_new();
	g_mutex_init(&queue->mutex);
	g_cond_init(&queue->command_processed);
	queue->sdi = sdi;
	queue->is_processing = FALSE;

	return queue;
}

SR_PRIV void sr_cmd_queue_free(struct sr_cmd_queue *queue)
{
	if (!queue)
		return;

	// Clear any remaining items in the queue
	while (!g_queue_is_empty(queue->queue)) {
		struct sr_cmd_queue_item *item = g_queue_pop_head(queue->queue);

		// Free any allocated data
		if (item->get_config_data)
			g_variant_unref(*item->get_config_data);
		if (item->set_config_data)
			g_variant_unref(item->set_config_data);

		g_free(item);
	}

	// Free the queue itself
	g_queue_free(queue->queue);

	// Destroy mutex and condition variable
	g_mutex_clear(&queue->mutex);
	g_cond_clear(&queue->command_processed);

	// Free the queue structure
	g_free(queue);
}

SR_PRIV gboolean sr_cmd_queue_process(gpointer user_data)
{
	struct sr_cmd_queue *queue = user_data;

	g_mutex_lock(&queue->mutex);

	if (queue->is_processing || g_queue_is_empty(queue->queue)) {
		g_mutex_unlock(&queue->mutex);
		return G_SOURCE_CONTINUE;
	}

	struct sr_cmd_queue_item *item = g_queue_peek_head(queue->queue);
	queue->is_processing = TRUE;

	g_mutex_unlock(&queue->mutex);

	switch (item->type) {
		case SR_CMD_CONFIG_SET:
			item->ret = queue->sdi->driver->config_set(
				item->key, item->set_config_data, queue->sdi, item->cg);
			break;

		case SR_CMD_CONFIG_GET:
			item->ret = queue->sdi->driver->config_get(
				item->key, item->get_config_data, queue->sdi, item->cg);
			break;

		case SR_CMD_CONFIG_LIST:
			// Not implemented yet
			break;
	}

	g_mutex_lock(&queue->mutex);
	g_queue_pop_head(queue->queue);
	queue->is_processing = FALSE;
	g_cond_signal(&queue->command_processed);
	g_mutex_unlock(&queue->mutex);

	return G_SOURCE_CONTINUE;
}
