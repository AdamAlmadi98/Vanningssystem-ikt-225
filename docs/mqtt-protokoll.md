# MQTT-protokoll

Broker: Mosquitto på port 1883. Alle meldinger er JSON.

| Topic | Retning | Innhold |
|---|---|---|
| `vanning/sensor` | ESP32 → server | Målinger og innstillinger, hvert 10. sekund og når pumpen endrer seg |
| `vanning/event` | ESP32 → server | Hendelser: vanning, alarmer, sikkerhetsstopp |
| `vanning/cmd` | server → ESP32 | Kommandoer fra appen |

## vanning/sensor

```json
{
  "soil": 42, "soil_raw": 2280, "soil_ok": true,
  "temp": 22.5, "hum": 41.0, "aht_ok": true,
  "water_ok": true, "water_sensor_ok": true, "pump": false,
  "auto": true, "threshold": 30, "interval_h": 5, "seconds": 3,
  "water_count": 12, "uptime_s": 3600
}
```

## vanning/event

| type | Felter | Betydning |
|---|---|---|
| `boot` | – | ESP32 har startet |
| `watered` | `reason` (manual/auto/schedule), `stopped` (done/user), `ms` | Vanning fullført |
| `safety_stop` | `reason`, `stopped` (low_water/water_sensor_fault), `ms` | Pumpen ble stoppet av sikkerhetshensyn |
| `rejected` | `reason` | Manuell vanning ble nektet |
| `low_water` | – | Vannivået ble lavt |
| `needs_water` | `soil` | Jorden ble tørrere enn grensen |
| `schedule_skipped` | `soil` | Tidsplanen hoppet over fordi jorden er våt |

## vanning/cmd

```json
{"cmd": "water", "seconds": 5}
{"cmd": "stop"}
{"cmd": "config", "auto": true, "threshold": 30, "interval_h": 5, "seconds": 3}
```

Alle felter i `config` er valgfrie. Pumpen går aldri lenger enn `PUMP_MAX_SECONDS` (15 s).
