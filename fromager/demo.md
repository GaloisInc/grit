Repos:

* fromager/cheesecloth/grit>, `master` branch
* fromager/cheesecloth/microram>, `no-implicit-flag` branch
* fromager/cheesecloth/witness-checker>, `grit-support` branch
* marc/swanky>, `mac-and-cheese` branch

Other dependencies:

* Clang + LLVM toolchain (version 9)
* `stack` (for Haskell builds)
* `cargo` (for Rust builds)

Build steps:

 1. Compile the `grit` driver to LLVM:

    ```sh
    cd grit
    bash fromager/build.sh
    ```

    If your Clang/LLVM toolchain uses a version suffix (for example, `clang-9`), then set `LLVM_SUFFIX` before building:

    ```sh
    LLVM_SUFFIX=-9 bash fromager/build.sh
    ```

    Using Clang/LLVM version 9 is recommended, for compatibility with the LLVM parsing library used in MicroRAM.

    The LLVM output is written to `driver-link.ll`.

 2. Compile to MicroRAM and produce a trace:

    ```sh
    cd MicroRAM
    stack run compile -- --from-llvm ../grit/driver-link.ll 5000 -o grit.cbor --verbose
    ```

    The argument `5000` sets the number of steps to execute.  The execution trace and generated advice will be written to `grit.cbor`.  CBOR is a binary format, but can be pretty-printed:

    ```sh
    python3 -c 'import cbor,pprint,sys; pprint.pprint(cbor.load(sys.stdin.buffer))' <grit.cbor
    ```

    (Requires installing the `cbor` package for Python 3.)

 3. Generate a witness checker circuit for the trace:

    ```sh
    cd witness-checker
    cargo run --release --features bellman -- ../MicroRAM/grit.cbor --zkif-out out/grit
    ```

    This should produce some output indicating that it detected a memory error:

    ```
    found bug: access of poisoned address 1c00000010000258 on cycle 2161
    internal evaluator: 133890 asserts passed, 0 failed; found 1 bugs; overall status: GOOD
    ```

    It will then generate constraints in ZKIF format and write them to files in the `out/grit/` directory.  This requires about 20GB of free disk space and about 10GB of RAM.  When finished, it will validate the generated constraints and print some statistics.

    For a report of the number of gates in the original and lowered abstract circuits, run the tool with `--stats` (and without `--zkif-out out/grit`).  This output is very verbose, so pipe stdout and stderr to a log file.  Then grep the log for `all gates` to see the grand totals before and after lowering.

 4. Check the ZKIF output with Quark.  This has two parts.

    First, start the prover server:

    ```
    cd swanky
    cargo run --release --example mac-n-cheese-zkif -- \
        prove \
        --listen-addr 127.0.0.1:4433 \
        --tls-cert mac-n-cheese/network/test-certs/localhost.fullchain.crt \
        --tls-key mac-n-cheese/network/test-certs/localhost.key \
        ../witness-checker/out/grit/{header,constraints,witness}.zkif
    ```

    In a second terminal, run the verifier:

    ```
    cd swanky
    cargo run --release --example mac-n-cheese-zkif -- \
        verify \
        --root-ca mac-n-cheese/network/test-certs/rootCA.crt \
        --host localhost --port 4433 \
        ../witness-checker/out/grit/{header,constraints}.zkif
    ```

    This part requires about 60GB RAM and takes about 15 minutes to run.  Once it finishes, the verifier should print a success message and exit:

    ```
    2020-10-19 09:46:24,540 INFO  [mac_n_cheese_network] verified successfully 335433094555710354779503521424195742509. VOLE stats read 0 bytes. wrote 0 bytes.. proving stats read 3645060096 bytes. wrote 32 bytes.
    ```

    The prover will keep running, so you should kill it with `^C` at this point.
