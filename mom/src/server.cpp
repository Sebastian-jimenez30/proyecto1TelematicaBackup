#include "broker.hpp"
#include <iostream>
#include <limits>

void mostrarMenu() {
    std::cout << "\n=========== MENÚ SERVER MOM ===========\n";
    std::cout << "1. Registrar usuario\n";
    std::cout << "2. Iniciar sesión\n";
    std::cout << "3. Crear cola\n";
    std::cout << "4. Eliminar cola\n";
    std::cout << "5. Autorizar usuario para cola\n";
    std::cout << "6. Enviar mensaje a cola\n";
    std::cout << "7. Consumir mensaje de cola\n";
    std::cout << "8. Crear tópico\n";
    std::cout << "9. Eliminar tópico\n";
    std::cout << "10. Suscribirse a tópico\n";
    std::cout << "11. Publicar en tópico\n";
    std::cout << "12. Consumir desde tópico\n";
    std::cout << "0. Salir\n";
    std::cout << "========================================\n";
    std::cout << "Seleccione una opción: ";
}

int main() {
    Broker broker("../data/mom.db");
    std::string token;
    bool sesionActiva = false;
    std::string usuarioActual;

    int opcion;
    do {
        mostrarMenu();
        std::cin >> opcion;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (opcion) {
            case 1: {
                std::string user, pass;
                std::cout << "Nuevo usuario: ";
                std::getline(std::cin, user);
                std::cout << "Contraseña: ";
                std::getline(std::cin, pass);
                if (broker.registrarUsuario(user, pass))
                    std::cout << "Usuario registrado.\n";
                else
                    std::cout << "Error al registrar.\n";
                break;
            }
            case 2: {
                std::string user, pass;
                std::cout << "Usuario: ";
                std::getline(std::cin, user);
                std::cout << "Contraseña: ";
                std::getline(std::cin, pass);
                if (broker.autenticarUsuario(user, pass, token)) {
                    sesionActiva = true;
                    usuarioActual = user;
                    std::cout << "Sesión iniciada. Token generado.\n";
                } else {
                    std::cout << "Credenciales incorrectas.\n";
                }
                break;
            }
            case 3: {
                std::string nombre;
                std::cout << "Nombre de la cola: ";
                std::getline(std::cin, nombre);
                std::cout << (broker.crearCola(nombre, token) ? "Cola creada.\n" : "Error al crear cola.\n");
                break;
            }
            case 4: {
                std::string nombre;
                std::cout << "Nombre de la cola: ";
                std::getline(std::cin, nombre);
                std::cout << (broker.eliminarCola(nombre, token) ? "Cola eliminada.\n" : "No se pudo eliminar.\n");
                break;
            }
            case 5: {
                std::string nombre, objetivo;
                std::cout << "Nombre de la cola: ";
                std::getline(std::cin, nombre);
                std::cout << "Usuario a autorizar: ";
                std::getline(std::cin, objetivo);
                std::cout << (broker.autorizarCola(nombre, objetivo, token) ? "Usuario autorizado.\n" : "Falló la autorización.\n");
                break;
            }
            case 6: {
                std::string cola, contenido;
                std::cout << "Cola destino: ";
                std::getline(std::cin, cola);
                std::cout << "Contenido del mensaje: ";
                std::getline(std::cin, contenido);
                std::cout << (broker.enviarMensajeACola(cola, contenido, token) ? "Mensaje enviado.\n" : "Falló el envío.\n");
                break;
            }
            case 7: {
                std::string cola;
                std::cout << "Cola: ";
                std::getline(std::cin, cola);
                auto msg = broker.consumirMensajeDeCola(cola, token);
                if (msg.has_value()) std::cout << msg->toString() << "\n";
                else std::cout << "No se pudo consumir.\n";
                break;
            }
            case 8: {
                std::string topico;
                std::cout << "Nombre del tópico: ";
                std::getline(std::cin, topico);
                std::cout << (broker.crearTopico(topico, token) ? "Tópico creado.\n" : "Error al crear.\n");
                break;
            }
            case 9: {
                std::string topico;
                std::cout << "Nombre del tópico: ";
                std::getline(std::cin, topico);
                std::cout << (broker.eliminarTopico(topico, token) ? "Tópico eliminado.\n" : "No se pudo eliminar.\n");
                break;
            }
            case 10: {
                std::string topico;
                std::cout << "Tópico: ";
                std::getline(std::cin, topico);
                std::cout << (broker.suscribirATopico(topico, token) ? "Suscripción exitosa.\n" : "Falló suscripción.\n");
                break;
            }
            case 11: {
                std::string topico, contenido;
                std::cout << "Tópico destino: ";
                std::getline(std::cin, topico);
                std::cout << "Contenido del mensaje: ";
                std::getline(std::cin, contenido);
                std::cout << (broker.publicarEnTopico(topico, contenido, token) ? "Publicado.\n" : "Falló publicación.\n");
                break;
            }
            case 12: {
                std::string topico;
                std::cout << "Tópico: ";
                std::getline(std::cin, topico);
                auto mensajes = broker.consumirDesdeTopico(topico, token);
                if (mensajes.empty()) std::cout << "No hay nuevos mensajes.\n";
                else for (const auto& msg : mensajes) std::cout << msg.toString() << "\n";
                break;
            }
            case 0:
                std::cout << "Hasta luego.\n";
                break;
            default:
                std::cout << "Opción inválida.\n";
        }

    } while (opcion != 0);

    return 0;
}
