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

#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/kernels/kernel_runner.h"
#include "tensorflow/lite/micro/micro_utils.h"
#include "tensorflow/lite/micro/test_helpers.h"
#include "tensorflow/lite/micro/testing/micro_test.h"

namespace tflite {
namespace testing {
namespace {

// Common inputs and outputs.
constexpr int kInputElements = 16;
static const int kInputShape[] = {4, 2, 2, 4, 1};
static const float kInputData[kInputElements] = {1, 1, 1, 1, 2, 2, 2, 2,
                                                 1, 2, 3, 4, 1, 2, 3, 4};

constexpr int kFilterElements = 12;
static const int kFilterShape[] = {4, 3, 2, 2, 1};
static const float kFilterData[kFilterElements] = {1,  2, 3,  4,  -1, 1,
                                                   -1, 1, -1, -1, 1,  1};

constexpr int kBiasElements = 3;
static const int kBiasShape[] = {1, 3};
static const float kBiasData[kBiasElements] = {1, 2, 3};

constexpr int kOutputElements = 12;
static const int kOutputShape[] = {4, 2, 1, 2, 3};
static const float kGoldenData[kOutputElements] = {18, 2, 5, 18, 2, 5,
                                                   17, 4, 3, 37, 4, 3};

static TfLiteConvParams common_conv_params = {
    kTfLitePaddingValid,  // padding
    2,                    // stride_width
    2,                    // stride_height
    kTfLiteActNone,       // activation
    1,                    // dilation_width_factor
    1,                    // dilation_height_factor
};

template <typename T>
TfLiteStatus InvokeConv(TfLiteTensor* tensors, int tensors_size, T* output_data,
                        int output_length, TfLiteConvParams* conv_params) {
  int inputs_array_data[] = {3, 0, 1, 2};
  TfLiteIntArray* inputs_array = IntArrayFromInts(inputs_array_data);
  int outputs_array_data[] = {1, 3};
  TfLiteIntArray* outputs_array = IntArrayFromInts(outputs_array_data);

  const TfLiteRegistration registration = Register_CONV_2D();
  micro::KernelRunner runner(
      registration, tensors, tensors_size, inputs_array, outputs_array,
      reinterpret_cast<void*>(conv_params), micro_test::reporter);

  const char* init_data = reinterpret_cast<const char*>(conv_params);
  TfLiteStatus status = runner.InitAndPrepare(init_data);
  if (status != kTfLiteOk) {
    return status;
  }
  return runner.Invoke();
}

template <typename T>
TfLiteStatus ValidateConvGoldens(TfLiteTensor* tensors, int tensors_size,
                                 const T* expected_output_data, T* output_data,
                                 int output_length,
                                 TfLiteConvParams* conv_params,
                                 float tolerance = 1e-5) {
  TfLiteStatus status = InvokeConv(tensors, tensors_size, output_data,
                                   output_length, conv_params);
  if (status != kTfLiteOk) {
    return status;
  }
  for (int i = 0; i < output_length; ++i) {
    TF_LITE_MICRO_EXPECT_NEAR(expected_output_data[i], output_data[i],
                              tolerance);
  }
  return kTfLiteOk;
}

#if !defined(XTENSA)  // Needed to avoid build errors from unused functions.
void TestConvFloat(const int* input_dims_data, const float* input_data,
                   const int* filter_dims_data, const float* filter_data,
                   const int* bias_dims_data, const float* bias_data,
                   const int* output_dims_data,
                   const float* expected_output_data, float* output_data,
                   TfLiteConvParams* conv_params) {
  TfLiteIntArray* input_dims = IntArrayFromInts(input_dims_data);
  TfLiteIntArray* filter_dims = IntArrayFromInts(filter_dims_data);
  TfLiteIntArray* bias_dims = IntArrayFromInts(bias_dims_data);
  TfLiteIntArray* output_dims = IntArrayFromInts(output_dims_data);
  const int output_dims_count = ElementCount(*output_dims);
  constexpr int inputs_size = 3;
  constexpr int outputs_size = 1;
  constexpr int tensors_size = inputs_size + outputs_size;
  TfLiteTensor tensors[tensors_size] = {
      CreateTensor(input_data, input_dims),
      CreateTensor(filter_data, filter_dims),
      CreateTensor(bias_data, bias_dims),
      CreateTensor(output_data, output_dims),
  };

  TF_LITE_MICRO_EXPECT_EQ(
      kTfLiteOk,
      ValidateConvGoldens(tensors, tensors_size, expected_output_data,
                          output_data, output_dims_count, conv_params));
}

void TestConvQuantizedPerLayer(
    const int* input_dims_data, const float* input_data,
    uint8_t* input_quantized, float input_scale, const int* filter_dims_data,
    const float* filter_data, uint8_t* filter_quantized, float filter_scale,
    const int* bias_dims_data, const float* bias_data, int32_t* bias_quantized,
    const int* output_dims_data, const float* expected_output_data,
    uint8_t* expected_output_quantized, uint8_t* output_data,
    float output_scale, TfLiteConvParams* conv_params) {
  TfLiteIntArray* input_dims = IntArrayFromInts(input_dims_data);
  TfLiteIntArray* filter_dims = IntArrayFromInts(filter_dims_data);
  TfLiteIntArray* bias_dims = IntArrayFromInts(bias_dims_data);
  TfLiteIntArray* output_dims = IntArrayFromInts(output_dims_data);
  const int output_dims_count = ElementCount(*output_dims);

  tflite::Quantize(expected_output_data, expected_output_quantized,
                   output_dims_count, output_scale, 128);

  constexpr int inputs_size = 3;
  constexpr int outputs_size = 1;
  constexpr int tensors_size = inputs_size + outputs_size;
  TfLiteTensor tensors[tensors_size] = {
      CreateQuantizedTensor(input_data, input_quantized, input_dims,
                            input_scale, 128),
      CreateQuantizedTensor(filter_data, filter_quantized, filter_dims,
                            filter_scale, 128),
      CreateQuantizedBiasTensor(bias_data, bias_quantized, bias_dims,
                                input_scale, filter_scale),
      CreateQuantizedTensor(output_data, output_dims, output_scale, 128)};

  // TODO(njeff): Affine Quantization Params should be set on tensor creation.
  float filter_scales[] = {1, filter_scale};
  int filter_zero_points[] = {1, 128};
  TfLiteAffineQuantization filter_quant = {FloatArrayFromFloats(filter_scales),
                                           IntArrayFromInts(filter_zero_points),
                                           0};
  tensors[1].quantization = {kTfLiteAffineQuantization, &filter_quant};

  TF_LITE_MICRO_EXPECT_EQ(
      kTfLiteOk,
      ValidateConvGoldens(tensors, tensors_size, expected_output_quantized,
                          output_data, output_dims_count, conv_params));
}

void TestConvQuantizedPerChannel(
    const int* input_dims_data, const float* input_data,
    int8_t* input_quantized, float input_scale, int input_zero_point,
    const int* filter_dims_data, const float* filter_data,
    int8_t* filter_data_quantized, const int* bias_dims_data,
    const float* bias_data, int32_t* bias_data_quantized, float* bias_scales,
    int* bias_zero_points, const int* output_dims_data,
    const float* expected_output_data, int8_t* expected_output_data_quantized,
    int8_t* output_data, float output_scale, int output_zero_point,
    TfLiteConvParams* conv_params) {
  TfLiteIntArray* input_dims = IntArrayFromInts(input_dims_data);
  TfLiteIntArray* filter_dims = IntArrayFromInts(filter_dims_data);
  TfLiteIntArray* bias_dims = IntArrayFromInts(bias_dims_data);
  TfLiteIntArray* output_dims = IntArrayFromInts(output_dims_data);
  const int output_dims_count = ElementCount(*output_dims);

  int filter_zero_points[5];
  float filter_scales[5];
  TfLiteAffineQuantization filter_quant;
  TfLiteAffineQuantization bias_quant;
  TfLiteTensor input_tensor = CreateQuantizedTensor(
      input_data, input_quantized, input_dims, input_scale, input_zero_point);
  TfLiteTensor filter_tensor = CreateSymmetricPerChannelQuantizedTensor(
      filter_data, filter_data_quantized, filter_dims, filter_scales,
      filter_zero_points, &filter_quant, 0 /* quantized dimension */);
  TfLiteTensor bias_tensor = CreatePerChannelQuantizedBiasTensor(
      bias_data, bias_data_quantized, bias_dims, input_scale, &filter_scales[1],
      bias_scales, bias_zero_points, &bias_quant, 0 /* quantized dimension */);
  TfLiteTensor output_tensor = CreateQuantizedTensor(
      output_data, output_dims, output_scale, output_zero_point);

  // TODO(njeff): Affine Quantization Params should be set on tensor creation.
  float input_scales[] = {1, input_scale};
  int input_zero_points[] = {1, input_zero_point};
  TfLiteAffineQuantization input_quant = {FloatArrayFromFloats(input_scales),
                                          IntArrayFromInts(input_zero_points),
                                          0};
  input_tensor.quantization = {kTfLiteAffineQuantization, &input_quant};

  float output_scales[] = {1, output_scale};
  int output_zero_points[] = {1, output_zero_point};
  TfLiteAffineQuantization output_quant = {FloatArrayFromFloats(output_scales),
                                           IntArrayFromInts(output_zero_points),
                                           0};
  output_tensor.quantization = {kTfLiteAffineQuantization, &output_quant};

  constexpr int inputs_size = 3;
  constexpr int outputs_size = 1;
  constexpr int tensors_size = inputs_size + outputs_size;
  TfLiteTensor tensors[tensors_size] = {
      input_tensor,
      filter_tensor,
      bias_tensor,
      output_tensor,
  };

  tflite::Quantize(expected_output_data, expected_output_data_quantized,
                   output_dims_count, output_scale, output_zero_point);
  TF_LITE_MICRO_EXPECT_EQ(
      kTfLiteOk,
      ValidateConvGoldens(tensors, tensors_size, expected_output_data_quantized,
                          output_data, output_dims_count, conv_params,
                          1.0 /* tolerance */));
}
#endif  // !defined(XTENSA)

}  // namespace
}  // namespace testing
}  // namespace tflite

