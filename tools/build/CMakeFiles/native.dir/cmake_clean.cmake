file(REMOVE_RECURSE
  "CMakeFiles/native"
  "CMakeFiles/native-complete"
  "native-prefix/src/native-stamp/native-build"
  "native-prefix/src/native-stamp/native-configure"
  "native-prefix/src/native-stamp/native-download"
  "native-prefix/src/native-stamp/native-install"
  "native-prefix/src/native-stamp/native-mkdir"
  "native-prefix/src/native-stamp/native-patch"
  "native-prefix/src/native-stamp/native-update"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/native.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
