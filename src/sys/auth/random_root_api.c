// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md
#include <sdk/api.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "sos/api/crypt_api.h"
#include "sos/dev/random.h"

#include "../scheduler/scheduler_timing.h"

typedef struct {
  int dummy;
} random_root_context_t;

static int set_device_attributes(random_root_context_t *context, u32 o_flags, u32 seed);

static int random_root_init(void **context) {
  MCU_UNUSED_ARGUMENT(context);
  return 0;
}

static void random_root_deinit(void **context) { MCU_UNUSED_ARGUMENT(context); }

static u32 random_root_get_context_size() { return sizeof(random_root_context_t); }

static int random_root_seed(void *context, const unsigned char *data, u32 data_len) {
  MCU_UNUSED_ARGUMENT(context);
  MCU_UNUSED_ARGUMENT(context);
  MCU_UNUSED_ARGUMENT(context);
  return 0;
}

static int random_root_random(void *context, unsigned char *output, u32 output_length) {
  struct mcu_timeval tval;
  for (u32 length = 0; length < output_length; length++) {
    u8 result;
    for (u32 i = 0; i < 32; i++) {
      scheduler_timing_root_get_realtime(&tval);
      result = tval.tv_usec;
      cortexm_delay_us(result & 0x0fff);
    }
    scheduler_timing_root_get_realtime(&tval);
    output[length] = tval.tv_usec;
  }
  return output_length;
}

const crypt_random_api_t random_root_api = {
  .sos_api = {.name = "crypt_random_root", .version = 0x0001, .git_hash = SOS_GIT_HASH},
  .init = random_root_init,
  .deinit = random_root_deinit,
  .seed = random_root_seed,
  .random = random_root_random,
  .get_context_size = random_root_get_context_size};
