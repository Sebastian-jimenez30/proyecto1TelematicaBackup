#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <chrono>

class Mensaje {
public:
    // Temporal
    Mensaje(const std::string& remitente,
            const std::string& contenido,
            const std::string& canal,
            const std::string& tipo,
            std::chrono::system_clock::time_point timestamp);

    // Mensaje identificado
    Mensaje(int id,
            const std::string& remitente,
            const std::string& contenido,
            const std::string& canal,
            const std::string& tipo,
            std::chrono::system_clock::time_point timestamp);

    int getId() const;
    std::string getRemitente() const;
    std::string getContenido() const;
    std::string getCanal() const;
    std::string getTipo() const;
    std::chrono::system_clock::time_point getTimestamp() const;

    std::string toString() const;

private:
    int id;
    std::string remitente;
    std::string contenido;
    std::string canal;
    std::string tipo;
    std::chrono::system_clock::time_point timestamp;
};

#endif
