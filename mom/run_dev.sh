#!/bin/bash

# Variables
SRC_DIR="./src"
STORAGE_DIR="./storage"
INCLUDE_DIR="./include"
PROTO_DIR="./proto"
BUILD_DIR="./build"
OUTPUT_BINARY="mom_server"
PROTO_FILE="$PROTO_DIR/mom.proto"

# Crear directorio de build si no existe
mkdir -p $BUILD_DIR

if [ "$1" == "0" ]; then
    echo "Ejecutando el servidor..."
    if [ -f "$BUILD_DIR/$OUTPUT_BINARY" ]; then
        $BUILD_DIR/$OUTPUT_BINARY
    else
        echo "Error: El ejecutable del servidor no existe. Por favor, compila el proyecto primero."
    fi
    exit 0
elif [ "$1" == "1" ]; then
    echo "Generando archivos gRPC a partir de $PROTO_FILE..."
    protoc -I=$PROTO_DIR --grpc_out=$INCLUDE_DIR --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $PROTO_FILE
    protoc -I=$PROTO_DIR --cpp_out=$INCLUDE_DIR $PROTO_FILE

    echo "Compilando el servidor..."
    g++ -std=c++17 -I$INCLUDE_DIR -I$BUILD_DIR -I$INCLUDE_DIR/jwt-cpp -I$INCLUDE_DIR/picojson -L/usr/local/lib \
        $SRC_DIR/mom_server.cpp $SRC_DIR/broker.cpp $STORAGE_DIR/persistence.cpp $STORAGE_DIR/message.cpp \
        $STORAGE_DIR/user.cpp $STORAGE_DIR/queue.cpp $STORAGE_DIR/topic.cpp \
        $INCLUDE_DIR/mom.pb.cc $INCLUDE_DIR/mom.grpc.pb.cc \
        -o $BUILD_DIR/$OUTPUT_BINARY \
        -lgrpc++ -lgrpc -lprotobuf -lpthread -labsl_synchronization -labsl_base -labsl_time -labsl_strings -lgpr -lsqlite3 -lcrypto -lssl

    if [ $? -eq 0 ]; then
        echo "Compilación exitosa. Ejecutando el servidor..."
        $BUILD_DIR/$OUTPUT_BINARY
    else
        echo "Error en la compilación."
    fi
elif [ "$1" == "2" ]; then
    echo "Instalando los requerimientos..."
    ./requirements.sh

    echo "Generando archivos gRPC a partir de $PROTO_FILE..."
    protoc -I=$PROTO_DIR --grpc_out=$INCLUDE_DIR --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $PROTO_FILE
    protoc -I=$PROTO_DIR --cpp_out=$INCLUDE_DIR $PROTO_FILE

    echo "Compilando el servidor..."
    g++ -std=c++17 -I$INCLUDE_DIR -I$BUILD_DIR -I$INCLUDE_DIR/jwt-cpp -I$INCLUDE_DIR/picojson -L/usr/local/lib \
        $SRC_DIR/mom_server.cpp $SRC_DIR/broker.cpp $STORAGE_DIR/persistence.cpp $STORAGE_DIR/message.cpp \
        $STORAGE_DIR/user.cpp $STORAGE_DIR/queue.cpp $STORAGE_DIR/topic.cpp \
        $INCLUDE_DIR/mom.pb.cc $INCLUDE_DIR/mom.grpc.pb.cc \
        -o $BUILD_DIR/$OUTPUT_BINARY \
        -lgrpc++ -lgrpc -lprotobuf -lpthread -labsl_synchronization -labsl_base -labsl_time -labsl_strings -lgpr -lsqlite3 -lcrypto -lssl

    if [ $? -eq 0 ]; then
        echo "Compilación exitosa. Ejecutando el servidor..."
        $BUILD_DIR/$OUTPUT_BINARY
    else
        echo "Error en la compilación."
    fi
else
    echo "Uso: $0 [0|1|2]"
    echo "  0: Solo ejecutar el servidor."
    echo "  1: Generar archivos .proto, compilar y ejecutar el servidor."
    echo "  2: Instalar los requerimientos, generar archivos .proto, compilar y ejecutar el servidor."
    exit 1
fi
