# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/workspaces/edgetx-sw")
  file(MAKE_DIRECTORY "/workspaces/edgetx-sw")
endif()
file(MAKE_DIRECTORY
  "/workspaces/edgetx-sw/native"
  "/workspaces/edgetx-sw/native-prefix"
  "/workspaces/edgetx-sw/native-prefix/tmp"
  "/workspaces/edgetx-sw/native-prefix/src/native-stamp"
  "/workspaces/edgetx-sw/native-prefix/src"
  "/workspaces/edgetx-sw/native-prefix/src/native-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/workspaces/edgetx-sw/native-prefix/src/native-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/workspaces/edgetx-sw/native-prefix/src/native-stamp${cfgdir}") # cfgdir has leading slash
endif()
