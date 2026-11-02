cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(vanningssystem)

target_include_directories(app PRIVATE src)

target_sources(app PRIVATE
    src/main.c

    src/core/app_state.c
    src/core/events.c

    src/sensors/sensors.c
    src/sensors/soil_sensor.c
    src/sensors/water_level.c
    src/sensors/aht20.c

    src/actuators/pump.c

    src/control/watering.c
    src/control/commands.c

    src/display/display.c

    src/network/network.c
    src/network/wifi.c
    src/network/mqtt_client.c
)
