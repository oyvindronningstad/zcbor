FILE(GLOB app_sources src/*.c)
FILE(GLOB app_sources_cpp src/*.cpp)
target_sources(app PRIVATE
  ${app_sources} ${app_sources_cpp}
  )

if (VERBOSE)
  zephyr_compile_definitions(ZCBOR_VERBOSE)
endif()

if (ASSERTS)
  zephyr_compile_definitions(ZCBOR_ASSERTS)
endif()

if (CANONICAL)
  zephyr_compile_definitions(ZCBOR_CANONICAL)
endif()

if (PRINT_CBOR)
  zephyr_compile_definitions(ZCBOR_PRINT_CBOR)
endif()

if (PRINT_CBOR_PRETTY)
  zephyr_compile_definitions(ZCBOR_PRINT_CBOR_PRETTY)
endif()

if (CONFIG_BIG_ENDIAN OR BIG_ENDIAN)
  zephyr_compile_definitions(ZCBOR_BIG_ENDIAN)
endif()

if (MAP_SMART_SEARCH)
  zephyr_compile_definitions(ZCBOR_MAP_SMART_SEARCH)
endif()

zephyr_compile_options(-Werror)

if (CONFIG_64BIT)
  set(bit_arg -b 64)
endif()
