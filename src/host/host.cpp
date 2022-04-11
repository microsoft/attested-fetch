// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <iostream>
#include <fstream>

#include <openenclave/host.h>

#include "afetch_u.h"

int main(int argc, const char* argv[])
{
    oe_result_t result;
    int ret = 1;
    oe_enclave_t* enclave = NULL;

    uint32_t flags = OE_ENCLAVE_FLAG_DEBUG_AUTO;

    if (argc != 5)
    {
        std::cerr << "Usage: " << argv[0] << " enclave_file out_file url nonce" << std::endl;
        return 1;
    }

    const char* enclave_file = argv[1];
    const char* out_file = argv[2];
    const char* url = argv[3];
    const char* nonce = argv[4];

    // Create the enclave
    result = oe_create_afetch_enclave(
        enclave_file, OE_ENCLAVE_TYPE_AUTO, flags, NULL, 0, &enclave);
    if (result != OE_OK)
    {
        std::cerr << "oe_create_fetch_enclave(): result=" << result 
                  << " (" << oe_result_str(result) << ")" << std::endl;
        return 1;
    }

    // Call into the enclave
    char* out;
    result = enclave_main(enclave, url, nonce, &out);
    if (result != OE_OK)
    {
        std::cerr << "calling into enclave_main failed: result=" << result 
                  << " (" << oe_result_str(result) << ")" << std::endl;
        return 1;
    }

    oe_terminate_enclave(enclave);

    std::ofstream out_stream(out_file);
    out_stream << out;
    out_stream.close();

    free(out);
    
    return 0;
}