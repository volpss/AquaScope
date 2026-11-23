import json
import re
from pathlib import Path
import tempfile
import unittest

from scripts.env_config import CONFIG_KEYS, generate_header, parse_wifi_networks, read_env


class EnvConfigTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.env_path = self.root / ".env"
        self.header = self.root / "generated" / "private_config.h"
        self.values = {key: "example" for key in CONFIG_KEYS}
        self.values["WIFI_REDES"] = json.dumps([{"nome": "Rede", "senha": "example"}])

    def write_env(self):
        self.env_path.write_text(
            "\n".join(key + "=" + json.dumps(value) for key, value in self.values.items()),
            encoding="utf-8",
        )

    def test_missing_file_explains_setup(self):
        with self.assertRaisesRegex(ValueError, r"\.env.example"):
            generate_header(self.env_path, self.header)
        self.assertFalse(self.header.exists())

    def test_quotes_spaces_hash_and_unicode_survive_cpp_generation(self):
        password = 'espaço # "aspas" \\ barra $VAR'
        networks = [{"nome": "Rede", "senha": password}, {"nome": "Outra rede", "senha": ""}]
        self.values["WIFI_REDES"] = json.dumps(networks)
        self.write_env()
        generate_header(self.env_path, self.header)
        rows = re.findall(r"^\s+\{(.+)\},$", self.header.read_text(encoding="utf-8"), re.M)
        self.assertEqual([json.loads("[" + row + "]") for row in rows],
                         [[network["nome"], network["senha"]] for network in networks])

    def test_comments_and_single_quotes(self):
        self.write_env()
        self.env_path.write_text(
            self.env_path.read_text(encoding="utf-8")
            .replace('OTA_SENHA="example"', "OTA_SENHA='senha # 1' # comentario")
            .replace('OTA_NOME="example"', 'OTA_NOME=dispositivo # comentario')
            + "\n# Comentario\n", encoding="utf-8")
        values = read_env(self.env_path)
        self.assertEqual(values["OTA_SENHA"], "senha # 1")
        self.assertEqual(values["OTA_NOME"], "dispositivo")

    def test_invalid_input_never_discloses_private_value(self):
        for line in ['OTA_NOME="segredo_invalido', 'segredo_invalido',
                     'OTA_NOME="segredo_invalido" texto_extra']:
            self.env_path.write_text(line, encoding="utf-8")
            with self.assertRaises(ValueError) as error:
                generate_header(self.env_path, self.header)
            self.assertNotIn("segredo_invalido", str(error.exception))
        self.assertFalse(self.header.exists())

    def test_required_values_and_duplicates(self):
        self.values["SUPABASE_CHAVE"] = ""
        self.write_env()
        with self.assertRaisesRegex(ValueError, "SUPABASE_CHAVE"):
            read_env(self.env_path)
        self.values["SUPABASE_CHAVE"] = "example"
        self.write_env()
        with self.env_path.open("a", encoding="utf-8") as file:
            file.write('\nOTA_NOME="outro"\n')
        with self.assertRaisesRegex(ValueError, "repetida"):
            read_env(self.env_path)

    def test_only_configuration_changes_rewrite_header(self):
        self.write_env()
        generate_header(self.env_path, self.header)
        original_mtime = self.header.stat().st_mtime_ns
        original_content = self.header.read_bytes()
        generate_header(self.env_path, self.header)
        self.assertEqual(self.header.stat().st_mtime_ns, original_mtime)
        self.values["WIFI_REDES"] = json.dumps([{"nome": "outra rede", "senha": "example"}])
        self.write_env()
        generate_header(self.env_path, self.header)
        self.assertNotEqual(self.header.read_bytes(), original_content)

    def test_invalid_network_lists_do_not_disclose_values(self):
        invalid = ["segredo_invalido", "[]", '{}', '["segredo_invalido"]',
                   '[{"nome":"segredo_invalido"}]',
                   '[{"nome":"segredo_invalido","senha":123}]',
                   json.dumps([{"nome": "", "senha": "segredo_invalido"}]),
                   json.dumps([{"nome": "x" * 33, "senha": "segredo_invalido"}]),
                   json.dumps([{"nome": "Rede", "senha": "segredo_invalido\n"}])]
        for raw in invalid:
            with self.subTest(raw=raw):
                self.values["WIFI_REDES"] = raw
                self.write_env()
                with self.assertRaises(ValueError) as error:
                    generate_header(self.env_path, self.header)
                self.assertNotIn("segredo_invalido", str(error.exception))
                self.assertFalse(self.header.exists())

    def test_example_produces_two_networks_in_priority_order(self):
        example = Path(__file__).resolve().parents[1] / ".env.example"
        generate_header(example, self.header)
        networks = parse_wifi_networks(read_env(example)["WIFI_REDES"])
        self.assertEqual([network["nome"] for network in networks], ["RedeCasa", "RedeFaculdade"])

    def test_single_network_and_apostrophe_are_supported(self):
        raw = '[{"nome":"Rede","senha":"d\\u0027agua"}]'
        self.write_env()
        text = self.env_path.read_text(encoding="utf-8")
        lines = ["WIFI_REDES='" + raw + "'" if line.startswith("WIFI_REDES=") else line
                 for line in text.splitlines()]
        self.env_path.write_text("\n".join(lines), encoding="utf-8")
        networks = parse_wifi_networks(read_env(self.env_path)["WIFI_REDES"])
        self.assertEqual(networks, [{"nome": "Rede", "senha": "d'agua"}])


if __name__ == "__main__":
    unittest.main()
