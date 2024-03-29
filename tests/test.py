from pathlib import Path
import json
import base64
import hashlib
import subprocess
import tempfile
import os

THIS_DIR = Path(__file__).parent
DIST_DIR = THIS_DIR.parent / 'dist'

OEVERIFY = "/opt/openenclave/bin/oeverify"

TEST_URL_2XX = "https://www.microsoft.com/en-gb/"
TEST_URL_NON_2XX = "https://www.microsoft.com/en-gbb/foo"
TEST_URL_REFUSED = "https://localhost:1"
TEST_NONCE = "nonce123"

PLATFORM = os.getenv("PLATFORM", "sgx")

def validate_virtual(envelope):
    if envelope["format"] != "ATTESTED_FETCH_VIRTUAL":
        raise RuntimeError(f"Unsupported format: {envelope['format']}")

    return json.loads(base64.b64decode(envelope["data"]))


def validate_sgx(envelope):
    if envelope["format"] != "ATTESTED_FETCH_OE_SGX_ECDSA_V2":
        raise RuntimeError(f"Unsupported format: {envelope['format']}")

    # Verify evidence and endorsements using Open Enclave
    with tempfile.TemporaryDirectory() as tmpdir:
        # Run oeverify
        evidence_path = Path(tmpdir) / "evidence.bin"
        endorsements_path = Path(tmpdir) / "endorsements.bin"
        with open(evidence_path, "wb") as f:
            f.write(base64.b64decode(envelope["evidence"]))
        with open(endorsements_path, "wb") as f:
            f.write(base64.b64decode(envelope["endorsements"]))
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
    format_hash = hashlib.sha256(envelope["format"].encode("ascii")).digest()
    data_json = base64.b64decode(envelope["data"])
    data_hash = hashlib.sha256(data_json).digest()
    computed_sgx_report_data = hashlib.sha256(format_hash + data_hash).digest().hex()
    assert sgx_report_data == computed_sgx_report_data, f"{computed_sgx_report_data} != {sgx_report_data}"

    return json.loads(data_json)


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

    if PLATFORM == "virtual":
        data = validate_virtual(out)
    elif PLATFORM == "sgx":
        data = validate_sgx(out)
    else:
        raise ValueError(f"Unsupported platform: {PLATFORM}")

    # Check if the data matches our expectations
    assert data["nonce"] == TEST_NONCE, data["nonce"]
    assert data["url"] == url, data["url"]
    if expected_status is None:
        assert "result" not in data, data["result"]
        assert data["error"]["message"] == "Curl error: Couldn't connect to server (https://localhost:1)", data["error"]["message"]
    else:
        assert data["result"]["status"] == expected_status, data["result"]["status"]
        assert len(data["result"]["certs"]) > 0, data["result"]["certs"]
        assert len(data["result"]["body"]) > 0, data["result"]["body"]
        assert "error" not in data, data["error"]


if __name__ == "__main__":
    test(TEST_URL_2XX, 200)
    print("2xx response test succeeded!")
    test(TEST_URL_NON_2XX, 404)
    print("Non 2xx response test succeeded!")
    test(TEST_URL_REFUSED, None)
    print("Connection refused test succeeded!")
