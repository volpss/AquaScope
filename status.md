# Situação atual — Aquario IoT

Atualização: 2026-11-25 (America/Sao_Paulo).

## Situação confirmada

O usuário informou que o firmware estava funcionando antes da modularização. Em 23/11/2026, o código foi separado em módulos `.h`/`.cpp`, preservando os cálculos, configurações, textos de comunicação e a ordem das tarefas. O `main.cpp` coordena a inicialização e o ciclo principal.

Os módulos agora estão agrupados em `hardware/`, `network/` e `data/`, com a mesma divisão em `src/` e `include/`. Os includes e as referências da documentação foram atualizados; a compilação após a movimentação passou, com o mesmo uso de memória.

O módulo `hardware/led_matrix` inicializa a matriz integrada e exibe um peixe estático no `setup()`, usando a biblioteca do framework `Arduino_LED_Matrix`. O mapa de responsabilidades está no [README.md](README.md).

As configurações privadas de Wi-Fi, OTA e Supabase ficam no `.env`, mantendo os valores locais. As redes são configuradas na lista JSON `WIFI_REDES`, que substitui o par `WIFI_NOME`/`WIFI_SENHA`. O `.env.example` contém duas redes fictícias. O hook de pré-compilação valida a lista e gera o array de redes no cabeçalho privado em `.pio/`; os nove testes do carregador e a compilação passaram.

O firmware percorre as redes em ordem, com intervalo de 15 segundos após cada tentativa. Ao perder uma conexão estabelecida, tenta recuperar a mesma rede antes de alternar. A retomada circular e a conexão em redes diferentes ainda precisam de validação na placa.

O ambiente `uno_r4_wifi_ph` usa `main.cpp` para conferir a tensão estimada de `PO` em `A1`. O usuário confirmou o ajuste de 2,50 V com o BNC em curto e optou por adiar a calibração da sonda em soluções tampão.

O ambiente padrão `uno_r4_wifi` usa `mainOriginal.cpp`. O firmware lê a temperatura em `A0`, o pH em `A1` e o TDS em `A2`. As leituras de pH usam a inclinação provisória de −5,70 pH/V. O TDS usa compensação pela temperatura lida em `A0`. Os dois ambientes compilaram para o Uno R4 WiFi. O firmware completo usou 16.220 bytes de RAM e 74.416 bytes de Flash.

O servidor HTTP local oferece as rotas JSON `GET /api/temperatura`, `GET /api/ph` e `GET /api/tds`. A compilação com as novas rotas passou no ambiente `uno_r4_wifi`.

O usuário informou que montou a alimentação do TDS com relé. O firmware configura `D7` como saída em `HIGH` e aciona o relé em `LOW` por 3 segundos a cada 5 minutos. A entrada `A2` é amostrada após 2,5 segundos de estabilização. Entre ciclos, o firmware mantém o último resultado. A compilação `uno_r4_wifi` passou com 16.240 bytes de RAM e 75.760 bytes de Flash.

## Pendências

- Validar a versão modularizada na placa: leituras, LEDs, alarme, página local, envio ao Supabase e retomada da fila após uma queda de Wi-Fi. Não foi feito upload nesta reorganização.
- Conferir o desenho do peixe na matriz integrada após o upload; a exibição física ainda não foi observada.
- Validar a alternância Wi-Fi com duas redes configuradas, a primeira indisponível e posterior queda da rede conectada.
- A fila permanece em RAM e perde os registros após reinicialização.
- Validar os três sensores juntos na placa e calibrar a sonda de pH em solução tampão quando possível.
- Após gravar o firmware, conferir a comutação do relé, medir o pulso no `+` do módulo e verificar a estabilidade da leitura do TDS.
- Após o upload, consultar as três rotas HTTP a partir de um dispositivo na mesma rede.
- O serviço OTA foi preservado, mas o upload via Wi-Fi pelo PlatformIO não foi configurado ou verificado nesta reorganização.

## Próximo passo

Gravar `uno_r4_wifi` por USB e conferir o pulso do relé e as leituras dos três sensores no Monitor Serial a 9600 baud.

## Evidências e cobertura do histórico

- [Registro da modularização](history/2026/2026-11-23-firmware-modularization.md).
- [Agrupamento por área](history/2026/2026-11-23-module-folders.md).
- [Peixe na matriz integrada](history/2026/2026-11-23-led-matrix-fish.md).
- [Configuração privada por .env](history/2026/2026-11-23-private-env-config.md).
- [Lista de redes Wi-Fi](history/2026/2026-11-23-wifi-network-list.md).
- [Teste de bancada do módulo de pH](history/2026/2026-11-25-ph-bench-voltage.md).
- [Integração dos três sensores](history/2026/2026-11-25-analog-sensor-integration.md).
- [Rotas HTTP das medições](history/2026/2026-11-25-measurement-http-routes.md).
- [Alimentação temporizada do TDS](history/2026/2026-11-25-tds-power-cycle.md).
- [Acionamento do relé do TDS](history/2026/2026-11-25-tds-relay-control.md).
- Código em `src/` e interfaces em `include/`.
- O histórico local começa com esta reorganização; não representa uma migração completa dos acontecimentos anteriores.
