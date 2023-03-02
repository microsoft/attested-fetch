// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include <openssl/evp.h>

namespace afetch {

std::string base64(const uint8_t *data, size_t size) {
  // The size computation comes from the EVP_EncodeBlock docs: "For every 3
  // bytes of input provided 4 bytes of output data will be produced". The
  // number of chunks is rounded up to the next whole integer.
  //
  // EVP_EncodeBlock also writes a final null byte, which we don't include in
  // the size computation, since std::string already allocates space for one.
  // EVP_EncodeBlock will overwrite std::string's null byte, which is allowed
  // per https://wg21.link/LWG2475.

  size_t chunks = (size + 2) / 3;
  size_t output_size = chunks * 4;
  std::string result(output_size, '\0');
  EVP_EncodeBlock((uint8_t*)result.data(), data, size);
  return result;
}

std::string base64(const std::vector<uint8_t> &v) {
  return base64(v.data(), v.size());
}

std::string base64(const std::string &s) {
  return base64((uint8_t *)s.data(), s.size());
}

std::vector<uint8_t> sha256(const uint8_t* data, size_t size) {
  std::vector<uint8_t> hash;
  hash.resize(32);

  if (EVP_Digest(data, size, hash.data(), nullptr, EVP_sha256(), NULL) != 1)
  {
    throw std::logic_error("Could not compute sha256");
  }

  return hash;
}

std::vector<uint8_t> sha256(const std::string &s) {
  return sha256((uint8_t *)s.data(), s.size());
}

std::vector<uint8_t> sha256(const std::vector<uint8_t> &s) {
  return sha256(s.data(), s.size());
}

std::vector<uint8_t> sha256_two(const std::vector<uint8_t> &a,
                                const std::vector<uint8_t> &b) {
  std::vector<uint8_t> total;
  total.insert(total.end(), a.begin(), a.end());
  total.insert(total.end(), b.begin(), b.end());
  return sha256(total);
}

} // namespace afetch
