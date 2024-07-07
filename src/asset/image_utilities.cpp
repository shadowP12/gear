#include "image_utilities.h"
#include <core/path.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBIR_FLAG_ALPHA_PREMULTIPLIED
#include <stb_image.h>

namespace ImageUtilities {

Image* load_image(const std::string& file)
{
    if (Path::extension(file) == "png")
    {
        int width, height, channels;
        unsigned char* pixels = stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);

        Image* image = new Image;
        image->width = width;
        image->height = height;
        image->data.resize(width * height * 4);
        memcpy(image->data.data(), pixels, width * height * 4);
        image->format = VK_FORMAT_R8G8B8A8_UNORM;
        stbi_image_free(pixels);

        return image;
    }
    return nullptr;
}

} // namespace ImageUtilities