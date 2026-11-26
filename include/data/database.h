#pragma once

#include <Arduino.h>
#include "models.h"

// Serialização das medições e envio HTTPS à API REST do Supabase.
String montarMedicaoJson(const MedicaoPendente &medicao);
bool enviarJsonSupabase(const String &conteudoJson);