TF_LITE_MICRO_TESTS_BEGIN

#if !defined(XTENSA)  // TODO(b/170321206): xtensa kernels are less general than
                      // reference kernels and we ifdef out test cases that are
                      // currently known to fail.
TF_LITE_MICRO_TEST(SimpleTestFloat) {
  float output_data[tflite::testing::kOutputElements];

  tflite::testing::TestConvFloat(
      tflite::testing::kInputShape, tflite::testing::kInputData,
      tflite::testing::kFilterShape, tflite::testing::kFilterData,
      tflite::testing::kBiasShape, tflite::testing::kBiasData,
      tflite::testing::kOutputShape, tflite::testing::kGoldenData, output_data,
      &tflite::testing::common_conv_params);
}

TF_LITE_MICRO_TEST(InputAndFilterSameWidthHeight) {
  const int output_dims_count = 2;
  float output_data[output_dims_count];

  const int kFilterShape[] = {4, 1, 2, 4, 1};
  const float filter_values[] = {1, 2, 3, 4, -1, -1, 1, 1};
  const int kBiasShape[] = {1, 1};
  const float bias_values[] = {0};
  const int kOutputShape[] = {4, 2, 1, 1, 1};
  const float expected_output[] = {10, 34};

  tflite::testing::TestConvFloat(
      tflite::testing::kInputShape, tflite::testing::kInputData, kFilterShape,
      filter_values, kBiasShape, bias_values, kOutputShape, expected_output,
      output_data, &tflite::testing::common_conv_params);
}

