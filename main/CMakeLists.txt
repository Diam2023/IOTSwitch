set(COMPONENT_REQUIRES)
set(COMPONENT_PRIV_REQUIRES)

file(GLOB_RECURSE COMPONENT_SRCS "./*.c*" "./src/*.c*")

set(COMPONENT_ADD_INCLUDEDIRS
        "./inc"
        "./src"
)

set(CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../config)

register_component()

if (EXISTS ${CONFIG_DIR})
    spiffs_create_partition_image(config ${CONFIG_DIR} FLASH_IN_PROJECT)
else ()
    message(FATAL_ERROR "${CONFIG_DIR} config file doesn't exit.")
endif ()
