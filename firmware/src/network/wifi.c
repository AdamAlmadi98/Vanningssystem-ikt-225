#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/dhcpv4.h>
#include <zephyr/logging/log.h>

#include "network/wifi.h"
#include "config/app_config.h"

LOG_MODULE_REGISTER(wifi, LOG_LEVEL_INF);

static struct net_mgmt_event_callback wifi_cb;
static struct net_mgmt_event_callback ipv4_cb;
static K_SEM_DEFINE(got_ip, 0, 1);
static volatile bool connected;

/* Merk: i Zephyr 4.2+ er "event" uint64_t. Bruk uint32_t i eldre versjoner. */
static void wifi_event_handler(struct net_mgmt_event_callback *cb, uint64_t event,
			       struct net_if *iface)
{
	if (event == NET_EVENT_WIFI_DISCONNECT_RESULT) {
		LOG_WRN("Wi-Fi frakoblet");
		connected = false;
	}
}

static void ipv4_event_handler(struct net_mgmt_event_callback *cb, uint64_t event,
			       struct net_if *iface)
{
	if (event == NET_EVENT_IPV4_ADDR_ADD) {
		LOG_INF("Fikk IP-adresse");
		connected = true;
		k_sem_give(&got_ip);
	}
}

void wifi_init(void)
{
	net_mgmt_init_event_callback(&wifi_cb, wifi_event_handler, NET_EVENT_WIFI_DISCONNECT_RESULT);
	net_mgmt_add_event_callback(&wifi_cb);
	net_mgmt_init_event_callback(&ipv4_cb, ipv4_event_handler, NET_EVENT_IPV4_ADDR_ADD);
	net_mgmt_add_event_callback(&ipv4_cb);
}

int wifi_connect(void)
{
	struct net_if *iface = net_if_get_default();
	struct wifi_connect_req_params params = {
		.ssid = (const uint8_t *)WIFI_SSID,
		.ssid_length = strlen(WIFI_SSID),
		.psk = (const uint8_t *)WIFI_PSK,
		.psk_length = strlen(WIFI_PSK),
		.security = WIFI_SECURITY_TYPE_PSK,
		.channel = WIFI_CHANNEL_ANY,
		.band = WIFI_FREQ_BAND_2_4_GHZ,
		.mfp = WIFI_MFP_OPTIONAL,
	};

	LOG_INF("Kobler til Wi-Fi \"%s\" ...", WIFI_SSID);
	k_sem_reset(&got_ip);

	int err = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &params, sizeof(params));
	if (err) {
		LOG_ERR("Wi-Fi-tilkobling feilet (%d)", err);
		return err;
	}
	net_dhcpv4_start(iface);

	if (k_sem_take(&got_ip, K_SECONDS(30)) != 0) {
		LOG_ERR("Fikk ikke IP-adresse");
		return -ETIMEDOUT;
	}
	return 0;
}

bool wifi_is_connected(void)
{
	return connected;
}
