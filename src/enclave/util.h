// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <mbedtls/base64.h>
#include <mbedtls/sha256.h>

namespace afetch {
  std::string base64(const uint8_t* data, size_t size)
  {
    size_t len_written = 0;

    // Obtain required size for output buffer
    auto rc = mbedtls_base64_encode(nullptr, 0, &len_written, data, size);
    if (rc < 0 && rc != MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL)
    {
      throw std::logic_error(
        "Could not obtain length required for encoded base64 buffer");
    }

    std::string b64_string(len_written, '\0');
    auto dest = (uint8_t*)(b64_string.data());

    rc = mbedtls_base64_encode(dest, b64_string.size(), &len_written, data, size);
    if (rc != 0)
    {
      throw std::logic_error(
        "Could not encode base64 string");
    }

    if (b64_string.size() > 0)
    {
      // mbedtls includes the terminating null, but std-string provides this
      // already
      b64_string.pop_back();
    }

    return b64_string;
  }

  std::string base64(const std::vector<uint8_t>& v)
  {
      return base64(v.data(), v.size());
  }

  std::string base64(const std::string& s)
  {
      return base64((uint8_t*)s.data(), s.size());
  }

  std::vector<uint8_t> sha256(uint8_t* data, size_t data_len)
  {
    std::vector<uint8_t> hash;
    hash.resize(32);
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts_ret(&ctx, 0);
    mbedtls_sha256_update_ret(&ctx, data, data_len);
    mbedtls_sha256_finish_ret(&ctx, hash.data());
    mbedtls_sha256_free(&ctx);
    return hash;
  }
  
  std::vector<uint8_t> sha256(const std::string& s)
  {
    return sha256((uint8_t*)s.data(), s.size());
  }
}
