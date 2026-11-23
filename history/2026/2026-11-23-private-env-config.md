---
date: "2026-11-23"
type: "change"
topics: ["firmware","configuração","privacidade"]
summary: "Credenciais migradas para .env local com exemplo fictício e geração de cabeçalho na compilação."
---

A pedido do usuário, as sete configurações privadas de Wi-Fi, OTA e Supabase foram retiradas de include/config.h e migradas para o .env local, preservando seus valores sem registrá-los neste histórico. O [.env.example](../../.env.example) contém somente valores fictícios; as regras de [.gitignore](../../.gitignore) excluem o .env, suas variantes e os artefatos de compilação.

O hook [load_env.py](../../scripts/load_env.py), configurado como pre-script no PlatformIO, usa o carregador [env_config.py](../../scripts/env_config.py) para validar o arquivo e gerar private_config.h dentro da pasta de build em .pio/. Os valores não são passados como macros na linha de comando. A configuração privada é incorporada ao firmware; esta separação não criptografa os dados gravados na placa.

Seis testes do carregador passaram, cobrindo caracteres especiais, comentários, configuração ausente, valores obrigatórios, erros sem exposição do conteúdo privado e atualização do cabeçalho. A compilação do ambiente uno_r4_wifi também passou, mantendo 15.968 bytes de RAM e 73.872 bytes de Flash. Não foi realizado upload ou teste na placa. As instruções para configurar uma nova cópia do projeto estão no [README](../../README.md).
