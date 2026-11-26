---
date: "2026-11-26"
type: "change"
topics: ["firmware","temperatura","pH","TDS","telemetria"]
summary: "Configurados os intervalos de medição de 3 segundos, 45 segundos e 15 minutos."
---

A pedido do usuário, temperatura passou a ter uma janela independente a cada 3 segundos e pH, a cada 45 segundos. O TDS usa 15 minutos entre inícios de ciclo com `USAR_INTERVALO_TESTE_TDS` em `false`. A primeira medição de TDS ainda começa após a primeira janela de temperatura. A [configuração](../../include/config.h) conserva a opção de teste de 30 segundos.

O [módulo de sensores](../../src/hardware/sensors.cpp) pausa as janelas de temperatura e pH durante o pulso de TDS e reinicia janelas interrompidas depois do pulso. A [telemetria](../../src/data/telemetry.cpp) enfileira cada leitura válida recém-concluída para envio individual, dispensando o temporizador separado que selecionava leituras da amostragem anterior. O controle de rede e a fila continuam podendo atrasar os envios. A mudança substitui as cadências descritas nos registros de [envio individual](2026-11-26-individual-telemetry.md) e [intervalo de teste do TDS](2026-11-25-tds-test-interval.md).

`pio run -e uno_r4_wifi` compilou com 13.060 bytes de RAM e 78.840 bytes de Flash. O upload e a verificação de cadência na placa e no Supabase permanecem no [checklist](../../PENDENCIES.md).
