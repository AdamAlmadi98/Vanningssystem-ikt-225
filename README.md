# Automatisk vanningssystem

Én plante, én vanntank, én pumpe, sensorer, ESP32-S3 TFT og en web-/mobilapp.

```
ESP32-S3 (Zephyr)  ──MQTT──►  Mosquitto  ◄──MQTT──  Server (FastAPI + SQLite)  ◄──HTTP──  Web/mobil-app
 sensorer, pumpe, skjerm                              lagrer målinger, REST-API             web/
```

## Mappestruktur

```
vanningsystem/
├── firmware/                 ESP32-S3 – Zephyr RTOS + C
│   ├── CMakeLists.txt
│   ├── prj.conf              Zephyr-innstillinger (Wi-Fi, MQTT, skjerm …)
│   ├── app.overlay           Pinnekart (hvilke GPIO-er brukes til hva)
│   └── src/
│       ├── main.c            Starter modulene og kjører hovedløkken
│       ├── config/           app_config.h (alle innstillinger) + secrets.h (Wi-Fi)
│       ├── core/             Felles tilstand og hendelseskø
│       ├── sensors/          soil_sensor, water_level, aht20 (+ sensors.c som samler dem)
│       ├── actuators/        pump (TB6612FNG)
│       ├── control/          watering (auto, tidsplan, sikkerhet) og commands (fra appen)
│       ├── display/          TFT-skjermen (LVGL)
│       └── network/          wifi, mqtt_client og network-tråden
├── server/                   Python + FastAPI
│   ├── app/
│   │   ├── main.py           Starter serveren
│   │   ├── config.py         Innstillinger
│   │   ├── database.py       SQLite – målinger og vanningshistorikk
│   │   ├── mqtt_bridge.py    Tar imot data fra ESP32 og sender kommandoer
│   │   ├── routes.py         REST-API (/api/…)
│   │   └── models.py         Datamodeller for forespørsler
├── web/                      HTML, CSS, JavaScript (mobiltilpasset)
│   ├── index.html
│   ├── css/style.css
│   └── js/  api.js · chart.js · app.js
├── mosquitto/mosquitto.conf  MQTT-broker
├── cad/                      3D-modeller: vanntank, plantepotte, pumpeholder, elektronikkhus, base
└── docs/                     Koblingsskjema, MQTT-protokoll, utstyrsliste
```

## Kom i gang (Windows / PowerShell)

### 1. MQTT-broker (Mosquitto i Docker)

Start Docker Desktop, deretter:

```powershell
docker start mosquitto
```

(Containeren er satt opp med `listener 1883` og `allow_anonymous true`, samme som `mosquitto/mosquitto.conf`.)

### 2. Server + webapp

```powershell
cd server
python -m venv .venv
.venv\Scripts\Activate.ps1
pip install -r requirements.txt
uvicorn app.main:app --host 0.0.0.0 --port 8000
```

Åpne <http://localhost:8000>. Fra mobilen: `http://<PC-ens-IP>:8000`, og velg «Legg til på hjemskjerm».
(Windows-brannmuren må tillate port 8000 og 1883 på privat nettverk.)

### 3. Firmware (ESP32-S3)

Bruker Zephyr-oppsettet i `D:\zephyr-esp32s3` (Zephyr v4.4.0). Fordi Zephyr ligger på D: og
prosjektet på C:, peker `D:\zephyr-esp32s3\vanningsystem` (en junction) til `firmware\`-mappen.

Første gang: kopier `firmware\src\config\secrets.example.h` til `secrets.h` og fyll inn Wi-Fi + IP til PC-en.

```powershell
cd D:\zephyr-esp32s3
.venv\Scripts\Activate.ps1
west build -b adafruit_feather_esp32s3_tft/esp32s3/procpu -d build-vanning vanningsystem
west flash -d build-vanning
west espressif monitor -d build-vanning
```

Første gang: kalibrer jordfuktighetssensoren (se `docs/koblingsskjema.md`).

## Funksjoner

| Funksjon | Hvor |
|---|---|
| Automatisk vanning når jorden er tørr | `firmware/src/control/watering.c` |
| Manuell vanning fra mobil/web | `web/` → `/api/water` → MQTT `vanning/cmd` |
| Tidsplan (hver N. time) | `watering.c` + innstillinger i appen |
| Jordfuktighet, temperatur, luftfuktighet | `firmware/src/sensors/` |
| Varsel ved lavt vannivå / planten trenger vann | ESP32-hendelser + `routes.py` + nettleservarsler |
| Målinger på ESP32-skjermen | `firmware/src/display/` |
| Lagring av målinger og historikk | `server/app/database.py` |
| Sikkerhetsstopp (lavt vann, sensorfeil, maks pumpetid) | `watering.c` |
