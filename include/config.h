#pragma once

#include <Arduino.h>

// Credenciais geradas pelo PlatformIO a partir do .env local.
#include "private_config.h"

// --- Pinos de hardware ---
constexpr int PINO_TEMPERATURA = A0;
constexpr int PINO_PH = A1;
constexpr int PINO_TDS = A2;

constexpr int PINO_LED_TEMPERATURA = 2;
constexpr int PINO_LED_PH = 3;
constexpr int PINO_LED_TDS = 4;
constexpr int PINO_BUZZER = 5;
constexpr int PINO_BOTAO = 6;

// --- Configuração dos sensores ---
constexpr float RESISTOR_TEMPERATURA = 10000.0;
constexpr float RESISTOR_PH = 10000.0;
constexpr float RESISTOR_TDS = 10000.0;
constexpr float RESISTENCIA_NOMINAL_TERMISTOR = 10000.0;
constexpr float TEMPERATURA_NOMINAL = 25.0;
constexpr float COEFICIENTE_BETA = 3950.0;

// --- Limites de alerta ---
constexpr float TEMPERATURA_MINIMA = 26.0;
constexpr float TEMPERATURA_MAXIMA = 28.0;
constexpr float PH_MINIMO = 6.5;
constexpr float PH_MAXIMO = 7.5;
constexpr float TDS_MINIMO = 150.0;
constexpr float TDS_MAXIMO = 350.0;

// --- Temporização e comunicação ---
constexpr unsigned long VELOCIDADE_SERIAL = 9600;
constexpr unsigned long INTERVALO_LEITURA_SENSORES_MS = 2000;
constexpr unsigned long INTERVALO_PISCA_LENTO_MS = 1000;
constexpr unsigned long INTERVALO_PISCA_RAPIDO_MS = 150;
constexpr unsigned long INTERVALO_RECONEXAO_WIFI_MS = 15000;
constexpr unsigned long INTERVALO_ENVIO_NUVEM_MS = 15000UL;
constexpr unsigned long INTERVALO_SINCRONIZACAO_FILA_MS = 1000;
constexpr unsigned long INTERVALO_DEBOUNCE_BOTAO_MS = 200;
constexpr unsigned long INCREMENTO_SILENCIO_MS = 15000UL;
constexpr unsigned long TIMEOUT_SUPABASE_MS = 3000;
constexpr unsigned long TIMEOUT_HTTP_MS = 1000;
constexpr unsigned long EPOCH_MINIMO_VALIDO = 1700000000UL;
constexpr float LEITURA_INVALIDA = -999.0;

// --- Fila offline em RAM ---
// 400 registros de 20 bytes: 8 KB, aproximadamente 1h40min a cada 15 s.
constexpr int CAPACIDADE_FILA_OFFLINE = 400;
constexpr int TAMANHO_LOTE_ENVIO = 50;
