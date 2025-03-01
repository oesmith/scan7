#ifndef _POLL_H_
#define _POLL_H_

#include <stdint.h>
#include "mbe.h"

typedef struct {
  // Page 0xf8
  float air_temp_c;
  float coolant_temp_c;
  float throttle_angle_volts;
  float throttle_site;
  float baro_pressure_bar;
  uint16_t engine_rpm;
  float lambda_volts;
  float battery_volts;
  float current_lambda;
  float target_lambda;

  // Page 0xf9
  uint16_t target_idle_rpm;
  float lambda_trim_percent;
  uint16_t soft_cut_rpm;
  uint16_t hard_cut_rpm;
  float warm_up_timer_s;

  // Page 0xfd
  uint16_t current_faults_a_flags;
  uint16_t current_faults_b_flags;
  uint16_t current_faults_c_flags;
  uint16_t current_faults_d_flags;
  uint16_t lambda_status_flags;
  uint8_t idlespeed_status_flags;
  uint8_t engine_synch_status_flags;
} status_t;

mbe_error poll_ecu(status_t* status_out);

#endif // _POLL_H_
