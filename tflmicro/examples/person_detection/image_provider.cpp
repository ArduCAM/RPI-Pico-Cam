/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "image_provider.h"

#include "model_settings.h"
#include "arducam.h"
#include "pico/stdlib.h"

#include "tensorflow/lite/micro/micro_time.h"
#include <climits>

#define TF_LITE_MICRO_EXECUTION_TIME_BEGIN      \
  int32_t start_ticks;                          \
  int32_t duration_ticks;                       \
  int32_t duration_ms;

#define TF_LITE_MICRO_EXECUTION_TIME(reporter, func)                    \
  if (tflite::ticks_per_second() == 0) {                                \
    TF_LITE_REPORT_ERROR(reporter,                                      \
                         "no timer implementation found");              \
  }                                                                     \
  start_ticks = tflite::GetCurrentTimeTicks();                          \
  func;                                                                 \
  duration_ticks = tflite::GetCurrentTimeTicks() - start_ticks;         \
  if (duration_ticks > INT_MAX / 1000) {                                \
    duration_ms = duration_ticks / (tflite::ticks_per_second() / 1000); \
  } else {                                                              \
    duration_ms = (duration_ticks * 1000) / tflite::ticks_per_second(); \
  }                                                                     \
  TF_LITE_REPORT_ERROR(reporter, "%s took %d ticks (%d ms)", #func,     \
                                    duration_ticks, duration_ms);

TfLiteStatus GetImage(tflite::ErrorReporter* error_reporter, int image_width,
                      int image_height, int channels, int8_t* image_data) {
  uint8_t header[2] = {0x55, 0xAA};
  static bool first = true;
  if (first) {
    arducam.systemInit();
    if(arducam.busDetect()) {
      TF_LITE_REPORT_ERROR(error_reporter, "Bus detect failed.");
      return kTfLiteError;
    }
    if(arducam.cameraProbe()) {
      TF_LITE_REPORT_ERROR(error_reporter, "Camera probe failed.");
      return kTfLiteError;
    }
    arducam.cameraInit(YUV);
    first = false;
  }
  TF_LITE_MICRO_EXECUTION_TIME_BEGIN
  TF_LITE_MICRO_EXECUTION_TIME(error_reporter, capture((uint8_t *)image_data));
#ifndef DO_NOT_OUTPUT_TO_UART
  TF_LITE_MICRO_EXECUTION_TIME(error_reporter, uart_write_blocking(UART_ID, header, 2));
  TF_LITE_MICRO_EXECUTION_TIME(error_reporter, uart_write_blocking(UART_ID, (uint8_t *)image_data, kMaxImageSize));
#endif
  for (int i = 0; i < image_width * image_height * channels; ++i) {
    image_data[i] = (uint8_t)image_data[i] - 128;
  }
  return kTfLiteOk;
}
