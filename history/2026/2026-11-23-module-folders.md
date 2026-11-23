---
date: "2026-11-23"
type: "change"
topics: ["firmware","organização","PlatformIO"]
summary: "Módulos agrupados em hardware, network e data, com cabeçalhos espelhados."
---

O usuário solicitou agrupar os módulos em pastas por área, citando sensores e indicadores como parte física e Wi-Fi e servidor HTTP como conectividade. A organização plana criada na [modularização](2026-11-23-firmware-modularization.md) foi substituída por três grupos correspondentes em `src/` e `include/`: `hardware/` (sensores e indicadores), `network/` (conectividade, servidor HTTP e relógio NTP) e `data/` (banco, fila offline e telemetria).

O `main.cpp` permanece na raiz de `src/`; configuração e modelos compartilhados permanecem na raiz de `include/`. Foram movidos 16 arquivos, atualizados os includes locais e ajustadas as referências da documentação. A comparação dos arquivos confirmou que somente os caminhos dos includes mudaram nas implementações movidas.

A compilação do ambiente `uno_r4_wifi` passou após a movimentação, com o mesmo uso de memória: 15.748 bytes de RAM e 72.072 bytes de Flash. Não foi realizado upload ou teste na placa. O [README](../../README.md) contém a árvore atual e o mapa dos módulos; a [situação vigente](../../status.md) mantém as verificações pendentes.
