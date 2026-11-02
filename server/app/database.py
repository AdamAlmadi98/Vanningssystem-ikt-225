"""Lagring av målinger og vanningshistorikk i SQLite."""
import json
import sqlite3
import time
from contextlib import contextmanager

from app.config import DB_PATH

SCHEMA = """
CREATE TABLE IF NOT EXISTS measurements (
    id        INTEGER PRIMARY KEY AUTOINCREMENT,
    ts        REAL NOT NULL,
    soil      INTEGER,
    temp      REAL,
    hum       REAL,
    water_ok  INTEGER,
    pump      INTEGER
);
CREATE INDEX IF NOT EXISTS idx_measurements_ts ON measurements(ts);

CREATE TABLE IF NOT EXISTS events (
    id      INTEGER PRIMARY KEY AUTOINCREMENT,
    ts      REAL NOT NULL,
    type    TEXT NOT NULL,
    reason  TEXT,
    data    TEXT
);
CREATE INDEX IF NOT EXISTS idx_events_ts ON events(ts);
"""


@contextmanager
def connect():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    try:
        yield conn
        conn.commit()
    finally:
        conn.close()


def init_db() -> None:
    DB_PATH.parent.mkdir(parents=True, exist_ok=True)
    with connect() as conn:
        conn.executescript(SCHEMA)


def add_measurement(m: dict) -> None:
    with connect() as conn:
        conn.execute(
            "INSERT INTO measurements (ts, soil, temp, hum, water_ok, pump) VALUES (?, ?, ?, ?, ?, ?)",
            (
                time.time(),
                m.get("soil") if m.get("soil_ok") else None,
                m.get("temp"),
                m.get("hum"),
                int(bool(m.get("water_ok"))),
                int(bool(m.get("pump"))),
            ),
        )


def add_event(event: dict) -> None:
    with connect() as conn:
        conn.execute(
            "INSERT INTO events (ts, type, reason, data) VALUES (?, ?, ?, ?)",
            (time.time(), event.get("type", "unknown"), event.get("reason"), json.dumps(event)),
        )


def get_history(hours: int, max_points: int = 300) -> list[dict]:
    """Målinger for de siste timene, gjennomsnittet i tidsbøtter så grafen ikke blir for tung."""
    since = time.time() - hours * 3600
    bucket = max(60, hours * 3600 // max_points)
    with connect() as conn:
        rows = conn.execute(
            """
            SELECT CAST(ts / :b AS INTEGER) * :b AS ts,
                   ROUND(AVG(soil), 1) AS soil,
                   ROUND(AVG(temp), 1) AS temp,
                   ROUND(AVG(hum), 1)  AS hum,
                   MIN(water_ok)       AS water_ok,
                   MAX(pump)           AS pump
            FROM measurements
            WHERE ts >= :since
            GROUP BY 1
            ORDER BY 1
            """,
            {"b": bucket, "since": since},
        ).fetchall()
    return [dict(r) for r in rows]


def get_events(limit: int) -> list[dict]:
    with connect() as conn:
        rows = conn.execute(
            "SELECT ts, type, reason, data FROM events ORDER BY ts DESC LIMIT ?", (limit,)
        ).fetchall()
    return [{**dict(r), "data": json.loads(r["data"]) if r["data"] else {}} for r in rows]