TF_LITE_MICRO_TEST(SimpleTestQuantized) {
  const int output_dims_count = 12;
  uint8_t output_data[output_dims_count];

  const float input_scale = 0.5f;
  const float filter_scale = 0.5f;
  const float output_scale = 1.0f;

  uint8_t input_quantized[tflite::testing::kInputElements];
  uint8_t filter_quantized[tflite::testing::kFilterElements];
  int32_t bias_quantized[tflite::testing::kBiasElements];
  uint8_t golden_quantized[tflite::testing::kOutputElements];

  tflite::testing::TestConvQuantizedPerLayer(
      tflite::testing::kInputShape, tflite::testing::kInputData,
      input_quantized, input_scale, tflite::testing::kFilterShape,
      tflite::testing::kFilterData, filter_quantized, filter_scale,
      tflite::testing::kBiasShape, tflite::testing::kBiasData, bias_quantized,
      tflite::testing::kOutputShape, tflite::testing::kGoldenData,
      golden_quantized, output_data, output_scale,
      &tflite::testing::common_conv_params);
}

TF_LITE_MICRO_TEST(InputOutputDifferentTypeIsError) {
  using tflite::testing::CreateQuantizedTensor;
  using tflite::testing::CreateTensor;
  using tflite::testing::IntArrayFromInts;

  TfLiteIntArray* input_dims = IntArrayFromInts(tflite::testing::kInputShape);
  TfLiteIntArray* filter_dims = IntArrayFromInts(tflite::testing::kFilterShape);
  TfLiteIntArray* bias_dims = IntArrayFromInts(tflite::testing::kBiasShape);
  TfLiteIntArray* output_dims = IntArrayFromInts(tflite::testing::kOutputShape);
  const int output_dims_count = tflite::ElementCount(*output_dims);
  constexpr int inputs_size = 3;
  constexpr int outputs_size = 1;
  constexpr int tensors_size = inputs_size + outputs_size;

  int8_t output_data[tflite::testing::kOutputElements];
  TfLiteTensor tensors[tensors_size] = {
      CreateTensor(tflite::testing::kInputData, input_dims),
      CreateTensor(tflite::testing::kFilterData, filter_dims),
      CreateTensor(tflite::testing::kBiasData, bias_dims),
      CreateQuantizedTensor(output_data, output_dims, /*scale=*/0.0f,
                            /*zero_point=*/0),
  };
  TF_LITE_MICRO_EXPECT_EQ(
      kTfLiteError, tflite::testing::InvokeConv(
                        tensors, tensors_size, output_data, output_dims_count,
                        &tflite::testing::common_conv_params));
}

TF_LITE_MICRO_TEST(HybridModeIsError) {
  using tflite::testing::CreateQuantizedTensor;
  using tflite::testing::CreateTensor;
  using tflite::testing::IntArrayFromInts;

  TfLiteIntArray* input_dims = IntArrayFromInts(tflite::testing::kInputShape);
  TfLiteIntArray* filter_dims = IntArrayFromInts(tflite::testing::kFilterShape);
  TfLiteIntArray* bias_dims = IntArrayFromInts(tflite::testing::kBiasShape);
  TfLiteIntArray* output_dims = IntArrayFromInts(tflite::testing::kOutputShape);
  const int output_dims_count = tflite::ElementCount(*output_dims);
  constexpr int inputs_size = 3;
  constexpr int outputs_size = 1;
  constexpr int tensors_size = inputs_size + outputs_size;

  int8_t filter_data[tflite::testing::kFilterElements] = {};
  float output_data[tflite::testing::kOutputElements];
  TfLiteTensor tensors[tensors_size] = {
      CreateTensor(tflite::testing::kInputData, input_dims),
      CreateQuantizedTensor(filter_data, filter_dims,
                            /*scale=*/0.0f,
                            /*zero_point=*/0),
      CreateTensor(tflite::testing::kBiasData, bias_dims),
      CreateTensor(output_data, output_dims),
  };
  TF_LITE_MICRO_EXPECT_EQ(
      kTfLiteError, tflite::testing::InvokeConv(
                        tensors, tensors_size, output_data, output_dims_count,
                        &tflite::testing::common_conv_params));
}

