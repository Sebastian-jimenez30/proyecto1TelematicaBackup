#include "NodoManager.hpp"
#include <fstream>
#include <iostream>

NodoManager::NodoManager(const std::string& miDireccion, const std::string& archivoNodos) {
    std::ifstream infile(archivoNodos);
    std::string linea;
    while (std::getline(infile, linea)) {
        if (!linea.empty() && linea != miDireccion) {
            std::cout << "[NodoManager] Creando stub para: " << linea << std::endl;
            auto canal = grpc::CreateChannel(linea, grpc::InsecureChannelCredentials());
            stubs[linea] = mom::MomService::NewStub(canal);
        }
    }
}

std::vector<std::string> NodoManager::obtenerOtrosNodos(const std::string& direccionPropia) {
    std::vector<std::string> otros;
    for (const auto& [dir, _] : stubs) {
        if (dir != direccionPropia) {
            otros.push_back(dir);
        }
    }
    return otros;
}

mom::MomService::Stub* NodoManager::obtenerStub(const std::string& direccion) {
    auto it = stubs.find(direccion);
    if (it != stubs.end()) {
        return it->second.get(); 
    }
    return nullptr;
}
