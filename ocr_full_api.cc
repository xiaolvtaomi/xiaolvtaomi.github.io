// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#include <gflags/gflags.h>
#include <stdio.h>
#include <vector>
#include "paddle_api.h"          // NOLINT
#include "paddle_use_kernels.h"  // NOLINT
#include "paddle_use_ops.h"      // NOLINT
#include "paddle_use_passes.h"   // NOLINT
using namespace paddle::lite_api;  // NOLINT

DEFINE_string(model_dir, "", "Model dir path.");
DEFINE_bool(prefer_int8_kernel, false, "Prefer to run model with int8 kernels");

int64_t ShapeProduction(const shape_t& shape) {
  int64_t res = 1;
  for (auto i : shape) res *= i;
  return res;
}

void RunModel() {
  // 1. Set CxxConfig
  CxxConfig config;
  config.set_model_dir(FLAGS_model_dir);
  std::vector<Place> valid_places{Place{TARGET(kARM), PRECISION(kFloat)},Place{TARGET(kHost), PRECISION(kFloat)}};
  config.set_preferred_place(Place{TARGET(kARM), PRECISION(kFloat)});
  config.set_valid_places(valid_places);

  // 2. Create PaddlePredictor by CxxConfig
  std::shared_ptr<PaddlePredictor> predictor =
      CreatePaddlePredictor<CxxConfig>(config);

  // 3. Prepare input data
  //input 0
  std::unique_ptr<Tensor> input_tensor(std::move(predictor->GetInput(1)));
  input_tensor->Resize(shape_t({1,3,100,380}));
  auto* data = input_tensor->mutable_data<float>();
  for(int i = 0; i < ShapeProduction(input_tensor->shape()); ++i){
      data[i] = 1;
  }
  //input1
  std::unique_ptr<Tensor> init_ids(std::move(predictor->GetInput(0)));
  init_ids->Resize(shape_t({1,1}));
  auto* data_ids = init_ids->mutable_data<float>();
  for(int i = 0; i < ShapeProduction(init_ids->shape()); ++i){
      data_ids[i] = 0;
  }
  lod_t lod_i{{0,1},{0,1}};
  init_ids->SetLoD(lod_i);
  //input2
  std::unique_ptr<Tensor> init_scores(std::move(predictor->GetInput(2)));
  init_scores->Resize(shape_t({1,1}));
  auto* data_scores = init_scores->mutable_data<float>();
  for(int i = 0; i < ShapeProduction(init_scores->shape()); ++i){
      data_scores[i] = 0;
  }
  lod_t lod_s{{0,1},{0,1}};
  init_scores->SetLoD(lod_s);


  // 4. Run predictor
  predictor->Run();

  // 5. Get output 
  std::unique_ptr<const Tensor> output_tensor(
      std::move(predictor->GetOutput(0)));
  for (int i = 0; i < ShapeProduction(output_tensor->shape()); i ++) {
    printf("Output[%d]: %f\n", i, output_tensor->data<float>()[i]);
  }
}

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  RunModel();
  return 0;
}




