/*
 * Copyright (c) 2014-2016 Cesanta Software Limited
 * All rights reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __TI_COMPILER_VERSION__
#include <unistd.h>
#endif

/* Driverlib includes */
#include "inc/hw_types.h"

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin.h"
#include "driverlib/prcm.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/utils.h"
#include "driverlib/wdt.h"

#include "common/platform.h"
#include "common/cs_dbg.h"

#include "simplelink.h"
#include "device.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include "mgos_hal.h"
#include "mgos_mongoose.h"
#include "mgos_updater_common.h"
#include "mgos_vfs_fs_spiffs.h"

#include "cc32xx_fs.h"
#include "cc32xx_main.h"
#include "cc32xx_vfs_dev_slfs_container.h"
#include "cc32xx_vfs_fs_slfs.h"

#include "cc3200_updater.h"
#include "fw/platforms/cc3200/boot/lib/boot.h"

/* These are FreeRTOS hooks for various life situations. */
void vApplicationMallocFailedHook(void) {
  fprintf(stderr, "malloc failed\n");
  exit(123);
}

void vApplicationIdleHook(void) {
  /* Ho-hum. Twiddling our thumbs. */
}

void vApplicationStackOverflowHook(OsiTaskHandle *th, signed char *tn) {
}

void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *e) {
}

/* Int vector table, defined in startup_gcc.c */
extern void (*const g_pfnVectors[])(void);

/* It may not be the best source of entropy, but it's better than nothing. */
static void cc3200_srand(void) {
  uint32_t r = 0, *p;
  for (p = (uint32_t *) 0x20000000; p < (uint32_t *) 0x20040000; p++) r ^= *p;
  srand(r);
}

enum mgos_init_result cc32xx_pre_nwp_init(void) {
  return MGOS_INIT_OK;
}

enum mgos_init_result cc32xx_init(void) {
  cc3200_srand();
#if MGOS_ENABLE_UPDATER
  if (!cc3200_upd_init()) return MGOS_INIT_UPD_INIT_FAILED;
#endif
  return MGOS_INIT_OK;
}

enum mgos_init_result mgos_fs_init(void) {
  if (!(cc32xx_vfs_dev_slfs_container_register_type() &&
        cc32xx_vfs_fs_slfs_register_type() &&
        mgos_vfs_fs_spiffs_register_type() &&
#if MGOS_ENABLE_UPDATER
        cc32xx_fs_spiffs_container_mount(
            "/", cc3200_upd_get_fs_container_prefix()) &&
#else
        cc32xx_fs_spiffs_container_mount("/", "spiffs.img.0") &&
#endif
        cc32xx_fs_slfs_mount("/slfs"))) {
    return MGOS_INIT_FS_INIT_FAILED;
  }
  return MGOS_INIT_OK;
}

/*
 * TODO(dfrank): figure what's going on here, and fix.
 *
 * Without it, when compiling with GCC without sntp lib, we're getting the
 * following linking error:
 *
 * LD    /app/fw/platforms/cc3200/.build/mongoose-os.elf
 * /usr/lib/gcc/arm-none-eabi/4.9.3/../../../arm-none-eabi/lib/armv7e-m/libc.a(lib_a-gettimeofdayr.o):
 * In function `_gettimeofday_r':
 * /build/newlib-5zwpxE/newlib-2.2.0+git20150830.5a3d536/build/arm-none-eabi/armv7e-m/newlib/libc/reent/../../../../../../newlib/libc/reent/gettimeofdayr.c:71:
 * undefined reference to `_gettimeofday'*
 *
 * The command to reproduce:
 *
 * $ make -C fw/platforms/cc3200 clean all TOOLCHAIN=gcc
 *
 * It's so weird, but it is what it is; need to fix sooner or later.
 */
void *cc3200_tmp_gettimeofday_workaround = settimeofday;

int main(void) {
  MAP_IntVTableBaseSet((unsigned long) &g_pfnVectors[0]);

  MAP_IntEnable(FAULT_SYSTICK);
  MAP_IntMasterEnable();

  cc32xx_main(); /* Does not return */

  return 0;
}
