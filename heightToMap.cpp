#include <SFML/Graphics.hpp>
#include <stdio.h>
#include <iostream>

struct TextureSettings {
    double water;
    double sand;
    double grass;
    double forest;
    double stone;
};

sf::Color getMapColor(double value);
sf::Color multColor(sf::Color c, double mult);

int main(int argc, char **argv) {
    const char *filename = argc > 1 ? argv[1] : "heightmap.png";
    const char *out = argc > 2 ? argv[2] : "map.png";
    sf::Image heightmap;
    std::cout << "Reading image...\n";
    heightmap.loadFromFile(filename);
    sf::Vector2u imageSize = heightmap.getSize();
    for (int x = 0; x < imageSize.x; x++) {
        for (int y = 0; y < imageSize.y; y++) {
            heightmap.setPixel(x, y, getMapColor((double) (heightmap.getPixel(x, y).r) / 255.0));
        }
        if (x % (imageSize.x / 100) == 0) {
            int donePercent = (int) (((double (x)) / imageSize.x) * 100);
            printf("\33[2K\rConverting: %d%% done", donePercent);
            fflush(stdout);
        }
    }
    printf("\33[2K\rConverting: 100%% done");
    std::cout << "\nWriting image...\n";
    heightmap.saveToFile(out);
    return 0;
}

sf::Color getMapColor(double value) {
    sf::Color color;
    TextureSettings textureSettings;
    textureSettings.water = 0.25;
    textureSettings.sand = 0.4;
    textureSettings.grass = 0.6;
    textureSettings.forest = 0.8;
    textureSettings.stone = 0.9;
    TextureSettings *settings = &textureSettings;
    bool foggy = true;
    if (value < settings->water) {
        color = sf::Color(0, 0, 220);
    } else if (value < settings->sand) {
        color = sf::Color(210, 210, 10);
        if (foggy) {
            color = multColor
                (color, 1.0
                 - (value - settings->water)
                 / (settings->sand - settings->water));
            color += multColor
                (sf::Color(0, 205, 0)
                 , (value - settings->water)
                 / (settings->sand - settings->water));
        }
    } else if (value < settings->grass) {
        color = sf::Color(0, 205, 0);
        if (foggy) {
            color = multColor
                (color, 1.0
                 - (value - settings->sand)
                 / (settings->grass - settings->sand));
            color += multColor
                (sf::Color(0, 100, 0)
                 , (value - settings->sand)
                 / (settings->grass - settings->sand));
        }
    } else if (value < settings->forest) {
        color = sf::Color(0, 100, 0);
        if (foggy) {
            color = multColor
                (color, 1.0
                 - (value - settings->grass)
                 / (settings->forest - settings->grass));
            color += multColor
                (sf::Color(120, 120, 120)
                 , (value - settings->grass)
                 / (settings->forest - settings->grass));
        }
    } else if (value < settings->stone) {
        color = sf::Color(120, 120, 120);
        if (foggy) {
            color = multColor
                (color, 1.0
                 - (value - settings->forest)
                 / (settings->stone - settings->forest));
            color += multColor
                (sf::Color(255, 255, 255)
                 , (value - settings->forest)
                 / (settings->stone - settings->forest));
        }
    } else {
        color = sf::Color(255, 255, 255);
    }
    return color;
}

sf::Color multColor(sf::Color c, double mult) {
    c.r = (double) (c.r) * mult;
    c.g = (double) (c.g) * mult;
    c.b = (double) (c.b) * mult;
    return c;
}
