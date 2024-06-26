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

// Command line tool to create compressed weights.

#include <iostream>
#include <string>

// copybara:import_next_line:gemma_cpp
#include "gemma.h"  // Gemma
// copybara:end
// copybara:import_next_line:gemma_cpp
#include "util/args.h"
// copybara:end

namespace gcpp {

struct Args : public ArgsBase<Args> {
  static constexpr size_t kDefaultNumThreads = ~size_t{0};

  void ChooseNumThreads() {
    if (num_threads == kDefaultNumThreads) {
      // This is a rough heuristic, replace with something better in the future.
      num_threads = static_cast<size_t>(std::clamp(
          static_cast<int>(std::thread::hardware_concurrency()) - 2, 1, 18));
    }
  }

 public:
  Args(int argc, char* argv[]) {
    InitAndParse(argc, argv);
    ChooseNumThreads();
  }

  static std::string ToLower(const std::string& text) {
    std::string result = text;
    std::transform(begin(result), end(result), begin(result),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
  }

  gcpp::Model ModelType() const {
    const std::string model_type_lc = ToLower(model_type);
    if (model_type_lc.substr(0, 2) == "2b") {
      return gcpp::Model::GEMMA_2B;
    } else if (model_type_lc.substr(0, 2) == "7b") {
      return gcpp::Model::GEMMA_7B;
    } else {
      HWY_ABORT("Unknown model type %s", model_type_lc.c_str());
    }
  }

  // Returns error string or nullptr if OK.
  const char* Validate() const {
    const std::string model_type_lc = ToLower(model_type);
    if (model_type.empty()) {
      return "Missing --model flag, need to specify either 2b-pt, 7b-pt, "
             "2b-it, 7b-it.";
    }
    if (model_type_lc != "2b-pt" && model_type_lc != "7b-pt" &&
        model_type_lc != "2b-it" && model_type_lc != "7b-it") {
      return "Model type must be 2b-pt, 7b-pt, 2b-it, 7b-it.";
    }
    if (weights.path.empty()) {
      return "Missing --weights flag, a file for the uncompressed model.";
    }
    if (compressed_weights.path.empty()) {
      return "Missing --compressed_weights flag, a file for the compressed "
             "model.";
    }
    if (!weights.exists()) {
      return "Can't open file specified with --weights flag.";
    }
    return nullptr;
  }

  Path weights;             // uncompressed weights file location
  Path compressed_weights;  // compressed weights file location
  std::string model_type;
  size_t num_threads;

  template <class Visitor>
  void ForEach(const Visitor& visitor) {
    visitor(weights, "weights", Path(),
            "Path name of model weights (.sbs) file.\n"
            "    Required argument.");
    visitor(model_type, "model", std::string(),
            "Model type\n    2b-it = 2B parameters, instruction-tuned\n    "
            "2b-pt = 2B parameters, pretrained\n    7b-it = 7B parameters "
            "instruction-tuned\n    7b-pt = 7B parameters, pretrained\n    "
            "    Required argument.");
    visitor(compressed_weights, "compressed_weights", Path(),
            "Path name where compressed weights file will be written.\n"
            "    Required argument.");
    visitor(num_threads, "num_threads",
            kDefaultNumThreads,  // see ChooseNumThreads
            "Number of threads to use.\n    Default = Estimate of the "
            "number of suupported concurrent threads.",
            2);
  }
};

void ShowHelp(gcpp::Args& args) {
  std::cerr
      << "Usage:\n./compress_weights --weights <path to uncompressed weights> "
      " --model <model type> --compressed_weights <output path>\n";
  std::cerr << "\n*Arguments*\n\n";
  args.Help();
  std::cerr << "\n";
}

void Run(Args& args) {
  hwy::ThreadPool pool(args.num_threads);
  gcpp::CompressWeights(args.ModelType(), args.weights, args.compressed_weights,
                        pool);
}

}  // namespace gcpp

int main(int argc, char** argv) {
  gcpp::Args args(argc, argv);

  if (gcpp::HasHelp(argc, argv)) {
    ShowHelp(args);
    return 0;
  }

  if (const char* error = args.Validate()) {
    ShowHelp(args);
    HWY_ABORT("\nInvalid args: %s", error);
  }

  gcpp::Run(args);

  return 0;
}
