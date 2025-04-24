#ifndef COMMON_H
#define COMMON_H

#define DEVEUI_STR "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x"

#define DEVEUI(x) (x)[0], (x)[1], (x)[2], (x)[3], (x)[4], (x)[5], (x)[6], (x)[7]

#define APPKEY(x) (x)[0], (x)[1], (x)[2], (x)[3], (x)[4], (x)[5], (x)[6], (x)[7], \
                  (x)[8], (x)[9], (x)[10], (x)[11], (x)[12], (x)[13], (x)[14], (x)[15]

#include <stdint.h>
#include <Arduino.h>

#if DEBUG
#define DEBUG_SERIAL_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG_SERIAL_PRINTF(...)
#endif

#define RETURN_IF_FALSE(cond, ret, ...)                     \
    if ((cond) == false) {                                  \
        DEBUG_SERIAL_PRINTF(__VA_ARGS__);                   \
        return (ret);                                       \
    }                                                       \

typedef struct {
    uint8_t dr;
    uint8_t adr_enabled;
    uint16_t sub_band_mask;
    uint32_t rx1_delay;
    uint32_t rx2_delay;
    uint32_t retry;
} __attribute__((packed)) sLoRaParameters;

typedef struct {
    uint8_t dev_eui[8];
    uint8_t app_eui[8];
    uint8_t app_key[16];
} sLoRa;

typedef struct {
    uint8_t yOff;           // year offset from 2000
    uint8_t m;              // month (1-12)
    uint8_t d;              // day (1-31)
    uint8_t hh;             // hour (0-23)
    uint8_t mm;             // minute (0-59)
    uint8_t ss;             // second (0-59)
} __attribute__((packed)) EdLoDatetime;

typedef struct {
    EdLoDatetime LastDatetimeSaved;
    uint8_t period_to_send;
    uint8_t delta_max_period;
} __attribute__((packed)) EdLoTiming;
#endif
