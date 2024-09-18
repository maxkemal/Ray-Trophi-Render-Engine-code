// Texture.cpp

#include "Texture.h"
#include <iostream>

Texture::Texture(const std::string& filename) {
    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (!surface) {
        std::cerr << "Error loading image: " << filename << ", SDL Error: " << IMG_GetError() << std::endl;
        return;
    }

    width = surface->w;
    height = surface->h;
    pixels.resize(width * height);

    SDL_LockSurface(surface);
    Uint8* pixelData = static_cast<Uint8*>(surface->pixels);
    SDL_PixelFormat* format = surface->format;
    // Eðer format 32-bit deðilse, alfa kanalý olmayabilir
    bool has_alpha = (surface->format->BitsPerPixel == 32);
   
    if (has_alpha) {
        alphas.resize(width * height);
    }
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Uint8 r, g, b, a;
            Uint32 pixel;
            alphas.resize(width * height);
            switch (format->BitsPerPixel) {
            case 8: // 8-bit paletteli format
            {

                Uint8* p = pixelData + (y * surface->pitch) + x;
                pixel = *p;               
               
                // Siyah-beyaz olup olmadýðýný kontrol et
                      // 8-bit paletteli format genellikle gri tonlama olarak kullanýlýr
                if (r != g || r != b) {
                    is_gray_scale = false;
                }

                // Gri tonlamalý piksellerin alfa deðerini oluþtur
                float gray = *p / 255.0f;
                // Opacity map ise gri tonlamayý alfa olarak kullan
                if (is_gray_scale) {
                    alphas[y * width + x] = (gray == 0) ? 0.0f : 1.0f; // Siyah 0, Beyaz 1
                }
                else {
                    pixels[y * width + x] = Vec3(gray / 255.0f, gray / 255.0f, gray / 255.0f);
                }
                SDL_GetRGBA(pixel, format, &r, &g, &b, &a);
            }
            break;
            case 15: // 15-bit format (5-5-5)
            case 16: // 16-bit format (5-6-5 veya 5-5-5-1)
            {
                Uint16* p = reinterpret_cast<Uint16*>(pixelData + (y * surface->pitch));
                pixel = p[x];
                SDL_GetRGBA(pixel, format, &r, &g, &b, &a);
            }
            break;
            case 24: // 24-bit format
            {
                Uint8* p = pixelData + (y * surface->pitch) + (x * 3);
                if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                    pixel = (p[0] << 16) | (p[1] << 8) | p[2];
                }
                else {
                    pixel = p[0] | (p[1] << 8) | (p[2] << 16);
                }
                SDL_GetRGB(pixel, format, &r, &g, &b);
                a = 255; // 24-bit formatlar genellikle alfa kanalý içermez
            }
            break;
            case 32: // 32-bit format
            {
                Uint32* p = reinterpret_cast<Uint32*>(pixelData + (y * surface->pitch));
                pixel = p[x];
                SDL_GetRGBA(pixel, format, &r, &g, &b, &a);
            }
            break;
            default:
                std::cerr << "Unsupported pixel format: " << static_cast<int>(format->BitsPerPixel) << " bits per pixel." << std::endl;
                SDL_UnlockSurface(surface);
                SDL_FreeSurface(surface);
                return; // Desteklenmeyen format durumunda fonksiyondan çýk
            }

            // Debug output to check pixel values
            // std::cout << "Pixel (" << x << ", " << y << "): R=" << static_cast<int>(r) << ", G=" << static_cast<int>(g) << ", B=" << static_cast<int>(b) << ", A=" << static_cast<int>(a) << std::endl;

            float normalized_r = r / 255.0f;
            float normalized_g = g / 255.0f;
            float normalized_b = b / 255.0f;
            float normalized_a = a / 255.0f;
            if (has_alpha) {
                alphas[y * width + x] = normalized_a;  // Sadece alfa kanalý varsa yazma iþlemi yap
            }
            // Alfa deðerini de kullanmak isterseniz:
            // pixels[y * width + x] = Vec4(normalized_r, normalized_g, normalized_b, normalized_a);
            // Eðer Vec3 kullanmaya devam etmek istiyorsanýz:
            pixels[y * width + x] = Vec3(normalized_r, normalized_g, normalized_b);
        }
    }


    SDL_UnlockSurface(surface);
    SDL_FreeSurface(surface);
}

