add_library(printf OBJECT EXCLUDE_FROM_ALL
  printf.c
  strtof.c
)

target_compile_options(printf PUBLIC -fno-builtin-printf)
target_compile_options(printf PRIVATE -DPRINTF_INCLUDE_CONFIG_H=1)
target_compile_options(printf PRIVATE -Wno-double-promotion)
