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
TEST_NONCE = "nonce123"

def test():
    out_path = "test.json"

    # Run afetch
    subprocess.run([
        DIST_DIR / "afetch",
        DIST_DIR / "libafetch.enclave.so",
        out_path,
        TEST_URL, TEST_NONCE
        ], check=True)
    
    # Read attested result
    with open(out_path) as f:
        out = json.load(f)
    
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

    # Check if data hash matches report data in evidence
    data_hash = "0x" + hashlib.sha256(out["data"].encode()).digest().hex()
    assert sgx_report_data == data_hash, f"{data_hash} != {sgx_report_data}"

    # Finally, check if the data is valid JSON
    data = json.loads(base64.b64decode(out["data"]))
    assert data["nonce"] == TEST_NONCE, data["nonce"]
    assert data["url"] == TEST_URL, data["url"]
    assert len(data["certs"]) > 0, data["certs"]
    assert len(data["body"]) > 0, data["body"]

if __name__ == "__main__":
    test()
    print("All tests succeeded!")