float Texture::get_alpha(double u, double v) const {
    if (width <= 0 || height <= 0 || alphas.empty()) {
        return 1.0f; // Geçerli bir opaklýk haritasý yoksa varsayýlan opaklýk
    }

    u = std::clamp(u, 0.0, 1.0);
    v = std::clamp(v, 0.0, 1.0);

    int x = static_cast<int>(u * (width - 1));
    int y = static_cast<int>((1 - v) * (height - 1));

    x = std::clamp(x, 0, width - 1);
    y = std::clamp(y, 0, height - 1);

    // Eðer gri tonlamalýysa, doðrudan alfa kanalýný kullan
    if (is_gray_scale) {
        return alphas[y * width + x];
    }

    return alphas[y * width + x]; // Normal opaklýk haritasý
}


void Texture::loadOpacityMap(const std::string& filename) {
    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (!surface) {
        std::cerr << "Error loading opacity map: " << filename << ", SDL Error: " << IMG_GetError() << std::endl;
        return;
    }

    // Opacity map'in boyutlarýný kontrol et ve gerekirse alphas vektörünü yeniden boyutlandýr
    if (surface->w != width || surface->h != height) {
        std::cout << "Opacity map dimensions do not match the main texture. Resizing alpha channel." << std::endl;
        width = surface->w;
        height = surface->h;
        alphas.resize(width * height, 1.0f);  // Yeni boyuta göre alphas'ý yeniden boyutlandýr ve 1.0f ile baþlat
    }

    SDL_LockSurface(surface);
    Uint8* pixelData = static_cast<Uint8*>(surface->pixels);
    SDL_PixelFormat* format = surface->format;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Uint8 gray;
            if (format->BitsPerPixel == 8) {
                // 8-bit gri tonlamalý resim
                gray = pixelData[y * surface->pitch + x];
            }
            else {
                // Diðer formatlar için gri deðeri hesapla
                Uint8 r, g, b, a;
                Uint32 pixel = *reinterpret_cast<Uint32*>(pixelData + y * surface->pitch + x * format->BytesPerPixel);
                SDL_GetRGBA(pixel, format, &r, &g, &b, &a);
                gray = static_cast<Uint8>(0.299 * r + 0.587 * g + 0.114 * b);
            }

            size_t index = y * width + x;
            if (index < alphas.size()) {
                alphas[index] = gray / 255.0f;
            }
            else {
                std::cerr << "Error: Attempting to access out of bounds index in alphas vector." << std::endl;
                SDL_UnlockSurface(surface);
                SDL_FreeSurface(surface);
                return;
            }
        }
    }

    SDL_UnlockSurface(surface);
    SDL_FreeSurface(surface);

    std::cout << "Opacity map loaded successfully. Dimensions: " << width << "x" << height << std::endl;
}
Vec3 Texture::get_color(double u, double v) const {
    // Sýnýr kontrolü
    if (width <= 0 || height <= 0 || pixels.empty()) {
        return Vec3(0, 0, 0);  // Geçersiz texture durumunda siyah döndür
    }

    u = std::clamp(u, 0.0, 1.0);
    v = std::clamp(v, 0.0, 1.0);

    int x = static_cast<int>(u * (width - 1));
    int y = static_cast<int>((1 - v) * (height - 1));  // Flip y-coordinate if necessary

    // Sýnýrlarý aþmadýðýndan emin ol
    x = std::clamp(x, 0, width - 1);
    y = std::clamp(y, 0, height - 1);

    // Bilineer interpolasyon için komþu pikselleri hesapla
    int x0 = x;
    int x1 = std::min(x + 1, width - 1);
    int y0 = y;
    int y1 = std::min(y + 1, height - 1);

    double tx = u * (width - 1) - x;
    double ty = (1 - v) * (height - 1) - y;

    // Güvenli indeks eriþimi için helper fonksiyon
    auto safe_pixel = [this](int y, int x) -> Vec3 {
        size_t index = static_cast<size_t>(y) * width + x;
        if (index < pixels.size()) {
            return pixels[index];
        }
        return Vec3(0, 0, 0);  // Geçersiz indeks durumunda siyah döndür
        };

    Vec3 c00 = safe_pixel(y0, x0);
    Vec3 c10 = safe_pixel(y0, x1);
    Vec3 c01 = safe_pixel(y1, x0);
    Vec3 c11 = safe_pixel(y1, x1);

    // Bilineer interpolasyon
    Vec3 c0 = c00 * (1 - tx) + c10 * tx;
    Vec3 c1 = c01 * (1 - tx) + c11 * tx;
    return c0 * (1 - ty) + c1 * ty;
}

Texture::~Texture() {
    // SDL_image'in kullandýðý kaynaklarý temizle
    IMG_Quit();
}