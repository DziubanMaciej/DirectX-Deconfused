#pragma once

#include "ExternalHeaders/stb_image.h"

struct StbImage {
    StbImage(const std::string &filePath, int reqComp) {
        data = stbi_load(filePath.c_str(), &width, &height, &channels, reqComp);
    }
    ~StbImage() {
        stbi_image_free(data);
    }

    int width;
    int height;
    int channels;
    unsigned char *data;
};