TF_LITE_MICRO_TEST(SimpleTestDilatedQuantized) {
  const int output_dims_count = 24;
  uint8_t output_data[output_dims_count];

  const float input_scale = 0.5f;
  const float filter_scale = 0.5f;
  const float output_scale = 1.0f;

  const int input_elements = 48;
  const int input_shape[] = {4, 2, 4, 6, 1};
  const float input_data[] = {
      // b = 0
      1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4,
      // b = 1
      1, 2, 3, 4, 5, 6, 2, 6, 2, 4, 4, 2, 3, 2, 6, 5, 1, 4, 1, 2, 1, 4, 6, 3};
  const int output_elements = 24;
  const int output_shape[] = {4, 2, 2, 2, 3};
  const float golden_data[] = {25, 2, 7, 25, 2, 7, 10, 2, -3, 10, 2, -3,
                               39, 7, 6, 50, 3, 4, 14, 4, -5, 15, 0, -7};

  uint8_t input_quantized[input_elements];
  uint8_t filter_quantized[tflite::testing::kFilterElements];
  int32_t bias_quantized[tflite::testing::kBiasElements];
  uint8_t golden_quantized[output_elements];

  TfLiteConvParams conv_params{tflite::testing::common_conv_params};
  conv_params.dilation_width_factor = 3;
  conv_params.dilation_height_factor = 2;

  tflite::testing::TestConvQuantizedPerLayer(
      input_shape, input_data, input_quantized, input_scale,
      tflite::testing::kFilterShape, tflite::testing::kFilterData,
      filter_quantized, filter_scale, tflite::testing::kBiasShape,
      tflite::testing::kBiasData, bias_quantized, output_shape, golden_data,
      golden_quantized, output_data, output_scale, &conv_params);
}

TF_LITE_MICRO_TEST(SimpleTestQuantizedPerChannel) {
  const int output_dims_count = 12;
  int8_t output_data[output_dims_count];

  const float input_scale = 0.5f;
  const float output_scale = 1.0f;
  const int input_zero_point = 0;
  const int output_zero_point = 0;

  int8_t input_quantized[tflite::testing::kInputElements];
  int8_t filter_quantized[tflite::testing::kFilterElements];
  int32_t bias_quantized[tflite::testing::kBiasElements];
  int8_t golden_quantized[tflite::testing::kOutputElements];
  int zero_points[tflite::testing::kBiasElements + 1];
  float scales[tflite::testing::kBiasElements + 1];

  tflite::testing::TestConvQuantizedPerChannel(
      tflite::testing::kInputShape, tflite::testing::kInputData,
      input_quantized, input_scale, input_zero_point,
      tflite::testing::kFilterShape, tflite::testing::kFilterData,
      filter_quantized, tflite::testing::kBiasShape, tflite::testing::kBiasData,
      bias_quantized, scales, zero_points, tflite::testing::kOutputShape,
      tflite::testing::kGoldenData, golden_quantized, output_data, output_scale,
      output_zero_point, &tflite::testing::common_conv_params);
}

TF_LITE_MICRO_TEST(SimpleTestDilatedQuantizedPerChannel) {
  const int output_dims_count = 24;
  int8_t output_data[output_dims_count];

  const float input_scale = 0.5f;
  const float output_scale = 1.0f;
  const int input_zero_point = 0;
  const int output_zero_point = 0;

  const int input_elements = 48;
  const int input_shape[] = {4, 2, 4, 6, 1};
  const float input_data[] = {
      // b = 0
      1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4,
      // b = 1
      1, 2, 3, 4, 5, 6, 2, 6, 2, 4, 4, 2, 3, 2, 6, 5, 1, 4, 1, 2, 1, 4, 6, 3};
  const int output_elements = 24;
  const int output_shape[] = {4, 2, 2, 2, 3};
  const float golden_data[] = {25, 2, 7, 25, 2, 7, 10, 2, -3, 10, 2, -3,
                               39, 7, 6, 50, 3, 4, 14, 4, -5, 15, 0, -7};

  int8_t input_quantized[input_elements];
  int8_t filter_quantized[tflite::testing::kFilterElements];
  int32_t bias_quantized[tflite::testing::kBiasElements];
  int8_t golden_quantized[output_elements];
  int zero_points[tflite::testing::kBiasElements + 1];
  float scales[tflite::testing::kBiasElements + 1];

  TfLiteConvParams conv_params{tflite::testing::common_conv_params};
  conv_params.dilation_width_factor = 3;
  conv_params.dilation_height_factor = 2;

  tflite::testing::TestConvQuantizedPerChannel(
      input_shape, input_data, input_quantized, input_scale, input_zero_point,
      tflite::testing::kFilterShape, tflite::testing::kFilterData,
      filter_quantized, tflite::testing::kBiasShape, tflite::testing::kBiasData,
      bias_quantized, scales, zero_points, output_shape, golden_data,
      golden_quantized, output_data, output_scale, output_zero_point,
      &conv_params);
}

