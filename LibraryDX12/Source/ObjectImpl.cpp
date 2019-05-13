#include "ObjectImpl.h"
#include <fstream>

namespace DXD {
std::unique_ptr<Object> Object::create() {
    return std::unique_ptr<Object>{new ObjectImpl()};
}
} // namespace DXD

ObjectImpl::ObjectImpl() {
}

ObjectImpl::~ObjectImpl() {
}

void ObjectImpl::setPosition(FLOAT x, FLOAT y, FLOAT z) {
    position.x = x;
    position.y = y;
    position.z = z;
}
