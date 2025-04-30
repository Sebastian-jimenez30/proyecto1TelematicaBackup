#!/bin/bash

# Actualizar la lista de paquetes
echo "Actualizando la lista de paquetes..."
sudo apt update

# Instalar las dependencias necesarias para el proyecto en C++
echo "Instalando dependencias de C++..."
sudo apt install -y \
    build-essential \
    libgrpc++-dev \
    libprotobuf-dev \
    libssl-dev \
    libsqlite3-dev \
    libboost-all-dev \
    libabsl-dev \

# Verificar que las dependencias se hayan instalado correctamente
echo "Verificando instalación de dependencias..."

dpkg -s libgrpc++-dev libprotobuf-dev libssl-dev libsqlite3-dev libboost-all-dev

echo "Instalación completada."