TF_LITE_MICRO_TEST(SimpleTestQuantizedPerChannelRelu6) {
  const int output_dims_count = 12;
  int8_t output_data[output_dims_count];

  const float bias_values[] = {1, 2, -3};
  const float golden_data[] = {6, 2, 0, 6, 2, 0, 6, 4, 0, 6, 4, 0};

  const float input_scale = 0.023529f;
  const float output_scale = 0.023529f;
  const int input_zero_point = -128;
  const int output_zero_point = -128;

  int8_t input_quantized[tflite::testing::kInputElements];
  int8_t filter_quantized[tflite::testing::kFilterElements];
  int32_t bias_quantized[tflite::testing::kBiasElements];
  int8_t golden_quantized[tflite::testing::kOutputElements];
  int zero_points[tflite::testing::kBiasElements + 1];
  float scales[tflite::testing::kBiasElements + 1];

  tflite::testing::TestConvQuantizedPerChannel(
      tflite::testing::kInputShape, tflite::testing::kInputData,
      input_quantized, input_scale, input_zero_point,
      tflite::testing::kFilterShape, tflite::testing::kFilterData,
      filter_quantized, tflite::testing::kBiasShape, bias_values,
      bias_quantized, scales, zero_points, tflite::testing::kOutputShape,
      golden_data, golden_quantized, output_data, output_scale,
      output_zero_point, &tflite::testing::common_conv_params);
}

TF_LITE_MICRO_TEST(Kernel1x1QuantizedPerChannel) {
  // conv params:
  // padding, stride_<width,height>, activation, dilation_<width, height>
  TfLiteConvParams conv_params = {kTfLitePaddingValid, 1, 1,
                                  kTfLiteActNone,      1, 1};

  constexpr int input_shape[] = {4, 1, 2, 2, 4};  // [len,N,H,W,C]
  constexpr int input_elements =
      input_shape[1] * input_shape[2] * input_shape[3] * input_shape[4];
  constexpr float input_data[input_elements] = {1, 1, 1, 1, 2, 2, 2, 2,
                                                1, 2, 3, 4, 1, 2, 3, 4};

  constexpr int filter_shape[] = {4, 3, 1, 1, 4};
  constexpr int filter_elements =
      filter_shape[1] * filter_shape[2] * filter_shape[3] * filter_shape[4];
  const float filter_data[filter_elements] = {1,  2, 3,  4,  -1, 1,
                                              -1, 1, -1, -1, 1,  1};

  constexpr int bias_elements = filter_shape[1];
  constexpr int bias_shape[] = {1, bias_elements};
  constexpr float bias_data[bias_elements] = {1, 2, 3};

  constexpr int output_shape[] = {4, 1, 2, 2, bias_elements};
  constexpr int output_elements = 4 * 3;
  int8_t output_data[output_elements];

  const float golden_data[output_elements] = {11, 2, 3, 21, 2, 3,
                                              31, 4, 7, 31, 4, 7};

  const float input_scale = 0.5f;
  const float output_scale = 1.0f;
  const int input_zero_point = 0;
  const int output_zero_point = 0;

  int8_t input_quantized[input_elements];
  int8_t filter_quantized[filter_elements];
  int32_t bias_quantized[bias_elements];
  int8_t golden_quantized[output_elements];
  int zero_points[bias_elements + 1];
  float scales[bias_elements + 1];

  tflite::testing::TestConvQuantizedPerChannel(
      input_shape, input_data, input_quantized, input_scale, input_zero_point,
      filter_shape, filter_data, filter_quantized, bias_shape, bias_data,
      bias_quantized, scales, zero_points, output_shape, golden_data,
      golden_quantized, output_data, output_scale, output_zero_point,
      &conv_params);
}

TF_LITE_MICRO_TEST(Kernel1x1QuantizedPerChannelRelu6) {
  // conv params:
  // padding, stride_<width,height>, activation, dilation_<width, height>
  TfLiteConvParams conv_params = {kTfLitePaddingValid, 1, 1,
                                  kTfLiteActRelu6,     1, 1};

  constexpr int input_shape[] = {4, 1, 2, 2, 4};  // [len,N,H,W,C]
  constexpr int input_elements =
      input_shape[1] * input_shape[2] * input_shape[3] * input_shape[4];
  constexpr float input_data[input_elements] = {1, 1, 1, 1, 2, 2, 2, 2,
                                                1, 2, 3, 4, 1, 2, 3, 4};

  constexpr int filter_shape[] = {4, 3, 1, 1, 4};
  constexpr int filter_elements =
      filter_shape[1] * filter_shape[2] * filter_shape[3] * filter_shape[4];
  const float filter_data[filter_elements] = {1,  2, 3,  4,  -1, 1,
                                              -1, 1, -1, -1, 1,  1};

  constexpr int bias_elements = filter_shape[1];
  constexpr int bias_shape[] = {1, bias_elements};
  constexpr float bias_data[bias_elements] = {1, 2, -3};

  constexpr int output_shape[] = {4, 1, 2, 2, bias_elements};
  constexpr int output_elements = 4 * 3;
  int8_t output_data[output_elements];

  const float golden_data[output_elements] = {6, 2, 0, 6, 2, 0,
                                              6, 4, 1, 6, 4, 1};

  const float input_scale = 0.023529f;
  const float output_scale = 0.023529f;
  const int input_zero_point = -128;
  const int output_zero_point = -128;

  int8_t input_quantized[input_elements];
  int8_t filter_quantized[filter_elements];
  int32_t bias_quantized[bias_elements];
  int8_t golden_quantized[output_elements];
  int zero_points[bias_elements + 1];
  float scales[bias_elements + 1];

  tflite::testing::TestConvQuantizedPerChannel(
      input_shape, input_data, input_quantized, input_scale, input_zero_point,
      filter_shape, filter_data, filter_quantized, bias_shape, bias_data,
      bias_quantized, scales, zero_points, output_shape, golden_data,
      golden_quantized, output_data, output_scale, output_zero_point,
      &conv_params);
}

