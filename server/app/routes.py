"""REST-API som web- og mobilappen bruker."""
from fastapi import APIRouter, HTTPException, Query

from app import database
from app.models import ConfigRequest, WaterRequest
from app.mqtt_bridge import bridge

router = APIRouter(prefix="/api")


def build_alerts(data: dict | None) -> list[dict]:
    alerts = []
    if not bridge.connected:
        alerts.append({"level": "danger", "text": "Serveren er ikke koblet til MQTT-brokeren"})
    if not bridge.esp_online:
        alerts.append({"level": "danger", "text": "ESP32 er offline"})
    if not data:
        return alerts

    if not data.get("water_sensor_ok", True):
        alerts.append({"level": "danger", "text": "Feil på vannivåsensoren – pumpen er stoppet"})
    elif not data.get("water_ok", True):
        alerts.append({"level": "danger", "text": "Lavt vannivå – fyll på tanken"})

    if not data.get("soil_ok", True):
        alerts.append({"level": "warning", "text": "Feil på jordfuktighetssensoren"})
    elif data.get("soil", 100) < data.get("threshold", 0):
        alerts.append({"level": "warning", "text": "Planten trenger vann"})
    return alerts


def send(cmd: dict) -> dict:
    if not bridge.send_command(cmd):
        raise HTTPException(503, "Ikke koblet til MQTT-brokeren")
    return {"ok": True, "sent": cmd}


@router.get("/status")
def status():
    return {
        "online": bridge.esp_online,
        "mqtt_connected": bridge.connected,
        "last_seen": bridge.last_seen or None,
        "data": bridge.latest,
        "alerts": build_alerts(bridge.latest),
    }


@router.get("/history")
def history(hours: int = Query(24, ge=1, le=24 * 30)):
    return database.get_history(hours)


@router.get("/events")
def events(limit: int = Query(30, ge=1, le=500)):
    return database.get_events(limit)


@router.post("/water")
def water(req: WaterRequest):
    return send({"cmd": "water", "seconds": req.seconds})


@router.post("/stop")
def stop():
    return send({"cmd": "stop"})


@router.post("/config")
def set_config(req: ConfigRequest):
    return send({"cmd": "config", **req.model_dump(exclude_none=True)})
