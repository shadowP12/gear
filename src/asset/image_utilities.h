#pragma once

#include "image.h"
#include <string>

namespace ImageUtilities {

Image* load_image(const std::string& file);

EzTexture create_texture(Image* image);

} // namespace ImageUtilities