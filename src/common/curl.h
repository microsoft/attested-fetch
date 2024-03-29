// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "util.h"

namespace afetch {

  class CurlError : public std::runtime_error {
  public:
    explicit CurlError(const std::string& msg, const std::string& url)
        : std::runtime_error("Curl error: " + msg + " (" + url + ")"), url(url) {}

    const std::string url;
  };

  class Curl
  {
  public:
    struct Response {
      int64_t status;
      std::vector<uint8_t> body;
      std::vector<std::string> cert_chain;
    };

    static void global_init() {
      curl_global_init(CURL_GLOBAL_ALL);
    }

    static void global_cleanup() {
      curl_global_cleanup();
    }

    Curl(bool verbose = false) : verbose(verbose) {
      curl = curl_easy_init();
      if (curl == nullptr) {
        throw std::runtime_error("Error initializing curl");
      }
    }

    ~Curl() {
      curl_easy_cleanup(curl);
    }

    nlohmann::json fetch(const std::string& url, const std::string& nonce)
    {
      nlohmann::json j;
      j["url"] = url;
      j["nonce"] = nonce;

      try {
          auto response = fetch_response(url);
          j["result"]["status"] = response.status;
          j["result"]["body"] = base64(response.body);
          j["result"]["certs"] = response.cert_chain;
      }
      catch (afetch::CurlError& e) {
          j["error"]["message"] = e.what();
      }

      return j;
    }

    Response fetch_response(const std::string& url) {
      Response response;

      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
      curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
      curl_easy_setopt(curl, CURLOPT_VERBOSE, verbose ? 1L : 0L);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
      curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
      curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
      curl_easy_setopt(curl, CURLOPT_CERTINFO, 1L);
      curl_easy_setopt(curl, CURLOPT_CAINFO, NULL);
      curl_easy_setopt(curl, CURLOPT_CAPATH, NULL);

      auto res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
        const std::string error_message = curl_easy_strerror(res);
        if (verbose) {
          std::cerr << "Fetch failed: " << error_message << std::endl;
        }
        throw CurlError(error_message, url);
      }

      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status);

      if (verbose) {
        std::cerr << "URL: " << url << std::endl;
        std::cerr << "Response status code: " << response.status << std::endl;
        std::cerr << "Response body:" << std::string((char*)response.body.data(), response.body.size()) << std::endl;
      }

      struct curl_certinfo *certinfo;
      res = curl_easy_getinfo(curl, CURLINFO_CERTINFO, &certinfo);

      if (res == CURLE_OK) {
        if (verbose) {
          std::cerr << "Certs: " << certinfo->num_of_certs << std::endl;
        }
        for (size_t i = 0; i < certinfo->num_of_certs; i++) {
          for (curl_slist* slist = certinfo->certinfo[i]; slist; slist = slist->next) {
            std::string s(slist->data);
            if (s.rfind("Cert:", 0) == 0) {
              auto pem = s.substr(5);
              if (verbose) {
                std::cerr << pem << std::endl;
              }
              response.cert_chain.push_back(std::move(pem));
            }
          }
        }
      }

      return response;
    }

  private:
    static size_t write_callback(void* ptr, size_t size, size_t nmemb, std::vector<uint8_t>* v) {
      auto data = reinterpret_cast<uint8_t*>(ptr);
      v->insert(v->end(), data, data + size * nmemb);
      return size * nmemb;
    }

    CURL* curl;
    bool verbose;
  };
}
