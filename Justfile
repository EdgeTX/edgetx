# EdgeTX code generation helpers.
#
# These recipes wrap existing scripts that regenerate files COMMITTED to git.
# After running one, review and commit the result.
#
# Reference environment is ghcr.io/edgetx/edgetx-dev:latest (see
# .devcontainer/devcontainer.json). CI checks committed output for drift
# against that image.

# Show available recipes
default:
    @just --list

# Needs: lv_font_conv (npm), python3, gcc, and the lvgl submodule checked out.
[doc('Regenerate the LVGL fonts (radio/src/fonts/lvgl/{std,sml,lrg}/lv_font_*.c)')]
[group('codegen')]
gen-fonts:
    radio/src/fonts/lvgl/make_fonts.sh

# Needs: a C++ compiler, plus the 19 system locales listed at the top of
# tools/cfn_sorter.sh (apt install locales && locale-gen ...).
[doc('Regenerate the custom-function sort order (radio/src/cfn_sort.cpp)')]
[group('codegen')]
cfn-sort:
    tools/cfn_sorter.sh

# FLAVOR is a semicolon-separated target list; empty uses the script's default.
# WARNING: generate-yaml.sh deletes and recreates ./build.
# Needs the libclang version shipped in the edgetx-dev container.
[doc('Regenerate the YAML parsers (radio/src/storage/yaml/yaml_datastructs_*.cpp)')]
[group('codegen')]
gen-yaml FLAVOR='':
    FLAVOR="{{ FLAVOR }}" tools/generate-yaml.sh

[doc('Regenerate the YAML parsers, LVGL fonts and cfn sort order')]
[group('codegen')]
codegen: gen-fonts cfn-sort gen-yaml
