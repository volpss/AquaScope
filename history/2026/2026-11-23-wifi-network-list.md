---
date: "2026-11-23"
type: "change"
topics: ["firmware","Wi-Fi","configuração"]
summary: "Lista de redes Wi-Fi no .env com tentativas em ordem e retomada circular."
---

O usuário aprovou a lista JSON WIFI_REDES e solicitou atualizar também o .env.example. A rede local anteriormente configurada foi migrada para uma lista de objetos com nome e senha, sem modificar suas credenciais ou os demais dados privados. O exemplo contém duas redes fictícias. Essa configuração substitui o par WIFI_NOME/WIFI_SENHA utilizado anteriormente.

O [carregador](../../scripts/env_config.py) valida a lista não vazia, os campos e tipos e gera RedeWifi, REDES_WIFI e QUANTIDADE_REDES_WIFI no cabeçalho privado em .pio/. A placa não precisa interpretar JSON. Senha vazia representa rede aberta. As mensagens de erro não reproduzem os valores privados.

O módulo de [conectividade](../../src/network/connectivity.cpp) começa na primeira rede, alterna após o intervalo configurado e volta ao início depois da última. O intervalo de 15 segundos é contado após o retorno da chamada WiFi.begin. Quando uma conexão estabelecida cai, tenta recuperar a mesma rede antes de passar à seguinte. Enquanto conectado com IP, permanece na rede atual. O log identifica as tentativas somente pelo índice.

Os nove testes do carregador passaram, incluindo geração de múltiplas redes na ordem configurada, caracteres especiais, rede aberta e rejeição de listas inválidas. A compilação passou no ambiente uno_r4_wifi: 15.972 bytes de RAM e 73.984 bytes de Flash. Não foi realizado upload nem teste real de alternância entre redes. O [README](../../README.md) descreve a configuração e o comportamento.
