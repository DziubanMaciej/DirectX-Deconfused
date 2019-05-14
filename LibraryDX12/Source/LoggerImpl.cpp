#include "DXD/Logger.h"

std::mutex DXD::LoggerData::mutex{};
char DXD::LoggerData::buffer[] = {};
