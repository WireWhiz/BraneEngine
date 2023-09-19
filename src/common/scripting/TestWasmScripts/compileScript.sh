

export RUSTFLAGS="--cfg=web_sys_unstable_apis -C target-feature=+atomics,+bulk-memory,+mutable-globals,+crt-static -C link-args=--no-entry -C link-arg=--shared-memory -C link-arg=--import-memory -C link-arg=--max-memory=2147483648"
echo "Compiling scripts with flags:" $RUSTFLAGS
#cargo +nightly build --target wasm32-wasi-preview1-threads
cargo +nightly build --target wasm32-unknown-unknown -Z build-std=std,panic_abort

