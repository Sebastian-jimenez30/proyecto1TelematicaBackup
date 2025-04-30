#ifndef TOPIC_HPP
#define TOPIC_HPP

#include <string>
#include <vector>
#include "user.hpp"
#include "message.hpp"
#include "persistence.hpp"

class Topico {
public:
    // Constructor para creación desde cliente
    Topico(const std::string& nombre, Usuario& creador, Persistencia& persistencia, bool registrarEvento = true);

    // Constructor para creación interna (replicación)
    Topico(const std::string& nombre, const std::string& creadorUsername, Persistencia& persistencia);

    const std::string& getNombre() const;
    const std::string& getCreadorUsername() const;

    // Para clientes
    bool publicarDesdeCliente(const Mensaje& mensaje);
    bool suscribirDesdeCliente(Usuario& usuario);
    bool eliminarDesdeCliente(const Usuario& usuario);

    // Para replicación
    bool publicarDesdeReplica(const Mensaje& mensaje);
    bool suscribirDesdeReplica(Usuario& usuario);
    bool eliminarDesdeReplica();

    std::vector<Mensaje> obtenerMensajesPara(Usuario& usuario, int maxMensajes = 10);
    bool puedeEliminar(const Usuario& usuario) const;

private:
    std::string nombre;
    std::string creadorUsername;
    Persistencia& db;
};

#endif
