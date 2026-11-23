#pragma once

#include <Arduino.h>
#include "models.h"

// Serialização das leituras e envio HTTPS à API REST do Supabase.
String montarLeituraJson(const LeituraOffline &item);
bool enviarJsonSupabase(const String &conteudoJson);
