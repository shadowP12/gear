#pragma once

#include "image.h"
#include <string>

namespace ImageUtilities {

Image* load_image(const std::string& file);

} // namespace ImageUtilities