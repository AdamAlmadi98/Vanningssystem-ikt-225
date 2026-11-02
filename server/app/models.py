"""Datamodeller for forespørsler fra appen."""
from pydantic import BaseModel, Field


class WaterRequest(BaseModel):
    seconds: int = Field(3, ge=1, le=15)


class ConfigRequest(BaseModel):
    auto: bool | None = None
    threshold: int | None = Field(None, ge=0, le=100)
    interval_h: int | None = Field(None, ge=0, le=72)
    seconds: int | None = Field(None, ge=1, le=15)
