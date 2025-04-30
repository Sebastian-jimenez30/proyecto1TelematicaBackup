#ifndef NODOMANAGER_HPP
#define NODOMANAGER_HPP

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <grpcpp/grpcpp.h>
#include "mom.grpc.pb.h"

class NodoManager {
public:
    NodoManager(const std::string& miDireccion, const std::string& archivoNodos);

    std::vector<std::string> obtenerOtrosNodos(const std::string& direccionPropia);
    mom::MomService::Stub* obtenerStub(const std::string& direccion);

private:
    std::unordered_map<std::string, std::unique_ptr<mom::MomService::Stub>> stubs;
};

#endif
