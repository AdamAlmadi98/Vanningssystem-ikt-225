"""Innstillinger for serveren. Kan overstyres med miljøvariabler."""
import os
from pathlib import Path

SERVER_DIR = Path(__file__).resolve().parent.parent
WEB_DIR = SERVER_DIR.parent / "web"

MQTT_HOST = os.getenv("MQTT_HOST", "localhost")
MQTT_PORT = int(os.getenv("MQTT_PORT", "1883"))

DB_PATH = Path(os.getenv("DB_PATH", SERVER_DIR / "data" / "vanning.db"))

TOPIC_SENSOR = "vanning/sensor"
TOPIC_EVENT = "vanning/event"
TOPIC_CMD = "vanning/cmd"

# ESP32 regnes som offline hvis vi ikke har hørt fra den på så mange sekunder
OFFLINE_AFTER_S = 60
