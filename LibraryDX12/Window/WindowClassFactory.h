#pragma once

#include <DXD/Utility/NonCopyableAndMovable.h>
#include <string>

struct WindowClassFactory : DXD::NonInstantiatable {
    static int windowClassIndex;
    static std::wstring createWindowClassName();
};
