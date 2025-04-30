#include "message.hpp"
#include <sstream>
#include <iomanip>

// Temporal
Mensaje::Mensaje(const std::string& remitente,
                 const std::string& contenido,
                 const std::string& canal,
                 const std::string& tipo,
                 std::chrono::system_clock::time_point timestamp)
    : id(-1), remitente(remitente), contenido(contenido), canal(canal), tipo(tipo), timestamp(timestamp) {}

// Mensaje Identificado
Mensaje::Mensaje(int id,
                 const std::string& remitente,
                 const std::string& contenido,
                 const std::string& canal,
                 const std::string& tipo,
                 std::chrono::system_clock::time_point timestamp)
    : id(id), remitente(remitente), contenido(contenido), canal(canal), tipo(tipo), timestamp(timestamp) {}

int Mensaje::getId() const {
    return id;
}

std::string Mensaje::getRemitente() const {
    return remitente;
}

std::string Mensaje::getContenido() const {
    return contenido;
}

std::string Mensaje::getCanal() const {
    return canal;
}

std::string Mensaje::getTipo() const {
    return tipo;
}

std::chrono::system_clock::time_point Mensaje::getTimestamp() const {
    return timestamp;
}

std::string Mensaje::toString() const {
    std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
    std::stringstream ss;
    ss << "[" << (id >= 0 ? std::to_string(id) : "nuevo") << "] "
       << "De: " << remitente << " | "
       << "Canal: " << canal << " | "
       << "Tipo: " << tipo << " | "
       << "Mensaje: " << contenido << " | "
       << "Fecha: " << std::put_time(std::localtime(&time), "%F %T");
    return ss.str();
}
