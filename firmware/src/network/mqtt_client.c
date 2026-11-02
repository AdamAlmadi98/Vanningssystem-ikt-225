#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>
#include <zephyr/logging/log.h>

#include "network/mqtt_client.h"
#include "control/commands.h"
#include "config/app_config.h"

LOG_MODULE_REGISTER(mqtt_app, LOG_LEVEL_INF);

static struct mqtt_client client;
static struct sockaddr_storage broker;
static uint8_t rx_buf[512];
static uint8_t tx_buf[512];
static char payload_buf[256];
static bool connected;
static uint16_t msg_id;

static void subscribe(void)
{
	struct mqtt_topic topic = {
		.topic = { .utf8 = (const uint8_t *)MQTT_TOPIC_CMD, .size = strlen(MQTT_TOPIC_CMD) },
		.qos = MQTT_QOS_0_AT_MOST_ONCE,
	};
	struct mqtt_subscription_list list = {
		.list = &topic,
		.list_count = 1,
		.message_id = ++msg_id,
	};

	mqtt_subscribe(&client, &list);
}

static void handle_publish(struct mqtt_client *c, const struct mqtt_publish_param *p)
{
	size_t len = p->message.payload.len;

	if (len >= sizeof(payload_buf)) {
		/* For stor melding: les og kast */
		uint8_t dump[32];
		while (len > 0) {
			int n = mqtt_read_publish_payload_blocking(c, dump, MIN(len, sizeof(dump)));
			if (n <= 0) {
				return;
			}
			len -= n;
		}
		LOG_WRN("For stor kommando ignorert");
		return;
	}

	if (mqtt_readall_publish_payload(c, payload_buf, len) != 0) {
		return;
	}
	payload_buf[len] = '\0';
	commands_handle(payload_buf, len);
}

static void mqtt_evt_handler(struct mqtt_client *c, const struct mqtt_evt *evt)
{
	switch (evt->type) {
	case MQTT_EVT_CONNACK:
		if (evt->result == 0) {
			LOG_INF("Koblet til MQTT-broker");
			connected = true;
			subscribe();
		}
		break;
	case MQTT_EVT_DISCONNECT:
		LOG_WRN("MQTT frakoblet");
		connected = false;
		break;
	case MQTT_EVT_PUBLISH:
		handle_publish(c, &evt->param.publish);
		break;
	default:
		break;
	}
}

static void abort_connection(void)
{
	mqtt_abort(&client);
	connected = false;
}

int mqtt_app_connect(void)
{
	struct sockaddr_in *b = (struct sockaddr_in *)&broker;

	b->sin_family = AF_INET;
	b->sin_port = htons(MQTT_BROKER_PORT);
	zsock_inet_pton(AF_INET, MQTT_BROKER_IP, &b->sin_addr);

	mqtt_client_init(&client);
	client.broker = &broker;
	client.evt_cb = mqtt_evt_handler;
	client.client_id.utf8 = (const uint8_t *)MQTT_CLIENT_ID;
	client.client_id.size = strlen(MQTT_CLIENT_ID);
	client.protocol_version = MQTT_VERSION_3_1_1;
	client.rx_buf = rx_buf;
	client.rx_buf_size = sizeof(rx_buf);
	client.tx_buf = tx_buf;
	client.tx_buf_size = sizeof(tx_buf);
	client.transport.type = MQTT_TRANSPORT_NON_SECURE;

	LOG_INF("Kobler til MQTT %s:%d ...", MQTT_BROKER_IP, MQTT_BROKER_PORT);
	int err = mqtt_connect(&client);
	if (err) {
		LOG_ERR("mqtt_connect feilet (%d)", err);
		return err;
	}

	/* Vent på CONNACK i opptil 3 sekunder */
	for (int i = 0; i < 30 && !connected; i++) {
		mqtt_app_process(100);
	}
	if (!connected) {
		abort_connection();
		return -ETIMEDOUT;
	}
	return 0;
}

bool mqtt_app_is_connected(void)
{
	return connected;
}

void mqtt_app_process(int timeout_ms)
{
	struct zsock_pollfd fds = {
		.fd = client.transport.tcp.sock,
		.events = ZSOCK_POLLIN,
	};

	int rc = zsock_poll(&fds, 1, timeout_ms);
	if (rc < 0 || (fds.revents & (ZSOCK_POLLERR | ZSOCK_POLLHUP | ZSOCK_POLLNVAL))) {
		abort_connection();
		return;
	}
	if (rc > 0 && (fds.revents & ZSOCK_POLLIN)) {
		if (mqtt_input(&client) != 0) {
			abort_connection();
			return;
		}
	}

	rc = mqtt_live(&client);
	if (rc != 0 && rc != -EAGAIN) {
		abort_connection();
	}
}

int mqtt_app_publish(const char *topic, const char *payload)
{
	struct mqtt_publish_param p = { 0 };

	if (!connected) {
		return -ENOTCONN;
	}
	p.message.topic.qos = MQTT_QOS_0_AT_MOST_ONCE;
	p.message.topic.topic.utf8 = (const uint8_t *)topic;
	p.message.topic.topic.size = strlen(topic);
	p.message.payload.data = (uint8_t *)payload;
	p.message.payload.len = strlen(payload);
	p.message_id = ++msg_id;

	int err = mqtt_publish(&client, &p);
	if (err) {
		abort_connection();
	}
	return err;
}
