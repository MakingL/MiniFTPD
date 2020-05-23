set(INC_DIR include)
include_directories(${INC_DIR})

FILE(GLOB YAML-CPP-SOURCES "*.cpp")

add_library(yaml-cpp ${YAML-CPP-SOURCES})
