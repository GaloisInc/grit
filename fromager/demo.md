Repos:

* fromager/cheesecloth/grit>, `master` branch
* fromager/cheesecloth/microram>, `no-implicit-flag` branch
* fromager/cheesecloth/witness-checker>, `grit-support` branch
* marc/swanky> ???

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

    The argument `5000` sets the number of steps to execute.  **Note: currently, `grit` traces with more than 497 steps produce ZKIF files over 2GB, which cannot be parsed by downstream tools.**  You may wish to set the step count to 497 until this is fixed.

    The execution trace and generated advice will be written to `grit.cbor`.  CBOR is a binary format, but can be pretty-printed:

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

    **Note: currently, generating ZKIF output for a full `grit` run requires 60GB of RAM and 20GB of disk space, and produces an invalid ZKIF file.**   If you passed a step count of 497 to MicroRAM, you must add `--validate-only` at the end of the `cargo run` command line, as no memory error occurs in the first 497 steps of the trace; this will result in a valid ZKIF file.  On the other hand, if you generated the full trace but want to skip ZKIF output, remove the `--zkif-out out/grit` arguments.

    ZKIF output will be written into the `out/grit/` directory.

    For a report of the number of gates in the original and lowered abstract circuits, run the tool with `--stats`.  This output is very verbose, so pipe stdout and stderr to a log file.  Then grep the log for `all gates` to see the grand totals before and after lowering.

    For stats on the generated ZKIF, install the zkinterface tools with `cargo install zkinterface`, then run `zkif stats out/grit`.

 4. Check the ZKIF output with Quark ???
