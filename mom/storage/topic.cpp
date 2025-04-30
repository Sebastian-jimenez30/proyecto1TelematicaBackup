#include "topic.hpp"
#include <iostream>

Topico::Topico(const std::string& nombre, Usuario& creador, Persistencia& persistencia, bool registrarEvento)
    : nombre(nombre), creadorUsername(creador.getUsername()), db(persistencia) {

    if (registrarEvento) {
        if (!db.crearTopico(nombre, creadorUsername)) {
            std::cerr << "El tópico '" << nombre << "' ya existe o hubo un error.\n";
        }
        suscribirDesdeCliente(creador);
    }
}

Topico::Topico(const std::string& nombre, const std::string& creadorUsername, Persistencia& persistencia)
    : nombre(nombre), creadorUsername(creadorUsername), db(persistencia) {
    // Solo inicializa estructura en memoria
}

const std::string& Topico::getNombre() const {
    return nombre;
}

const std::string& Topico::getCreadorUsername() const {
    return creadorUsername;
}

bool Topico::publicarDesdeCliente(const Mensaje& mensaje) {
    return db.guardarMensaje(mensaje);
}

bool Topico::publicarDesdeReplica(const Mensaje& mensaje) {
    return db.guardarMensaje(mensaje);  // Igual que cliente, pero sin registrar evento nuevo
}

bool Topico::suscribirDesdeCliente(Usuario& usuario) {
    return db.suscribirUsuarioATopico(usuario.getUsername(), nombre);
}

bool Topico::suscribirDesdeReplica(Usuario& usuario) {
    return db.suscribirUsuarioATopico(usuario.getUsername(), nombre);
}

bool Topico::eliminarDesdeCliente(const Usuario& usuario) {
    if (!puedeEliminar(usuario)) return false;
    return db.eliminarTopico(nombre, usuario.getUsername());
}

bool Topico::eliminarDesdeReplica() {
    return db.eliminarTopico(nombre, creadorUsername);
}

std::vector<Mensaje> Topico::obtenerMensajesPara(Usuario& usuario, int maxMensajes) {
    int offset = db.obtenerOffset(usuario.getUsername(), nombre);
    std::vector<Mensaje> mensajes = db.cargarMensajesDesdeOffset(nombre, offset, maxMensajes);

    if (!mensajes.empty()) {
        db.actualizarOffset(usuario.getUsername(), nombre, offset + mensajes.size());
    }

    return mensajes;
}

bool Topico::puedeEliminar(const Usuario& usuario) const {
    return usuario.getUsername() == creadorUsername;
}
