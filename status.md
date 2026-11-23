# Situação atual — Aquario IoT

Atualização: 2026-11-23 (America/Sao_Paulo).

## Situação confirmada

O usuário informou que o firmware estava funcionando antes da modularização. Em 23/11/2026, o código foi separado em módulos `.h`/`.cpp`, preservando os cálculos, configurações, textos de comunicação e a ordem das tarefas. O `main.cpp` coordena a inicialização e o ciclo principal.

Os módulos agora estão agrupados em `hardware/`, `network/` e `data/`, com a mesma divisão em `src/` e `include/`. Os includes e as referências da documentação foram atualizados; a compilação após a movimentação passou, com o mesmo uso de memória.

O módulo `hardware/led_matrix` inicializa a matriz integrada e exibe um peixe estático no `setup()`, usando a biblioteca do framework `Arduino_LED_Matrix`. A compilação atual passou no ambiente `uno_r4_wifi`, com Renesas RA 1.9.0 e ArduinoOTA 1.1.1: 15.972 bytes de RAM e 73.984 bytes de Flash. O mapa de responsabilidades está no [README.md](README.md).

As configurações privadas de Wi-Fi, OTA e Supabase ficam no `.env`, mantendo os valores locais. As redes são configuradas na lista JSON `WIFI_REDES`, que substitui o par `WIFI_NOME`/`WIFI_SENHA`. O `.env.example` contém duas redes fictícias. O hook de pré-compilação valida a lista e gera o array de redes no cabeçalho privado em `.pio/`; os nove testes do carregador e a compilação passaram.

O firmware percorre as redes em ordem, com intervalo de 15 segundos após cada tentativa. Ao perder uma conexão estabelecida, tenta recuperar a mesma rede antes de alternar. A retomada circular e a conexão em redes diferentes ainda precisam de validação na placa.

## Pendências

- Validar a versão modularizada na placa: leituras, LEDs, alarme, página local, envio ao Supabase e retomada da fila após uma queda de Wi-Fi. Não foi feito upload nesta reorganização.
- Conferir o desenho do peixe na matriz integrada após o upload; a exibição física ainda não foi observada.
- Validar a alternância Wi-Fi com duas redes configuradas, a primeira indisponível e posterior queda da rede conectada.
- A fila permanece em RAM, sem persistência após reinicialização; pH e TDS mantêm a simulação atual.
- O serviço OTA foi preservado, mas o upload via Wi-Fi pelo PlatformIO não foi configurado ou verificado nesta reorganização.

## Próximo passo

Enviar o firmware por USB e conferir o funcionamento descrito no README.

## Evidências e cobertura do histórico

- [Registro da modularização](history/2026/2026-11-23-firmware-modularization.md).
- [Agrupamento por área](history/2026/2026-11-23-module-folders.md).
- [Peixe na matriz integrada](history/2026/2026-11-23-led-matrix-fish.md).
- [Configuração privada por .env](history/2026/2026-11-23-private-env-config.md).
- [Lista de redes Wi-Fi](history/2026/2026-11-23-wifi-network-list.md).
- Código em `src/` e interfaces em `include/`.
- O histórico local começa com esta reorganização; não representa uma migração completa dos acontecimentos anteriores.
