include(FetchContent)

set(Rust_CARGO_TARGET "thumbv7em-none-eabihf")

FetchContent_Declare(
    Corrosion
    GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
    GIT_TAG v0.3.5 # Optionally specify a commit hash, version tag or branch here
)
FetchContent_MakeAvailable(Corrosion)

corrosion_import_crate(MANIFEST_PATH ${RADIO_SRC_DIR}/rs/Cargo.toml NO_STD)
corrosion_set_linker(rs "ld")

include_directories(${RADIO_SRC_DIR}/rs/pub_headers)

target_link_options(firmware PUBLIC "-Wl,--allow-multiple-definition")
target_link_options(firmware PUBLIC "-static")
target_link_libraries(firmware PUBLIC rs)
