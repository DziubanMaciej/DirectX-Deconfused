#include "WindowClassFactory.h"

int WindowClassFactory::windowClassIndex = 0;

std::wstring WindowClassFactory::createWindowClassName() {
    return L"DXD_Window_Class_" + std::to_wstring(windowClassIndex++);
}
