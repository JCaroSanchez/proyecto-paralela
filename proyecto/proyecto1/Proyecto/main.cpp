#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

// Estructura para almacenar un píxel RGB
struct RGBPixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

// Función para cargar una imagen JPEG y obtener los píxeles
std::vector<RGBPixel> loadJPEG(const std::string& filename, int& width, int& height) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cout << "No se puede abrir el archivo de imagen.\n";
        width = 0;
        height = 0;
        return {};
    }

    // Leer la cabecera del archivo JPEG (simplificado)
    char header[4];
    file.read(header, 4);
    if (header[0] != 0xFF || header[1] != 0xD8 || header[2] != 0xFF || header[3] != 0xE0) {
        std::cout << "Formato de archivo JPEG inválido.\n";
        width = 0;
        height = 0;
        return {};
    }

    // Saltar la información de la cabecera
    file.seekg(2, std::ios::cur);
    while (true) {
        char marker[2];
        file.read(marker, 2);
        unsigned short segmentSize = (marker[0] << 8) + marker[1];

        if (marker[0] == 0xFF) {
            if (marker[1] == 0xC0 || marker[1] == 0xC1 || marker[1] == 0xC2) {
                // Encontrada la información de tamaño de la imagen
                file.seekg(1, std::ios::cur);
                file.read(marker, 4);
                height = (marker[0] << 8) + marker[1];
                width = (marker[2] << 8) + marker[3];
                break;
            } else {
                // Saltar segmentos no deseados
                file.seekg(segmentSize - 2, std::ios::cur);
            }
        } else {
            // Saltar segmentos no deseados
            file.seekg(segmentSize - 2, std::ios::cur);
        }
    }

    // Leer los píxeles de la imagen
    std::vector<RGBPixel> pixels(width * height);
    file.read(reinterpret_cast<char*>(pixels.data()), width * height * sizeof(RGBPixel));

    return pixels;
}

// Función para convertir una imagen a escala de grises
std::vector<unsigned char> convertToGrayscale(const std::vector<RGBPixel>& pixels) {
    std::vector<unsigned char> grayscalePixels(pixels.size());
    for (size_t i = 0; i < pixels.size(); i++) {
        unsigned char r = pixels[i].r;
        unsigned char g = pixels[i].g;
        unsigned char b = pixels[i].b;
        unsigned char grayscale = static_cast<unsigned char>((r + g + b) / 3);
        grayscalePixels[i] = grayscale;
    }
    return grayscalePixels;
}

// Función para guardar una imagen en formato JPEG
void saveJPEG(const std::string& filename, const std::vector<unsigned char>& pixels, int width, int height) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cout << "No se puede crear el archivo de salida.\n";
        return;
    }

    // Escribir la cabecera del archivo JPEG (simplificado)
    file << char(0xFF) << char(0xD8);
    file << char(0xFF) << char(0xE0);
    file << char(0x00) << char(0x10);
    file << "JFIF\x00";
    file << char(0x01) << char(0x01);
    file << char(0x00) << char(0x00);
    file << char(0x01) << char(0x00);
    file << char(0x01) << char(0x00);
    file << char(0x00) << char(0xFF) << char(0xDB);
    file << char(0x00) << char(0x43);
    file << char(0x00);

    // Escribir el tamaño de la imagen
    file << char(0xFF) << char(0xC0);
    file << char(0x00) << char(0x0B);
    file << char((height >> 8) & 0xFF) << char(height & 0xFF);
    file << char((width >> 8) & 0xFF) << char(width & 0xFF);
    file << char(0x01) << char(0x11);
    file << char(0x00);

    // Escribir los píxeles de la imagen
    file << char(0xFF) << char(0xC2);
    file << char(0x00) << char(0x0A);
    file << char(0x08);
    file << char((height >> 8) & 0xFF) << char(height & 0xFF);
    file << char((width >> 8) & 0xFF) << char(width & 0xFF);
    file << char(0x03);
    file << char(0x01);
    file << char(0x11);
    file << char(0x00) << char(0x3F) << char(0x00);

    // Escribir los datos de píxeles en formato JPEG
    file << char(0xFF) << char(0xDA);
    file << char(0x00) << char(0x08);
    file << char(0x01) << char(0x01);
    file << char(0x00) << char(0x00);

    // Escribir los píxeles en el archivo
    for (unsigned char pixel : pixels) {
        file << pixel;
    }

    // Escribir la marca de finalización
    file << char(0xFF) << char(0xD9);
}

int main() {
    // Cargar la imagen JPEG
    std::string filename = "/home/caro/proyecto1/lena_color.jpeg";
    int width, height;

    auto startLoad = std::chrono::high_resolution_clock::now(); // Medir el tiempo de carga
    std::vector<RGBPixel> pixels = loadJPEG(filename, width, height);
    auto endLoad = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> loadTime = endLoad - startLoad;

    // Convertir la imagen a escala de grises
    auto startConvert = std::chrono::high_resolution_clock::now(); // Medir el tiempo de conversión
    std::vector<unsigned char> grayscalePixels = convertToGrayscale(pixels);
    auto endConvert = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> convertTime = endConvert - startConvert;

    // Guardar la imagen en escala de grises en formato JPEG
    const std::string grayscaleImageFile = "/home/caro/proyecto1/lena_grayscale.jpeg";

    auto startSave = std::chrono::high_resolution_clock::now(); // Medir el tiempo de guardado
    saveJPEG(grayscaleImageFile, grayscalePixels, width, height);
    auto endSave = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> saveTime = endSave - startSave;

    // Imprimir los tiempos de ejecución
    std::cout << "Tiempo de carga: " << loadTime.count() << " segundos\n";
    std::cout << "Tiempo de conversión: " << convertTime.count() << " segundos\n";
    std::cout << "Tiempo de guardado: " << saveTime.count() << " segundos\n";

    return 0;
}
