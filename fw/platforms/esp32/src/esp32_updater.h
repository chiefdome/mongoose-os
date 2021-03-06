/*
 * Copyright (c) 2014-2017 Cesanta Software Limited
 * All rights reserved
 */

#ifndef CS_FW_PLATFORMS_ESP32_SRC_ESP32_UPDATER_H_
#define CS_FW_PLATFORMS_ESP32_SRC_ESP32_UPDATER_H_

#include <stdbool.h>

#if MGOS_ENABLE_UPDATER

#ifdef __cplusplus
extern "C" {
#endif

void esp32_updater_early_init();

#ifdef __cplusplus
}
#endif

#endif /* MGOS_ENABLE_UPDATER */

#endif /* CS_FW_PLATFORMS_ESP32_SRC_ESP32_UPDATER_H_ */
