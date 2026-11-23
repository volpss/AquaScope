"""Leitura do .env e geração da configuração privada do firmware."""

import json
import re
from pathlib import Path


CONFIG_KEYS = (
    "WIFI_REDES", "OTA_NOME", "OTA_SENHA",
    "SUPABASE_HOST", "SUPABASE_CAMINHO", "SUPABASE_CHAVE",
)


def read_env(path):
    path = Path(path)
    if not path.is_file():
        raise ValueError("Crie o arquivo .env a partir de .env.example antes de compilar.")

    values = {}
    for number, line in enumerate(path.read_text(encoding="utf-8-sig").splitlines(), 1):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        match = re.fullmatch(r"([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.*)", line)
        if not match:
            raise ValueError(f"Formato inválido no .env, linha {number}.")
        key, raw = match.groups()
        try:
            if raw.startswith('"'):
                value, end = json.JSONDecoder().raw_decode(raw)
                tail = raw[end:].strip()
                if not isinstance(value, str) or (tail and not tail.startswith("#")):
                    raise ValueError()
            elif raw.startswith("'"):
                end = raw.index("'", 1)
                value = raw[1:end]
                tail = raw[end + 1:].strip()
                if tail and not tail.startswith("#"):
                    raise ValueError()
            else:
                value = re.split(r"\s+#", raw, maxsplit=1)[0].strip()
            if any(ord(char) < 32 for char in value):
                raise ValueError()
        except ValueError:
            # Nunca incluir o conteúdo da linha ou o valor privado no erro.
            raise ValueError(f"Valor inválido no .env, linha {number}.") from None
        if key in values:
            raise ValueError(f"Variável repetida no .env, linha {number}.")
        values[key] = value

    missing = [key for key in CONFIG_KEYS if not values.get(key)]
    if missing:
        raise ValueError("Preencha as variáveis obrigatórias no .env: " + ", ".join(missing))
    return values


def generate_header(env_path, output_path):
    values = read_env(env_path)
    networks = parse_wifi_networks(values["WIFI_REDES"])
    lines = ["#pragma once", "", "// Gerado a partir do .env; não versionar."]
    lines.extend([
        "struct RedeWifi", "{", "  const char *nome;", "  const char *senha;", "};",
        "", "constexpr RedeWifi REDES_WIFI[] = {",
    ])
    for network in networks:
        name = json.dumps(network["nome"], ensure_ascii=False)
        password = json.dumps(network["senha"], ensure_ascii=False)
        lines.append(f"    {{{name}, {password}}},")
    lines.extend([
        "};", "constexpr unsigned int QUANTIDADE_REDES_WIFI = sizeof(REDES_WIFI) / sizeof(REDES_WIFI[0]);", "",
    ])
    for key in CONFIG_KEYS:
        if key == "WIFI_REDES":
            continue
        lines.append(f"constexpr const char *{key} = {json.dumps(values[key], ensure_ascii=False)};")
    content = "\n".join(lines) + "\n"
    output_path = Path(output_path)
    if not output_path.exists() or output_path.read_text(encoding="utf-8") != content:
        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(content, encoding="utf-8")


def parse_wifi_networks(raw):
    try:
        networks = json.loads(raw)
    except ValueError:
        raise ValueError("WIFI_REDES deve conter uma lista JSON válida; confira .env.example.") from None
    if not isinstance(networks, list) or not networks:
        raise ValueError("WIFI_REDES deve conter pelo menos uma rede.")
    for index, network in enumerate(networks, 1):
        if not isinstance(network, dict) or set(network) != {"nome", "senha"}:
            raise ValueError(f"Rede {index} de WIFI_REDES deve conter os campos nome e senha.")
        name, password = network["nome"], network["senha"]
        if not isinstance(name, str) or not isinstance(password, str):
            raise ValueError(f"Rede {index} de WIFI_REDES: nome e senha devem ser strings.")
        if not name or len(name.encode("utf-8")) > 32:
            raise ValueError(f"Rede {index} de WIFI_REDES: nome deve ter de 1 a 32 bytes em UTF-8.")
        if any(ord(char) < 32 for char in name + password):
            raise ValueError(f"Rede {index} de WIFI_REDES contém caracteres de controle inválidos.")
    return networks
