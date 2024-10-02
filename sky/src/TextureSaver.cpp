#include "TextureSaver.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "vendor/stb_image/stb_image_write.h"

bool TextureSaver::SaveTextureToFile(const std::vector<float>& data, const std::string& path, int width, int depth, int comp)
{
    char* buffer = new char[data.size()];
    for(int i = 0; i < data.size(); i++)
    {
        buffer[i] = char(data[i] * 255.f);
    }

    stbi_write_bmp(path.c_str(), width, depth, comp, buffer);
    delete buffer;
    return false;
}
