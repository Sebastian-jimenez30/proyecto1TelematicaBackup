#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <string>
#include <queue>
#include "message.hpp"
#include "persistence.hpp"
#include "user.hpp"

class Cola {
public:
    // Constructor para creación desde cliente
    Cola(const std::string& nombre, Usuario& creador, Persistencia& persistencia, bool registrarEvento = true);

    // Constructor para creación interna (replicación)
    Cola(const std::string& nombre, const std::string& creadorUsername, Persistencia& persistencia);

    const std::string& getNombre() const;
    const std::string& getCreadorUsername() const;

    // Para clientes
    bool encolarDesdeCliente(const Mensaje& mensaje);
    bool eliminarDesdeCliente(const Usuario& usuario);
    bool autorizarUsuarioDesdeCliente(const std::string& usernameAutorizado);

    // Para replicación
    bool encolarDesdeReplica(const Mensaje& mensaje);
    bool eliminarDesdeReplica();
    bool autorizarUsuarioDesdeReplica(const std::string& usernameAutorizado);

    bool hayMensajes() const;
    Mensaje desencolar();

    bool usuarioPuedeConsumir(const Usuario& usuario) const;

private:
    std::string nombre;
    std::string creadorUsername;
    std::queue<Mensaje> mensajes;
    Persistencia& db;

    void cargarMensajesDesdeBD();
};

#endif
