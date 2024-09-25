# set source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)
# set paths
set(MOSQUITTO_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/mosquitto/include)
set(MOSQUITTO_LIB_DIR ${CMAKE_SOURCE_DIR}/3rdparty/mosquitto/lib)

# make exe
add_executable(mosquittoclient ${SRC_FILES})

# linking
target_include_directories(mosquittoclient PRIVATE ${MOSQUITTO_INCLUDE_DIR})
target_link_directories(mosquittoclient PRIVATE ${MOSQUITTO_LIB_DIR})
target_link_libraries(mosquittoclient PRIVATE mosquitto)