TF_LITE_MICRO_TEST(BroadcastPerLayerQuantizationToPerChannelShouldMatchGolden) {
  const int output_dims_count = 12;
  int8_t output_data[output_dims_count];

  const float input_scale = 1.0f;
  const float filter_scale = 1.0f;
  const float output_scale = 1.0f;

  int8_t input_quantized[tflite::testing::kInputElements];
  int8_t filter_quantized[tflite::testing::kFilterElements];
  int32_t bias_quantized[tflite::testing::kBiasElements];
  int8_t golden_quantized[tflite::testing::kOutputElements];

  TfLiteIntArray* input_dims =
      tflite::testing::IntArrayFromInts(tflite::testing::kInputShape);
  TfLiteIntArray* filter_dims =
      tflite::testing::IntArrayFromInts(tflite::testing::kFilterShape);
  TfLiteIntArray* bias_dims =
      tflite::testing::IntArrayFromInts(tflite::testing::kBiasShape);
  TfLiteIntArray* output_dims =
      tflite::testing::IntArrayFromInts(tflite::testing::kOutputShape);

  // Create per-layer quantized int8_t input tensor.
  TfLiteTensor input_tensor = tflite::testing::CreateQuantizedTensor(
      tflite::testing::kInputData, input_quantized, input_dims, input_scale, 0);
  int input_zero_points[2] = {1, 0};
  float input_scales[2] = {1, input_scale};
  TfLiteAffineQuantization input_quant = {
      tflite::testing::FloatArrayFromFloats(input_scales),
      tflite::testing::IntArrayFromInts(input_zero_points), 0};
  input_tensor.quantization = {kTfLiteAffineQuantization, &input_quant};

  // Create per-layer quantized int8_t filter tensor.
  TfLiteTensor filter_tensor = tflite::testing::CreateQuantizedTensor(
      tflite::testing::kFilterData, filter_quantized, filter_dims, filter_scale,
      0);
  int filter_zero_points[2] = {1, 0};
  float filter_scales[2] = {1, filter_scale};
  TfLiteAffineQuantization filter_quant = {
      tflite::testing::FloatArrayFromFloats(filter_scales),
      tflite::testing::IntArrayFromInts(filter_zero_points), 0};
  filter_tensor.quantization = {kTfLiteAffineQuantization, &filter_quant};

  // Create per-layer quantized int32_t bias tensor.
  tflite::SymmetricQuantize(tflite::testing::kBiasData, bias_quantized,
                            tflite::testing::kBiasElements,
                            input_scale * output_scale);
  TfLiteTensor bias_tensor =
      tflite::testing::CreateTensor(bias_quantized, bias_dims);

  int bias_zero_points[2] = {1, 0};
  float bias_scales[2] = {1, input_scale * filter_scale};
  TfLiteAffineQuantization bias_quant = {
      tflite::testing::FloatArrayFromFloats(bias_scales),
      tflite::testing::IntArrayFromInts(bias_zero_points), 0};
  bias_tensor.quantization = {kTfLiteAffineQuantization, &bias_quant};

  // Create per-layer quantized int8_t output tensor.
  TfLiteTensor output_tensor = tflite::testing::CreateQuantizedTensor(
      output_data, output_dims, output_scale, 0 /* quantized dimension */);
  int output_zero_points[2] = {1, 0};
  float output_scales[2] = {1, output_scale};
  TfLiteAffineQuantization output_quant = {
      tflite::testing::FloatArrayFromFloats(output_scales),
      tflite::testing::IntArrayFromInts(output_zero_points), 0};
  output_tensor.quantization = {kTfLiteAffineQuantization, &output_quant};

  constexpr int inputs_size = 3;
  constexpr int outputs_size = 1;
  constexpr int tensors_size = inputs_size + outputs_size;
  TfLiteTensor tensors[tensors_size] = {
      input_tensor,
      filter_tensor,
      bias_tensor,
      output_tensor,
  };

  tflite::Quantize(tflite::testing::kGoldenData, golden_quantized,
                   output_dims_count, output_scale, 0);

  TF_LITE_MICRO_EXPECT_EQ(
      kTfLiteOk, tflite::testing::ValidateConvGoldens(
                     tensors, tensors_size, golden_quantized, output_data,
                     output_dims_count, &tflite::testing::common_conv_params));
}

#endif  // !defined(XTENSA)

