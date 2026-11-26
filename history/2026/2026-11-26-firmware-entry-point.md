---
date: "2026-11-26"
type: "change"
topics: ["firmware","PlatformIO","pH"]
summary: "Restaurado src/main.cpp como ponto de entrada do firmware padrão."
---

O usuário pediu para mover o firmware de `src/main-original.cpp` para [src/main.cpp](../../src/main.cpp). O [sketch de bancada](../../src/benches/ph-bench.cpp) que ocupava esse caminho foi movido na ocasião para `src/ph-bench.cpp`, preservando o teste de tensão do módulo de pH. A conferência por SHA-256 confirmou o conteúdo dos dois arquivos após a movimentação.

O [PlatformIO](../../platformio.ini) compila `main.cpp` e os módulos no ambiente padrão `uno_r4_wifi`. O ambiente `uno_r4_wifi_ph` seleciona somente `ph-bench.cpp`. Os dois ambientes compilaram com `pio run -e uno_r4_wifi -e uno_r4_wifi_ph`. A validação na placa segue pendente.
