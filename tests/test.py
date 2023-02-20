from pathlib import Path
import json
import base64
import hashlib
import subprocess
import tempfile

THIS_DIR = Path(__file__).parent
DIST_DIR = THIS_DIR.parent / 'dist'

OEVERIFY = "/opt/openenclave/bin/oeverify"

TEST_URL = "https://www.microsoft.com/en-gb/"
TEST_URL_NON_2XX = "https://www.microsoft.com/en-gbb/foo"
TEST_URL_REFUSED = "https://localhost:1"
TEST_NONCE = "nonce123"


def test(url, expected_status):
    out_path = "test.json"

    # Run afetch
    subprocess.run([
        DIST_DIR / "afetch",
        DIST_DIR / "libafetch.enclave.so.signed",
        out_path,
        url, TEST_NONCE
        ], check=True)
    
    # Read attested result
    with open(out_path) as f:
        out = json.load(f)

    # Check if the format is known
    if out["format"] != "ATTESTED_FETCH_OE_SGX_ECDSA":
        raise RuntimeError(f"Unsupported format: {out['format']}")
    
    # Verify evidence and endorsements using Open Enclave
    with tempfile.TemporaryDirectory() as tmpdir:
        # Run oeverify
        evidence_path = Path(tmpdir) / "evidence.bin"
        endorsements_path = Path(tmpdir) / "endorsements.bin"
        with open(evidence_path, "wb") as f:
            f.write(base64.b64decode(out["evidence"]))
        with open(endorsements_path, "wb") as f:
            f.write(base64.b64decode(out["endorsements"]))
        result = subprocess.run([
            OEVERIFY, "-r", evidence_path, "-e", endorsements_path
            ], capture_output=True, universal_newlines=True, check=True)
        
        # Extract report data from stdout
        prefix = "sgx_report_data:"
        sgx_report_data = None
        for line in result.stdout.splitlines():
            if line.startswith(prefix):
                sgx_report_data = line[len(prefix):].strip()
        assert sgx_report_data is not None
        sgx_report_data = sgx_report_data[len("0x"):]

    # Check if data hash matches report data in evidence
    format_hash = hashlib.sha256(out["format"].encode("ascii")).digest()
    data_json = base64.b64decode(out["data"])
    data_hash = hashlib.sha256(data_json).digest()
    computed_sgx_report_data = hashlib.sha256(format_hash + data_hash).digest().hex()
    assert sgx_report_data == computed_sgx_report_data, f"{computed_sgx_report_data} != {sgx_report_data}"

    # Finally, check if the data is valid JSON
    data = json.loads(data_json)
    assert data["nonce"] == TEST_NONCE, data["nonce"]
    assert data["url"] == url, data["url"]
    assert data["status"] == expected_status, data["status"]
    if url != TEST_URL_REFUSED:
        assert len(data["certs"]) > 0, data["certs"]
        assert len(data["body"]) > 0, data["body"]
    else:
        print(data) 
        assert len(data["certs"]) == 0, data["certs"]
        assert len(data["body"]) == 0, data["body"]


if __name__ == "__main__":
    test(TEST_URL, 200)
    print("Mainline test succeeded!")
    test(TEST_URL_NON_2XX, 404)
    print("Non 2xx response test succeeded!")
    test(TEST_URL_REFUSED, 0)
    print("Connection refused test succeeded!")

