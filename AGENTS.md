# Aquario IoT

Consulte `project.md` para identidade, escopo e fontes e `status.md` para a situação vigente. Para perguntas históricas, use a skill `maintain-project-history`: pesquise os metadados antes de abrir poucos registros pertinentes.

- Use nomes de arquivos em inglês e mantenha as referências locais relativas à raiz do projeto ou ao arquivo que as contém.
- Não inclua caminhos absolutos da máquina, nomes de usuário, diretórios pessoais, credenciais ou outros dados pessoais nas instruções do projeto.
- Preserve regras específicas, estrutura técnica, fontes e histórico existentes.
- Distinga fatos confirmados, hipóteses e atividades planejadas. Registre decisões, mudanças, verificações ou marcos relevantes com evidências em `history/YYYY/YYYY-MM-DD-topic.md`; atualize `status.md` quando a situação vigente mudar.
- Consultas e planejamentos não autorizam gravações por si só. A estrutura local não autoriza editar fontes externas ou configurar automações.

## Código

- Mantenha os nomes de arquivos em inglês e os identificadores existentes em português, seguindo o padrão atual.
- Use pastas correspondentes em `include/` e `src/`: `hardware/` para sensores e indicadores, `network/` para Wi-Fi, HTTP e relógio NTP, e `data/` para banco, fila e telemetria. Restrinja o estado interno ao módulo responsável e preserve `src/main.cpp` como coordenador.
- Nos includes locais, use o caminho a partir de `include/`, como `hardware/sensors.h`.
- Configurações públicas ficam em `include/config.h`; credenciais e dados privados ficam no `.env`, ignorado pelo Git. Mantenha `.env.example` somente com valores fictícios e não copie credenciais para documentação, logs ou exemplos.
- O hook `scripts/load_env.py` gera `private_config.h` dentro de `.pio/` antes da compilação. Não versione o cabeçalho gerado. Valide alterações nesse carregamento com `python -m unittest discover -s test -p test_env_config.py` e com a compilação do firmware.
- Após mudanças no firmware, compile com o ambiente `uno_r4_wifi`. Distinga compilação, upload e funcionamento observado na placa.
- Atualize o mapa do `README.md` quando alterar as responsabilidades dos módulos.
