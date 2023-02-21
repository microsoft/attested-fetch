// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <exception>
#include <openenclave/enclave.h>
#include <openenclave/attestation/attester.h>
#include <openenclave/attestation/custom_claims.h>
#include <openenclave/attestation/sgx/evidence.h>
#include <nlohmann/json.hpp>

#include "curl.h"
#include "util.h"
#include "afetch_t.h"

extern "C" void enclave_main(const char* url, const char* nonce, char** output) {
    oe_load_module_host_socket_interface();
    oe_load_module_host_resolver();
    afetch::Curl::global_init();

    std::string format = "ATTESTED_FETCH_OE_SGX_ECDSA_V2";
    
    try {
        afetch::Curl curl;

        // Fetch URL
        auto response = curl.fetch(url);

        // Create output JSON
        nlohmann::json j;
        j["url"] = url;
        j["nonce"] = nonce;
        if (response.status == NULL) {
            j["error"]["message"] = response.error_message;
        } else {
            j["result"]["status"] = response.status;
            j["result"]["body"] = afetch::base64(response.body);
            j["result"]["certs"] = response.cert_chain;
        }

        std::string data_json = j.dump(1);
        std::vector<uint8_t> data_hash = afetch::sha256(data_json);
        std::string data = afetch::base64(data_json);

        std::vector<uint8_t> format_hash = afetch::sha256(format);
        std::vector<uint8_t> sgx_report_data = afetch::sha256_two(format_hash, data_hash);

        // Create SGX quote with digest of output JSON as report data
        auto rc = oe_attester_initialize();
        if (rc != OE_OK)
        {
            throw std::logic_error("Failed to initialise evidence attester");
        }

        const size_t custom_claim_length = 1;
        oe_claim_t custom_claim;
        custom_claim.name = const_cast<char*>(OE_CLAIM_SGX_REPORT_DATA);
        custom_claim.value = sgx_report_data.data();
        custom_claim.value_size = sgx_report_data.size();

        uint8_t* serialised_custom_claims_buf = nullptr;
        size_t serialised_custom_claims_size = 0;

        rc = oe_serialize_custom_claims(
            &custom_claim,
            custom_claim_length,
            &serialised_custom_claims_buf,
            &serialised_custom_claims_size);
        if (rc != OE_OK)
        {
            throw std::logic_error("Could not serialise OE custom claim");
        }

        uint8_t* evidence_buf;
        size_t evidence_size;
        uint8_t* endorsements_buf;
        size_t endorsements_size;

        oe_uuid_t oe_quote_format = {OE_FORMAT_UUID_SGX_ECDSA};

        rc = oe_get_evidence(
            &oe_quote_format,
            0,
            serialised_custom_claims_buf,
            serialised_custom_claims_size,
            nullptr,
            0,
            &evidence_buf,
            &evidence_size,
            &endorsements_buf,
            &endorsements_size);
        if (rc != OE_OK)
        {
            throw std::logic_error("Failed to get evidence");
        }

        std::string evidence = afetch::base64(evidence_buf, evidence_size);
        std::string endorsements = afetch::base64(endorsements_buf, endorsements_size);

        oe_free_serialized_custom_claims(serialised_custom_claims_buf);
        oe_free_evidence(evidence_buf);
        oe_free_endorsements(endorsements_buf);

        oe_attester_shutdown();

        // Create attested output
        nlohmann::json out;
        out["format"] = format;
        out["evidence"] = evidence;
        out["endorsements"] = endorsements;
        out["data"] = data;
        std::string out_str = out.dump(1);

        // Return to host
        *output = oe_host_strndup(out_str.data(), out_str.size());

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        abort();
    }

    afetch::Curl::global_cleanup();
}
