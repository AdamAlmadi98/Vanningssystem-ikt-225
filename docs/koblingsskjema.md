# Koblingsskjema

Mikrokontroller: **Adafruit ESP32-S3 TFT Feather**

| Komponent | Komponentpinne | Feather-pinne | ESP32-S3 GPIO | Merknad |
|---|---|---|---|---|
| Jordfuktighet (Gravity IP65) | Signal (A) | A5 | GPIO8 | Må være ADC1 (ADC2 virker ikke med Wi-Fi) |
| | VCC / GND | 3V / GND | | |
| Vannivå SEN0715 | OUT | D5 | GPIO5 | Signalet må være maks 3,3 V |
| | VCC / GND | se datablad / GND | | |
| AHT20 | SDA / SCL | STEMMA QT | GPIO42 / GPIO41 | I2C-adresse 0x38 |
| TB6612FNG | AIN1 | D6 | GPIO6 | |
| | AIN2 | D9 | GPIO9 | |
| | PWMA | D10 | GPIO10 | Settes høy = full fart |
| | STBY | D11 | GPIO11 | Lav = motorstyringen av |
| | VCC (logikk) | 3V | | |
| | VM (motor) | +5 V fra FIT0976 | | |
| | AO1 / AO2 | Pumpe + / – | | Bytt ledningene hvis pumpen går feil vei |
| | GND | GND | | **Felles jord** med ESP32 og strømforsyning |

## Strøm

- 5 V / 5 A (FIT0976) → VM på TB6612FNG.
- ESP32 får strøm fra USB under utvikling.
- **Felles GND** mellom strømforsyning, TB6612FNG og ESP32 er påkrevd.
- Aldri kjør pumpen fra 3V-pinnen på ESP32.
- TB6612FNG tåler ca. 1,2 A kontinuerlig per kanal – sjekk at FIT0800 trekker mindre enn dette.
- Hold vann og elektronikk adskilt (pumpeholder utenfor tanken, elektronikkhus).

## Kalibrering av jordfuktighetssensoren

1. Flash firmware og åpne webappen (eller seriell monitor).
2. Hold sensoren i luft → les `soil_raw` → sett `SOIL_RAW_DRY` i `firmware/src/config/app_config.h`.
3. Sett sensoren i et glass vann (bare opp til streken!) → sett `SOIL_RAW_WET`.
4. Bygg og flash på nytt.

## Ting å sjekke på maskinvaren

- **SEN0715**: sjekk i databladet om utgangen er høy eller lav når den ser vann. Er den aktiv lav,
  endre `GPIO_ACTIVE_HIGH` til `GPIO_ACTIVE_LOW` for `water-level-gpios` i `firmware/app.overlay`.
- **STEMMA QT / skjerm**: på ESP32-S3 TFT styrer GPIO21 (`TFT_I2C_POWER`) strømmen til
  skjermen og I2C-kontakten. Hvis AHT20 eller skjermen ikke svarer, sjekk at denne er slått på
  i board-definisjonen til Zephyr-versjonen dere bruker.
