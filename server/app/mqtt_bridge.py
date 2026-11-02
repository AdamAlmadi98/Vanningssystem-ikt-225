"""Kobler serveren til Mosquitto: lagrer meldinger fra ESP32 og sender kommandoer."""
import json
import logging
import time

import paho.mqtt.client as mqtt

from app import config, database

log = logging.getLogger("mqtt")


class MqttBridge:
    def __init__(self) -> None:
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="vanning-server")
        self.client.on_connect = self._on_connect
        self.client.on_disconnect = self._on_disconnect
        self.client.on_message = self._on_message
        self.connected = False
        self.latest: dict | None = None
        self.last_seen: float = 0.0

    def start(self) -> None:
        # connect_async + loop_start: serveren starter selv om brokeren ikke kjører ennå
        self.client.connect_async(config.MQTT_HOST, config.MQTT_PORT)
        self.client.loop_start()

    def stop(self) -> None:
        self.client.loop_stop()
        self.client.disconnect()

    def send_command(self, cmd: dict) -> bool:
        if not self.connected:
            return False
        info = self.client.publish(config.TOPIC_CMD, json.dumps(cmd))
        return info.rc == mqtt.MQTT_ERR_SUCCESS

    @property
    def esp_online(self) -> bool:
        return time.time() - self.last_seen < config.OFFLINE_AFTER_S

    def _on_connect(self, client, userdata, flags, reason_code, properties):
        if reason_code.is_failure:
            log.error("MQTT-tilkobling feilet: %s", reason_code)
            return
        log.info("Koblet til MQTT %s:%s", config.MQTT_HOST, config.MQTT_PORT)
        self.connected = True
        client.subscribe([(config.TOPIC_SENSOR, 0), (config.TOPIC_EVENT, 0)])

    def _on_disconnect(self, client, userdata, flags, reason_code, properties):
        log.warning("MQTT frakoblet: %s", reason_code)
        self.connected = False

    def _on_message(self, client, userdata, msg):
        try:
            payload = json.loads(msg.payload)
        except (json.JSONDecodeError, UnicodeDecodeError):
            log.warning("Ugyldig JSON på %s", msg.topic)
            return

        self.last_seen = time.time()
        if msg.topic == config.TOPIC_SENSOR:
            self.latest = payload
            database.add_measurement(payload)
        elif msg.topic == config.TOPIC_EVENT:
            database.add_event(payload)


bridge = MqttBridge()
