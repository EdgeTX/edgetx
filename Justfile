# EdgeTX code generation helpers.
#
# These recipes wrap existing scripts that regenerate files COMMITTED to git.
# After running one, review and commit the result.
#
# The codegen recipes run on the host and need the tools installed locally. The
# docker- variants run the same scripts in the dev container instead, which
# already has them. CI uses the host recipes, as its jobs run in that image.

IMAGE := "ghcr.io/edgetx/edgetx-dev:latest"

# --user keeps generated files owned by the invoking user rather than root
_docker := 'docker run --rm --user "$(id -u):$(id -g)" -v "$PWD":/src -w /src ' + IMAGE

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
# Recreates ./build from scratch, and needs the libclang version from the
# edgetx-dev container.
[doc('Regenerate the YAML parsers (radio/src/storage/yaml/yaml_datastructs_*.cpp)')]
[group('codegen')]
gen-yaml FLAVOR='':
    FLAVOR="{{ FLAVOR }}" tools/generate-yaml.sh

[doc('Regenerate the YAML parsers, LVGL fonts and cfn sort order')]
[group('codegen')]
codegen: gen-fonts cfn-sort gen-yaml

[doc('Regenerate the LVGL fonts in the dev container')]
[group('codegen (docker)')]
docker-gen-fonts:
    {{ _docker }} radio/src/fonts/lvgl/make_fonts.sh

[doc('Regenerate the custom-function sort order in the dev container')]
[group('codegen (docker)')]
docker-cfn-sort:
    {{ _docker }} tools/cfn_sorter.sh

# Uses its own FetchContent cache: the default one is inside the repo, so it
# would be shared with host builds and their CMake state is not portable here.
[doc('Regenerate the YAML parsers in the dev container')]
[group('codegen (docker)')]
docker-gen-yaml FLAVOR='':
    {{ _docker }} env FLAVOR="{{ FLAVOR }}" \
        FETCHCONTENT_BASE_DIR=/src/.cache/fetchcontent-docker \
        tools/generate-yaml.sh

[doc('Regenerate everything in the dev container')]
[group('codegen (docker)')]
docker-codegen: docker-gen-fonts docker-cfn-sort docker-gen-yaml
