cmake_minimum_required(VERSION 3.10)
project(MOMCore)

set(CMAKE_CXX_STANDARD 17)

# -------------------- DEPENDENCIAS --------------------
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# -------------------- VERIFICAR PLUGIN DE gRPC --------------------
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin PATHS /usr/local/bin /usr/bin)
if (NOT GRPC_CPP_PLUGIN)
    message(FATAL_ERROR "gRPC plugin not found. Please ensure grpc_cpp_plugin is installed.")
endif()

# -------------------- INCLUDES --------------------
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/jwt-cpp
    ${PROJECT_SOURCE_DIR}/include/picojson
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/storage
    ${Protobuf_INCLUDE_DIRS}
    ${gRPC_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${SQLite3_INCLUDE_DIRS}
)

# -------------------- GENERAR CÓDIGO gRPC Y PROTOBUF --------------------
set(PROTO_FILE ${PROJECT_SOURCE_DIR}/proto/mom.proto)
set(GENERATED_SRC_DIR ${PROJECT_SOURCE_DIR}/include)

add_custom_command(
  OUTPUT
    ${GENERATED_SRC_DIR}/mom.pb.cc
    ${GENERATED_SRC_DIR}/mom.pb.h
    ${GENERATED_SRC_DIR}/mom.grpc.pb.cc
    ${GENERATED_SRC_DIR}/mom.grpc.pb.h
  COMMAND protoc
    --proto_path=${PROJECT_SOURCE_DIR}/proto
    --cpp_out=${GENERATED_SRC_DIR}
    --grpc_out=${GENERATED_SRC_DIR}
    --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
    ${PROTO_FILE}
  DEPENDS ${PROTO_FILE}
)

add_custom_target(proto_files_target DEPENDS
  ${GENERATED_SRC_DIR}/mom.pb.cc
  ${GENERATED_SRC_DIR}/mom.pb.h
  ${GENERATED_SRC_DIR}/mom.grpc.pb.cc
  ${GENERATED_SRC_DIR}/mom.grpc.pb.h
)

# -------------------- ARCHIVOS FUENTE --------------------
add_executable(mom_server
  src/mom_server.cpp
  src/broker.cpp
  src/mom_server.cpp
  src/NodoManager.cpp
  storage/persistence.cpp
  storage/message.cpp
  storage/user.cpp
  storage/queue.cpp
  storage/topic.cpp
  include/mom.pb.cc
  include/mom.grpc.pb.cc
)

add_dependencies(mom_server proto_files_target)

# -------------------- LINKING --------------------
target_link_libraries(mom_server
  gRPC::grpc++
  protobuf::libprotobuf
  OpenSSL::SSL
  OpenSSL::Crypto
  SQLite::SQLite3
  pthread
)
