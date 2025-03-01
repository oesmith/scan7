#include <stdint.h>
#include <string.h>

#include "mbe.h"
#include "poll.h"

#define READ_U16(p) ({\
    uint16_t retval = *p++; \
    retval |= *p++ << 8; \
    retval; })

#define READ_F16(p, min, max) \
    (min + ((float)READ_U16(p)) * (max - min) / 65535.f)

#define READ_U8(p) (*p++)

#define READ_F8(p, min, max) \
    (min + ((float)READ_U8(p)) * (max - min) / 255.f)

const uint8_t PAGE_1 = 0xf8;
const uint8_t PAGE_1_OFFSETS[] = {
  0x36, 0x37, // RT_AIRTEMP1(LIM)
  0x44, 0x45, // RT_COOLANTTEMP1(LIM)
  0x50, 0x51, // RT_THROTTLEANGLEINCREASING
  0x64, // RT_THROTTLESITE1
  0x6a, 0x6b, // RT_BAROSCALEDLIM
  0x7c, 0x7d, // RT_ENGINESPEED
  0x92, 0x93, // RT_OXYGENA(LIM)
  0x9e, 0x9f, // RT_BATTERYVOLTAGE(LIM)
  0xa3, // RT_TARGETLAMBDA
  0xa5, // RT_CURRENTLAMBDAA
};

const uint8_t PAGE_2 = 0xf9;
const uint8_t PAGE_2_OFFSETS[] = {
  0x98, 0x99, // RT_TARGET_IDLESPEED
  0xb2, 0xb3, // RT_LAMBDAINTTERM1
  0xba, 0xbb, // RT_SOFTCUTTIME
  0xbc, 0xbd, // RT_HARDCUTTIME
  0xd6, 0xd7, // RT_WARMUPTIMER
};

const uint8_t PAGE_3 = 0xfd;
const uint8_t PAGE_3_OFFSETS[] = {
  0x24, 0x25, // RT_CURRENTFAULTSA
  0x26, 0x27, // RT_CURRENTFAULTSB
  0x28, 0x29, // RT_CURRENTFAULTSC
  0x2a, 0x2b, // RT_CURRENTFAULTSD
  0x34, 0x35, // RT_LAMBDASTATUS(A)
  0x45, // RT_IDLESPEED_STATUS
  0x73, // RT_ENGINESYNCHSTATUS
};

mbe_error poll_ecu(status_t *status_out) {
  memset(status_out, 0, sizeof(status_t));
  uint8_t buf[256];
  uint8_t *ptr;
  mbe_error err;

  // IMPORTANT: Data is read out of the response messages in byte order. The
  // order of the reads below *must* match the order of the field offsets
  // defined in the PAGE_*_OFFSETS constants above.

  err = mbe_query(PAGE_1, PAGE_1_OFFSETS, buf, sizeof(PAGE_1_OFFSETS));
  if (err != MBE_OK) {
    return err;
  }
  ptr = buf;
  status_out->air_temp_c = READ_F16(ptr, -30.f, 130.f);
  status_out->coolant_temp_c = READ_F16(ptr, -30.f, 130.f);
  status_out->throttle_angle_volts = READ_F16(ptr, 0.f, 5.f);
  status_out->throttle_site = READ_F8(ptr, 0.f, 16.f);
  status_out->baro_pressure_bar = READ_F16(ptr, -1.f, 5.5535f);
  status_out->engine_rpm = READ_U16(ptr);
  status_out->lambda_volts = READ_F16(ptr, 0.f, 5.f);
  status_out->battery_volts = READ_F16(ptr, 0.f, 20.f);
  status_out->target_lambda = READ_F8(ptr, 0.f, 2.55f);
  status_out->current_lambda = READ_F8(ptr, 0.f, 2.55f);

  err = mbe_query(PAGE_2, PAGE_2_OFFSETS, buf, sizeof(PAGE_2_OFFSETS));
  if (err != MBE_OK) {
    return err;
  }
  ptr = buf;
  status_out->target_idle_rpm = READ_U16(ptr);
  status_out->lambda_trim_percent = READ_F16(ptr, -100., 100.);
  status_out->soft_cut_rpm = READ_U16(ptr);
  status_out->hard_cut_rpm = READ_U16(ptr);
  status_out->warm_up_timer_s = READ_F16(ptr, 0.f, 6881.f);

  err = mbe_query(PAGE_3, PAGE_3_OFFSETS, buf, sizeof(PAGE_3_OFFSETS));
  if (err != MBE_OK) {
    return err;
  }
  ptr = buf;
  status_out->current_faults_a_flags = READ_U16(ptr);
  status_out->current_faults_b_flags = READ_U16(ptr);
  status_out->current_faults_c_flags = READ_U16(ptr);
  status_out->current_faults_d_flags = READ_U16(ptr);
  status_out->lambda_status_flags = READ_U16(ptr);
  status_out->idlespeed_status_flags = READ_U8(ptr);
  status_out->engine_synch_status_flags = READ_U8(ptr);

  return MBE_OK;
};
