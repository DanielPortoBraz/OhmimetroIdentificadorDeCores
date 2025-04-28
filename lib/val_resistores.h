#include <stdlib.h>

// Código de cores
static const char cores[10][10] = {
    "Preto", // 0
    "Marrom", // 1 
    "Vermelho", // 2
    "Laranja", // 3
    "Amarelo", // 4
    "Verde", // 5
    "Azul", // 6
    "Violeta", // 7
    "Cinza", // 8
    "Branco" // 9
};

// Cores para LED RGB
static const float rgb_cores[10][3] = {
    { 0.0f, 0.0f, 0.0f }, // Preto
    { 0.3f, 0.2f, 0.0f }, // Marrom
    { 1.0f, 0.0f, 0.0f }, // Vermelho
    { 1.0f, 0.5f, 0.0f }, // Laranja
    { 1.0f, 1.0f, 0.0f }, // Amarelo
    { 0.0f, 1.0f, 0.0f }, // Verde
    { 0.0f, 0.0f, 1.0f }, // Azul
    { 0.5f, 0.15f, 1.0f }, // Violeta
    { 0.5f, 0.5f, 0.5f }, // Cinza
    { 1.0f, 1.0f, 1.0f }  // Branco
};

// Valores da serie E24
static const int serie_E24[] = {
    10, 11, 12, 13, 15, 16, 18, 20,
    22, 24, 27, 30, 33, 36, 39, 43,
    47, 51, 56, 62, 68, 75, 82, 91
};
