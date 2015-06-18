/*
Copyright (C) 2015  Marien Raat

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <random>
#include "lodepng.h" // For png writing
#include <math.h>
#include <vector>
#include <iostream>

std::random_device randomD;
std::mt19937 mt(randomD());
std::uniform_real_distribution<double> dist(-1.0, 1.0);

struct PassSettings {
    double *map; // Map to run the pass on
    double displacementModifier; // Displacement from drand() is multiplied
                                 // by this value. Given by 2 ^ (- (h * pass))
    int passSize; // Length of the sides of the square
    int mapSize; // Size of each side of the map
};

void drawValues(double *values, unsigned width, unsigned height, const char *filename);
void drawTexture(double *values, unsigned width, unsigned height, const char *filename);
int *getTextureRgb(double value);
void normalizeValues(double *values, int size);
double* generateMap(
    int size, // Size of the square, should be a power of 2 plus 1
    double h // The displacement will be decreased by 2 ^ (- h) each pass
             // A high h-value will result in a smooth map and a low h-value
             // will result in a rough map
    ); // Returns a pointer to an array of the length size * size
void passDiamondSquare(PassSettings *settings);
void diamond(int x, int y, PassSettings *settings);
void square(int x, int y, PassSettings *settings);
bool isValid(int x, int y, int size);
double drand(); // Returns a double between -1 and 1
    
int main(int argc, char **argv) {
    const char *filename = argc > 1 ? argv[1] : "heightmap.png";
    int size = 1025;
    double *map = generateMap(size, 0.9);
    normalizeValues(map, size * size);
    drawValues(map, size, size, filename); 
    if (argc > 2 && argv[2][0] == 't')
        drawTexture(map, size, size, "atexture.png");
    return 0;
}

void drawValues(double *values, unsigned width, unsigned height, const char *filename) {
    std::vector<unsigned char> image;
    image.resize(width * height * 4);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            unsigned char value = values[y * width + x] * 255;
            image[4 * width * y + 4 * x + 0] = value;
            image[4 * width * y + 4 * x + 1] = value;
            image[4 * width * y + 4 * x + 2] = value;
            image[4 * width * y + 4 * x + 3] = 255;
        }
    }
    lodepng::encode(filename, image, width, height);
}

void drawTexture(double *values, unsigned width, unsigned height, const char *filename) {
    std::vector<unsigned char> image;
    image.resize(width * height * 4);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            int *rgb = getTextureRgb(values[width * y + x]);
            image[4 * width * y + 4 * x + 0] = rgb[0];
            image[4 * width * y + 4 * x + 1] = rgb[1];
            image[4 * width * y + 4 * x + 2] = rgb[2];
            image[4 * width * y + 4 * x + 3] = 255;
        }
    }
    lodepng::encode(filename, image, width, height);
}

int *getTextureRgb(double value) {
    int *rgb = new int[3];
    if (value < 0.25) {
        rgb[0] = 0;
        rgb[1] = 0;
        rgb[2] = 220;
    } else if (value < 0.3) {
        rgb[0] = 210;
        rgb[1] = 210;
        rgb[2] = 10;
    } else if (value < 0.4) {
        rgb[0] = 0;
        rgb[1] = 205;
        rgb[2] = 0;
    } else if (value < 0.6) {
        rgb[0] = 0;
        rgb[1] = 100;
        rgb[2] = 0;
    } else if (value < 0.8) {
        rgb[0] = 120;
        rgb[1] = 120;
        rgb[2] = 120;
    } else {
        rgb[0] = 255;
        rgb[1] = 255;
        rgb[2] = 255;
    }
    return rgb;
}

void normalizeValues(double *values, int size) {
    double max = -1.0 / 0.0, min = 1.0 / 0.0;
    for (int i = 0; i < size; i++) {
        if (values[i] > max)
            max = values[i];
        if (values[i] < min)
            min = values[i];
    }
    for (int i = 0; i < size; i++) {
        values[i] -= min;
        values[i] /= (max - min);
    }
}

double* generateMap(int size, double h) {
    double *map = new double[size * size];
    double initialValue = 0.0;
    for (int i = 0; i < size * size; i++) map[i] = initialValue;
    PassSettings settings;
    settings.map = map;
    settings.displacementModifier = 1.0;
    settings.passSize = size - 1;
    settings.mapSize = size;
    while (settings.passSize > 1) {
        std::cout << settings.passSize << "\n";
        passDiamondSquare(&settings);
        settings.passSize /= 2;
        settings.displacementModifier *= pow(2.0, -1.0 * h);
    }
    return map;
}

void passDiamondSquare(PassSettings *settings) {
    // Pass diamond
    for (int x = settings->passSize / 2; x < settings->mapSize;
         x += settings->passSize) {
        for (int y = settings->passSize / 2; y < settings->mapSize;
             y += settings->passSize) {
            diamond(x, y, settings);
        }
    }
    // Pass square
    for (int x = 0; x < settings->mapSize; x += settings->passSize / 2) {
        for (int y = settings->passSize / 2; y < settings->mapSize;
             y += settings->passSize) {
            square(x, y, settings);
        }
        x += settings->passSize / 2;
        if (x < settings->mapSize) {
            for (int y = 0; y < settings->mapSize; y += settings->passSize) {
                square(x, y, settings);
            }
        }
    }
}

void diamond(int x, int y, PassSettings *settings) {
    int points = 0;
    double total = 0.0;
    int dif = settings->passSize / 2;
    // Top left
    if (isValid(x - dif, y - dif, settings->mapSize)) {
        points++;
        total += (settings->map)[(y - dif) * settings->mapSize + (x - dif)];
    }
    // Top right
    if (isValid(x + dif, y - dif, settings->mapSize)) {
        points++;
        total += (settings->map)[(y - dif) * settings->mapSize + (x + dif)];
    }
    // Bottom right
    if (isValid(x + dif, y + dif, settings->mapSize)) {
        points++;
        total += (settings->map)[(y + dif) * settings->mapSize + (x + dif)];
    }
    // Bottom left
    if (isValid(x - dif, y + dif, settings->mapSize)) {
        points++;
        total += (settings->map)[(y + dif) * settings->mapSize + (x - dif)];
    }

    total /= (double) points;
    total += drand() * settings->displacementModifier;
    (settings->map)[y * settings->mapSize + x] = total;
}

void square(int x, int y, PassSettings *settings) {
    int points = 0;
    double total = 0.0;
    int dif = settings->passSize / 2;
    // Left
    if (isValid(x - dif, y, settings->mapSize)) {
        points++;
        total += (settings->map)[(y) * settings->mapSize + (x - dif)];
    }
    // Top
    if (isValid(x, y - dif, settings->mapSize)) {
        points++;
        total += (settings->map)[(y - dif) * settings->mapSize + (x)];
    }
    // Right
    if (isValid(x + dif, y, settings->mapSize)) {
        points++;
        total += (settings->map)[(y) * settings->mapSize + (x + dif)];
    }
    // Bottom
    if (isValid(x, y + dif, settings->mapSize)) {
        points++;
        total += (settings->map)[(y + dif) * settings->mapSize + (x)];
    }

    total /= (double) points;
    total += drand() * settings->displacementModifier;
    (settings->map)[y * settings->mapSize + x] = total;
}

bool isValid(int x, int y, int size) {
    return x >= 0 && x < size && y >= 0 && y < size;
}

double drand() {
    return dist(mt);
}
