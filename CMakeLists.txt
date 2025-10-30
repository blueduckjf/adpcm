cmake_minimum_required(VERSION 3.10)
project(adpcm LANGUAGES C)

set(ADPCM_BASE_PATH     ${CMAKE_CURRENT_LIST_DIR})
set(ADPCM_INC_PATH      "${ADPCM_BASE_PATH}/inc")
set(ADPCM_INC_INT_PATH  "${ADPCM_INC_PATH}/adpcm")
set(ADPCM_SRC_PATH      "${ADPCM_BASE_PATH}/src")

add_library(adpcm "${ADPCM_SRC_PATH}/adpcm.c")
target_include_directories(adpcm 
    PRIVATE     ${ADPCM_INC_INT_PATH}
    INTERFACE   ${ADPCM_INC_PATH}
)