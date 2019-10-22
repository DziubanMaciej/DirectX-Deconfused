#pragma once

#ifdef DXD_COMPILE
// Compiling DXD library from source
#define EXPORT __declspec(dllexport)

#elif defined DXD_STATIC_LINK
// Linking to static DXD library
#define EXPORT

#else
// Linking to dynamic DXD library, the default
#define EXPORT __declspec(dllimport)

#endif
