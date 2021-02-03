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

#include <limits>

#include "tensorflow/lite/c/common.h"
#include "model_settings.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/testing/micro_test.h"
#include "tensorflow/lite/micro/benchmarks/micro_benchmark.h"

namespace {
tflite::ErrorReporter* error_reporter = nullptr;
int8_t image_data[kMaxImageSize];
}

void Capture() {
  TfLiteStatus get_status = GetImage(error_reporter, kNumCols, kNumRows,
                                     kNumChannels, image_data);
}


void Initialize() {
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;
  
  Capture();
}

void CaptureIerations() {
  for (int i = 0; i < 100; i++) {
    Capture();
  }
}

TF_LITE_MICRO_BENCHMARKS_BEGIN

TF_LITE_MICRO_BENCHMARK(Initialize());
TF_LITE_MICRO_BENCHMARK(CaptureIerations());

TF_LITE_MICRO_BENCHMARKS_END
