// Copyright 2024 Google LLC
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stddef.h>

#include <string>

// Placeholder for internal header, do not modify.
#include "third_party/gemma_cpp/examples/simplified_gemma/gemma.hpp"
#include "util/app.h"  // LoaderArgs

int main(int argc, char** argv) {
  {
    // Placeholder for internal init, do not modify.
  }

  // Standard usage: LoaderArgs takes argc and argv as input, then parses
  // necessary flags.
  gcpp::LoaderArgs loader(argc, argv, /*validate=*/true);

  // Optional: LoaderArgs can also take tokenizer and weights paths directly.
  //
  // gcpp::LoaderArgs loader("/path/to/tokenizer", "/path/to/weights",
  // "model_identifier");

  // Optional: InferenceArgs and AppArgs can be passed in as well. If not
  // specified, default values will be used.
  //
  // gcpp::InferenceArgs inference(argc, argv);
  // gcpp::AppArgs app(argc, argv);
  // SimplifiedGemma gemma(loader, inference, app);

  SimplifiedGemma gemma(loader);
  std::string prompt = "Write a greeting to the world.";
  gemma.Generate(prompt, 256, 0.6);

  return 0;
}