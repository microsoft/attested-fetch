# attested-fetch (archived)

An experimental tool for fetching content from an HTTPS URL within an SGX enclave.
The output is a JSON file containing SGX evidence and the HTTPS response.

An example implementation could be seen in [scitt-ccf-ledger](https://github.com/microsoft/scitt-ccf-ledger/blob/97799deb0d7f64690cc100102539ff9731ad5b7b/.gitmodules) prior to the removal of `did:web` issuer verification.

## Getting started

```sh
./build.sh
./test.sh
```

Run manually:
```sh
dist/afetch dist/libafetch.enclave.so out.json https://github.com nonce42
```

## Virtual mode

In addition to the normal mode of usage described above, attested fetch may be
used in a "virtual mode". In this mode, no SGX enclave is used. This can be
useful for development, but provides no verifiable security guarantees about
its output.

Virtual mode can be enabled by setting the `PLATFORM` variable:
```sh
export PLATFORM=virtual
./build.sh
./test.sh
```

## Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## Trademarks

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft 
trademarks or logos is subject to and must follow 
[Microsoft's Trademark & Brand Guidelines](https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks/usage/general).
Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship.
Any use of third-party trademarks or logos are subject to those third-party's policies.
