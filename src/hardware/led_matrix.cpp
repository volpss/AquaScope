#include "hardware/led_matrix.h"

#include <Arduino_LED_Matrix.h>

namespace
{
ArduinoLEDMatrix matrizLed;

// Cada linha representa uma linha da matriz: 1 acende e 0 apaga o LED.
// Peixe voltado para a direita, com a cauda à esquerda e o olho apagado.
uint8_t desenhoPeixe[8][12] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
    {0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
} // namespace

void iniciarMatrizLed()
{
  matrizLed.begin();
  matrizLed.renderBitmap(desenhoPeixe, 8, 12);
}
