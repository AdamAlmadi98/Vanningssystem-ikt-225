# 3D-modeller

Hver del har sin egen mappe:

- `kilde/` – kildefiler fra Fusion 360 / FreeCAD (`.f3d`, `.FCStd`, `.step`)
- `stl/` – eksporterte filer klare for slicer

| Del | Mappe | Krav |
|---|---|---|
| Vanntank | `vanntank/` | Lokk, hull for slanger, SEN0715 på **utsiden** (sjekk maks veggtykkelse i databladet). Tettes og lekkasjetestes. |
| Plantepotte | `plantepotte/` | Plass til plante, jord og jordfuktighetssensor. Dreneringshull + skål, innløp for slange. |
| Pumpeholder | `pumpeholder/` | Holder FIT0800 stabilt **utenfor** tanken. Plass til slangekoblinger. |
| Elektronikkhus | `elektronikkhus/` | Beskytter ESP32, TB6612FNG og koblinger. Vindu for skjerm, USB-C-tilgang, kabelgjennomføringer. Plasseres unna/høyere enn tanken. |
| Base 50 × 50 cm | `base/` | Større enn de fleste printerbord – del i 4 × (25 × 25 cm) med skjøter. Faste plasser for alle delene. |

Materiale: PETG. Navngi filer med versjon, f.eks. `vanntank_v2.stl`.
