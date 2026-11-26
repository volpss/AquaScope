#pragma once

#include <Arduino.h>

// --- Amostragem analógica ---
constexpr int RESOLUCAO_ADC = 10;
constexpr int ADC_MAXIMO = (1 << RESOLUCAO_ADC) - 1;
constexpr float TENSAO_REFERENCIA_ADC = 5.0f;
constexpr int TOTAL_AMOSTRAS = 30;
constexpr unsigned long INTERVALO_AMOSTRA_MS = 10UL;
constexpr int DESCARTE_POR_EXTREMO = 5;
constexpr int AMOSTRAS_CENTRAIS = TOTAL_AMOSTRAS - 2 * DESCARTE_POR_EXTREMO;
constexpr float LEITURA_INVALIDA = -999.0;

// --- Temperatura ---
constexpr int PINO_TEMPERATURA = A0;
constexpr unsigned long INTERVALO_MEDICAO_TEMPERATURA_MS = 3000UL;
constexpr float RESISTOR_TEMPERATURA = 10000.0;
constexpr float RESISTENCIA_NOMINAL_TERMISTOR = 10000.0;
constexpr float TEMPERATURA_NOMINAL = 25.0;
constexpr float COEFICIENTE_BETA = 3950.0;
constexpr float TEMPERATURA_MINIMA = 23.0;
constexpr float TEMPERATURA_MAXIMA = 28.0;

// --- pH ---
constexpr int PINO_PH = A1;
constexpr unsigned long INTERVALO_MEDICAO_PH_MS = 45000UL;
// Referência provisória ajustada com o BNC em curto. A inclinação é estimada.
constexpr float TENSAO_PH_NEUTRO = 2.500f;
constexpr float INCLINACAO_PH = -5.70f;
constexpr float PH_MINIMO = 6.5;
constexpr float PH_MAXIMO = 7.5;

// --- TDS ---
constexpr int PINO_TDS = A2;
constexpr int PINO_ALIMENTACAO_TDS = 7; // IN do módulo relé de 5 V. LOW aciona.
constexpr bool USAR_INTERVALO_TESTE_TDS = false;
constexpr unsigned long INTERVALO_MEDICAO_TDS_MS = USAR_INTERVALO_TESTE_TDS ? 30000UL : 900000UL;
constexpr unsigned long DURACAO_ALIMENTACAO_TDS_MS = 3000UL;
constexpr unsigned long TEMPO_ESTABILIZACAO_TDS_MS = 2500UL;
constexpr float TENSAO_MAXIMA_TDS = 2.4f; // Saída nominal de até 2,3 V com margem.
constexpr float FATOR_CALIBRACAO_TDS = 1.0f;
constexpr float TDS_MINIMO = 0.0; // ppm. Leituras válidas de TDS baixo são normais.
constexpr float TDS_MAXIMO = 350.0;

// --- Indicadores e alarme ---
constexpr int PINO_LED_TEMPERATURA = 2;
constexpr int PINO_LED_PH = 3;
constexpr int PINO_LED_TDS = 4;
constexpr int PINO_BUZZER = 5;
constexpr int PINO_BOTAO = 6;
constexpr unsigned long INTERVALO_PISCA_LENTO_MS = 1000UL;
constexpr unsigned long INTERVALO_PISCA_RAPIDO_MS = 150UL;
constexpr unsigned long INTERVALO_DEBOUNCE_BOTAO_MS = 200UL;
constexpr unsigned long INCREMENTO_SILENCIO_MS = 15000UL;

// --- Comunicação e relógio ---
constexpr unsigned long VELOCIDADE_SERIAL = 9600UL;
constexpr unsigned long INTERVALO_RECONEXAO_WIFI_MS = 15000UL;
constexpr unsigned long TEMPO_LIMITE_HTTP_MS = 1000UL;
constexpr unsigned long EPOCH_MINIMO_VALIDO = 1700000000UL;

// --- Telemetria e fila em RAM ---
constexpr unsigned long TEMPO_LIMITE_SUPABASE_MS = 3000UL;
constexpr unsigned long INTERVALO_SINCRONIZACAO_FILA_MS = 1000UL;
constexpr int CAPACIDADE_FILA_OFFLINE = 400;
