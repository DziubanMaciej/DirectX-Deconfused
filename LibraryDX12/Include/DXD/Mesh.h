#pragma once

#include "DXD/Export.h"
#include <memory>
#include <string>

namespace DXD {
class EXPORT Mesh {
public:
    virtual int loadFromObj(const std::string filePath) = 0;

    virtual ~Mesh() = default;
    static std::unique_ptr<Mesh> create();

protected:
    Mesh() = default;
};
} // namespace DXD
