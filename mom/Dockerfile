# Imagen base
FROM ubuntu:22.04

# Evitar prompts de configuración
ENV DEBIAN_FRONTEND=noninteractive

# Instalar herramientas esenciales
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    unzip \
    autoconf \
    libtool \
    pkg-config \
    libssl-dev \
    libsqlite3-dev \
    libprotobuf-dev \
    protobuf-compiler \
    ca-certificates

# Instalar gRPC y Protobuf manualmente (como lo hiciste tú)
WORKDIR /opt
RUN git clone --recurse-submodules -b v1.70.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc && \
    cd grpc && \
    mkdir -p cmake/build && cd cmake/build && \
    cmake -DgRPC_INSTALL=ON \
          -DgRPC_BUILD_TESTS=OFF \
          -DCMAKE_CXX_STANDARD=17 \
          -DCMAKE_INSTALL_PREFIX=/usr/local \
          ../.. && \
    make -j$(nproc) && make install

RUN git clone --recurse-submodules -b v21.12 --depth 1 https://github.com/protocolbuffers/protobuf.git && \
    cd protobuf && \
    mkdir -p cmake/build && cd cmake/build && \
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_CXX_STANDARD=17 \
          -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
          -DCMAKE_INSTALL_PREFIX=/usr/local \
          ../.. && \
    make -j$(nproc) && make install && ldconfig

# Crear directorio de trabajo para el proyecto
WORKDIR /app

# Copiar todo el código fuente
COPY . .

# Crear carpeta de datos (como volumen externo)
RUN mkdir -p /data

# Compilar proyecto
RUN mkdir -p /app/build && cd /app/build && cmake .. && make -j$(nproc)

# Exponer puerto gRPC
EXPOSE 50051

# Ejecutable por defecto
ENTRYPOINT ["./build/mom_server"]

# Valores por defecto para IP y carpeta de datos (pueden ser sobrescritos)
CMD ["./build/mom_server", "0.0.0.0", "/data", "50051"]

