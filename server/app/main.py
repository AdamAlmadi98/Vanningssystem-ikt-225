"""Vanningssystem – server. Start med:  uvicorn app.main:app --host 0.0.0.0 --port 8000"""
import logging
from contextlib import asynccontextmanager

from fastapi import FastAPI
from fastapi.staticfiles import StaticFiles

from app import config, database
from app.mqtt_bridge import bridge
from app.routes import router

logging.basicConfig(level=logging.INFO)


@asynccontextmanager
async def lifespan(app: FastAPI):
    database.init_db()
    bridge.start()
    yield
    bridge.stop()


app = FastAPI(title="Vanningssystem", lifespan=lifespan)
app.include_router(router)
app.mount("/", StaticFiles(directory=config.WEB_DIR, html=True), name="web")