TF_LITE_MICRO_TEST(FilterDimsNotMatchingAffineQuantization) {
  const int output_dims_count = 12;
  int8_t output_data[output_dims_count];

  const float input_scale = 0.5f;
  const float output_scale = 1.0f;

  int8_t input_quantized[tflite::testing::kInputElements];
  int8_t filter_quantized[tflite::testing::kFilterElements];
  int32_t bias_quantized[tflite::testing::kBiasElements];
  int8_t golden_quantized[tflite::testing::kOutputElements];
  int zero_points[tflite::testing::kBiasElements + 1];
  float scales[tflite::testing::kBiasElements + 1];

  TfLiteIntArray* input_dims =
      tflite::testing::IntArrayFromInts(tflite::testing::kInputShape);
  TfLiteIntArray* filter_dims =
      tflite::testing::IntArrayFromInts(tflite::testing::kFilterShape);
  TfLiteIntArray* bias_dims =
      tflite::testing::IntArrayFromInts(tflite::testing::kBiasShape);
  TfLiteIntArray* output_dims =
      tflite::testing::IntArrayFromInts(tflite::testing::kOutputShape);

  int filter_zero_points[5];
  float filter_scales[5];
  TfLiteAffineQuantization filter_quant;
  TfLiteAffineQuantization bias_quant;
  TfLiteTensor input_tensor = tflite::testing::CreateQuantizedTensor(
      tflite::testing::kInputData, input_quantized, input_dims, input_scale, 0);
  TfLiteTensor filter_tensor =
      tflite::testing::CreateSymmetricPerChannelQuantizedTensor(
          tflite::testing::kFilterData, filter_quantized, filter_dims,
          filter_scales, filter_zero_points, &filter_quant,
          0 /* quantized dimension */);
  TfLiteTensor bias_tensor =
      tflite::testing::CreatePerChannelQuantizedBiasTensor(
          tflite::testing::kBiasData, bias_quantized, bias_dims, input_scale,
          &filter_scales[1], scales, zero_points, &bias_quant, 0);
  TfLiteTensor output_tensor = tflite::testing::CreateQuantizedTensor(
      output_data, output_dims, output_scale, 0 /* quantized dimension */);

  float input_scales[] = {1, input_scale};
  int input_zero_points[] = {1, 128};
  TfLiteAffineQuantization input_quant = {
      tflite::testing::FloatArrayFromFloats(input_scales),
      tflite::testing::IntArrayFromInts(input_zero_points), 0};
  input_tensor.quantization = {kTfLiteAffineQuantization, &input_quant};

  constexpr int inputs_size = 3;
  constexpr int outputs_size = 1;
  constexpr int tensors_size = inputs_size + outputs_size;
  TfLiteTensor tensors[tensors_size] = {
      input_tensor,
      filter_tensor,
      bias_tensor,
      output_tensor,
  };

  tflite::Quantize(tflite::testing::kGoldenData, golden_quantized,
                   output_dims_count, output_scale, 0);

  // Set filter quant to mismatched dimension.
  TfLiteAffineQuantization* quant = reinterpret_cast<TfLiteAffineQuantization*>(
      filter_tensor.quantization.params);

  // Choose arbitrary incorrect scale and zero point sizes which are neither 1
  // (for broadcast case) nor the quantized dimension size.
  quant->scale->size = 2;
  TF_LITE_MICRO_EXPECT_EQ(
      kTfLiteError,
      tflite::testing::ValidateConvGoldens(
          tensors, tensors_size, golden_quantized, output_data,
          output_dims_count, &tflite::testing::common_conv_params));

  // Set scale back to correct dimension, and make zero point array too short.
  quant->scale->size = tflite::testing::kFilterShape[0];
  quant->zero_point->size = 2;
  TF_LITE_MICRO_EXPECT_EQ(
      kTfLiteError,
      tflite::testing::ValidateConvGoldens(
          tensors, tensors_size, golden_quantized, output_data,
          output_dims_count, &tflite::testing::common_conv_params));
}

