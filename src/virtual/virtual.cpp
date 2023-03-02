#include <fstream>
#include "curl.h"
#include "util.h"

int main(int argc, const char* argv[])
{
    if (argc != 5)
    {
        std::cerr << "Usage: " << argv[0] << " enclave_file out_file url nonce" << std::endl;
        return 1;
    }

    const char* out_file = argv[2];
    const char* url = argv[3];
    const char* nonce = argv[4];

    afetch::Curl::global_init();

    afetch::Curl curl;
    std::string data_json = curl.fetch(url, nonce).dump(1);
    std::string data = afetch::base64(data_json);

    nlohmann::json output;
    output["format"] = "ATTESTED_FETCH_VIRTUAL";
    output["data"] = data;

    std::ofstream out_stream(out_file);
    out_stream << output.dump(1);
    out_stream.close();

    afetch::Curl::global_cleanup();

    return 0;
}
