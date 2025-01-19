/*
 * Copyright 2015 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "support/file.h"
#include "support/debug.h"
#include "support/path.h"
#include "support/utilities.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#endif

#define DEBUG_TYPE "file"

std::vector<char> wasm::read_stdin() {
  BYN_TRACE("Loading stdin...\n");
  std::vector<char> input;
  char c;
#if defined(_WIN32)
  // Windows opens stdin in text mode. Set the file mode to binary so
  // that we can read Wasm binary files correctly.
  assert(_setmode(fileno(stdin), _O_BINARY));
#endif
  while (std::cin.get(c) && !std::cin.eof()) {
    input.push_back(c);
  }
  return input;
}

template<typename T> struct do_read_stdin { T operator()(); };

template<> std::vector<char> do_read_stdin<std::vector<char>>::operator()() {
  return wasm::read_stdin();
}

template<> std::string do_read_stdin<std::string>::operator()() {
  auto vec = wasm::read_stdin();
  return std::string(vec.begin(), vec.end());
}

template<typename T>
T wasm::read_file(const std::string& filename, Flags::BinaryOption binary) {
  if (filename == "-") {
    return do_read_stdin<T>{}();
  }
  BYN_TRACE("Loading '" << filename << "'...\n");
  std::ifstream infile;
  std::ios_base::openmode flags = std::ifstream::in;
  if (binary == Flags::Binary) {
    flags |= std::ifstream::binary;
  }
  infile.open(wasm::Path::to_path(filename), flags);
  if (!infile.is_open()) {
    Fatal() << "Failed opening '" << filename << "'";
  }
  infile.seekg(0, std::ios::end);
  std::streampos insize = infile.tellg();
  if (uint64_t(insize) >= std::numeric_limits<size_t>::max()) {
    // Building a 32-bit executable where size_t == 32 bits, we are not able to
    // create strings larger than 2^32 bytes in length, so must abort here.
    Fatal() << "Failed opening '" << filename
            << "': Input file too large: " << insize
            << " bytes. Try rebuilding in 64-bit mode.";
  }
  // Zero-initialize the string or vector with the expected size.
  T input(size_t(insize), '\0');
  if (size_t(insize) == 0) {
    return input;
  }
  infile.seekg(0);
  infile.read(&input[0], insize);
  if (binary == Flags::Text) {
    size_t chars = size_t(infile.gcount());
    // Truncate size to the number of ASCII characters actually read in text
    // mode (which is generally less than the number of bytes on Windows, if
    // \r\n line endings are present)
    input.resize(chars);
  }
  return input;
}

std::string wasm::read_possible_response_file(const std::string& input) {
  if (input.size() == 0 || input[0] != '@') {
    return input;
  }
  return wasm::read_file<std::string>(input.substr(1), Flags::Text);
}

// Explicit instantiations for the explicit specializations.
template std::string wasm::read_file<>(const std::string&, Flags::BinaryOption);
template std::vector<char> wasm::read_file<>(const std::string&,
                                             Flags::BinaryOption);

wasm::Output::Output(const std::string& filename, Flags::BinaryOption binary)
  : outfile(), out([this, filename, binary]() {
      // Ensure a single return at the very end, to avoid clang-tidy warnings
      // about the types of different returns here.
      std::streambuf* buffer;
      if (filename == "-" || filename.empty()) {
#if defined(_WIN32)
        // Windows opens stdout in text mode. Set the file mode to binary
        // so that we can output Wasm binary files correctly.
        assert(_setmode(fileno(stdout), _O_BINARY));
#endif
        buffer = std::cout.rdbuf();
      } else {
        BYN_TRACE("Opening '" << filename << "'\n");
        std::ios_base::openmode flags =
          std::ofstream::out | std::ofstream::trunc;
        if (binary == Flags::Binary) {
          flags |= std::ofstream::binary;
        }
        outfile.open(wasm::Path::to_path(filename), flags);
        if (!outfile.is_open()) {
          Fatal() << "Failed opening output file '" << filename
                  << "': " << strerror(errno);
        }
        buffer = outfile.rdbuf();
      }
      return buffer;
    }()) {}

void wasm::copy_file(std::string input, std::string output) {
  std::ifstream src(wasm::Path::to_path(input), std::ios::binary);
  std::ofstream dst(wasm::Path::to_path(output), std::ios::binary);
  dst << src.rdbuf();
}

size_t wasm::file_size(std::string filename) {
  std::ifstream infile(wasm::Path::to_path(filename),
                       std::ifstream::ate | std::ifstream::binary);
  return infile.tellg();
}