TF_LITE_MICRO_TEST(Int8Input32x1Filter32x32ShouldMatchGolden) {
  constexpr int kSampleSize = 32;
  constexpr int kNumFilters = 32;
  const int input_shape[] = {4, 1, 1, 1, kSampleSize};
  const int filter_shape[] = {4, kNumFilters, 1, 1, kSampleSize};
  const int bias_shape[] = {1, kSampleSize};
  const int output_shape[] = {4, 1, 1, 1, kSampleSize};
  float filter_values[kNumFilters * kSampleSize];
  float input_values[kSampleSize];
  float bias_values[kSampleSize];

  // Generated these outputs using the floating point reference conv kernel.
  // TODO(b/149942509): Do this comparison automatically on random inputs.
  float expected_output[kSampleSize] = {
      5168.000000,  3377.000000,  306.000000,   -4045.000000, -4556.000000,
      -1227.000000, 822.000000,   1591.000000,  5176.000000,  3385.000000,
      314.000000,   -4037.000000, -4548.000000, -1219.000000, 830.000000,
      1599.000000,  5184.000000,  3393.000000,  322.000000,   -4029.000000,
      -4540.000000, -1211.000000, 838.000000,   1607.000000,  5192.000000,
      3401.000000,  330.000000,   -4021.000000, -4532.000000, -1203.000000,
      846.000000,   1615.000000};

  for (int i = 0; i < kSampleSize; i++) {
    bias_values[i] = i;
    // Generate inputs from -16 to 15.
    input_values[i] = i - 16;
  }

  // Generate samples of varying values between -128 and 127.
  for (int i = 0; i < kNumFilters * kSampleSize; i++) {
    filter_values[i] = (i * 25) % 256 - 128;
  }

  TfLiteConvParams conv_params;
  conv_params.activation = kTfLiteActNone;
  conv_params.dilation_height_factor = 1;
  conv_params.dilation_width_factor = 1;
  conv_params.stride_height = 1;
  conv_params.stride_width = 1;
  conv_params.padding = kTfLitePaddingValid;

  TfLiteIntArray* input_dims = tflite::testing::IntArrayFromInts(input_shape);
  TfLiteIntArray* filter_dims = tflite::testing::IntArrayFromInts(filter_shape);
  TfLiteIntArray* bias_dims = tflite::testing::IntArrayFromInts(bias_shape);
  TfLiteIntArray* output_dims = tflite::testing::IntArrayFromInts(output_shape);
  const int output_dims_count = tflite::ElementCount(*output_dims);

  // Quantization Parameters.  All scales except output are 1.0, and all zero
  // points are 0. This direct-maps the values to floating point and makes it
  // easy to reson about them.
  int input_zero_point = 0;
  float input_scale = 1.0f;
  int filter_zero_point = 0;
  float filter_scale = 1.0f;
  int output_zero_point = 0;
  // Output scale of 50 is needed to accomodate a float range of [-6400, 6350]
  float output_scale = 50.0f;

  // Create per-tensor quantized int8_t input tensor.
  int8_t input_quantized[kSampleSize];
  TfLiteTensor input_tensor = tflite::testing::CreateQuantizedTensor(
      input_values, input_quantized, input_dims, input_scale, input_zero_point);
  // Set zero point and scale arrays with a single element for each.
  int input_zero_points[] = {1, input_zero_point};
  float input_scales[] = {1, input_scale};
  TfLiteAffineQuantization input_quant = {
      tflite::testing::FloatArrayFromFloats(input_scales),
      tflite::testing::IntArrayFromInts(input_zero_points), 0};
  input_tensor.quantization = {kTfLiteAffineQuantization, &input_quant};

  // Create per-tensor quantized int8_t filter tensor.
  int8_t filter_quantized[kNumFilters * kSampleSize];
  TfLiteTensor filter_tensor = tflite::testing::CreateQuantizedTensor(
      filter_values, filter_quantized, filter_dims, filter_scale,
      filter_zero_point);
  // Set zero point and scale arrays with a single element for each.
  int filter_zero_points[] = {1, filter_zero_point};
  float filter_scales[] = {1, filter_scale};
  TfLiteAffineQuantization filter_quant = {
      tflite::testing::FloatArrayFromFloats(filter_scales),
      tflite::testing::IntArrayFromInts(filter_zero_points), 0};
  filter_tensor.quantization = {kTfLiteAffineQuantization, &filter_quant};

  // Create per-tensor quantized int32_t bias tensor.
  int32_t bias_quantized[kSampleSize];
  tflite::SymmetricQuantize(bias_values, bias_quantized, kSampleSize,
                            input_scale * output_scale);
  TfLiteTensor bias_tensor =
      tflite::testing::CreateTensor(bias_quantized, bias_dims);

  // There is a single zero point of 0, and a single scale of
  // input_scale * filter_scale.
  int bias_zero_points[] = {1, 0};
  float bias_scales[] = {1, input_scale * filter_scale};
  TfLiteAffineQuantization bias_quant = {
      tflite::testing::FloatArrayFromFloats(bias_scales),
      tflite::testing::IntArrayFromInts(bias_zero_points), 0};
  bias_tensor.quantization = {kTfLiteAffineQuantization, &bias_quant};

  // Create per-tensor quantized int8_t output tensor.
  int8_t output_quantized[kSampleSize];
  TfLiteTensor output_tensor = tflite::testing::CreateQuantizedTensor(
      output_quantized, output_dims, output_scale, output_zero_point);
  // Set zero point and scale arrays with a single element for each.
  int output_zero_points[] = {1, output_zero_point};
  float output_scales[] = {1, output_scale};
  TfLiteAffineQuantization output_quant = {
      tflite::testing::FloatArrayFromFloats(output_scales),
      tflite::testing::IntArrayFromInts(output_zero_points), 0};
  output_tensor.quantization = {kTfLiteAffineQuantization, &output_quant};

  // The 3 inputs include the input, filter and bias tensors.
  constexpr int kInputsSize = 3;
  constexpr int kOutputsSize = 1;
  constexpr int kTensorsSize = kInputsSize + kOutputsSize;
  TfLiteTensor tensors[kTensorsSize] = {
      input_tensor,
      filter_tensor,
      bias_tensor,
      output_tensor,
  };

  int8_t golden_quantized[kSampleSize];
  tflite::Quantize(expected_output, golden_quantized, output_dims_count,
                   output_scale, output_zero_point);

  // Rounding errors due to quantization should not exceed 1.
  constexpr int kQuantizationTolerance = 1;

  TF_LITE_MICRO_EXPECT_EQ(
      kTfLiteOk, tflite::testing::ValidateConvGoldens(
                     tensors, kTensorsSize, golden_quantized, output_quantized,
                     output_dims_count, &conv_params, kQuantizationTolerance));
}

TF_LITE_MICRO_TESTS_END
