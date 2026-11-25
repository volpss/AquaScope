#include <Arduino.h>

constexpr int PINO_PH = A1;
constexpr int AMOSTRAS = 30;

void setup()
{
  Serial.begin(9600);
  analogReadResolution(10); // 0 a 1023
  delay(1000);

  Serial.println("Leitura de tensao do modulo de pH em A1");
}

void loop()
{
  int leituras[AMOSTRAS];

  for (int i = 0; i < AMOSTRAS; i++)
  {
    leituras[i] = analogRead(PINO_PH);
    delay(10);
  }

  // Ordena as amostras para remover as 5 menores e as 5 maiores.
  for (int i = 0; i < AMOSTRAS - 1; i++)
  {
    for (int j = i + 1; j < AMOSTRAS; j++)
    {
      if (leituras[j] < leituras[i])
      {
        int temporaria = leituras[i];
        leituras[i] = leituras[j];
        leituras[j] = temporaria;
      }
    }
  }

  long soma = 0;
  for (int i = 5; i < AMOSTRAS - 5; i++)
  {
    soma += leituras[i];
  }

  float mediaADC = soma / 20.0;
  float tensao = mediaADC * (5.0 / 1023.0);

  Serial.print("ADC medio: ");
  Serial.print(mediaADC, 1);
  Serial.print(" | Tensao: ");
  Serial.print(tensao, 3);
  Serial.println(" V");

  delay(1000);
}