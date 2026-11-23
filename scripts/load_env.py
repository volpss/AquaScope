"""Hook de pré-compilação do PlatformIO, sem dependências externas."""

from pathlib import Path
import runpy

Import("env")

project_dir = Path(env.subst("$PROJECT_DIR"))
generated_dir = Path(env.subst("$BUILD_DIR")) / "generated"
config = runpy.run_path(str(project_dir / "scripts" / "env_config.py"))

try:
    config["generate_header"](project_dir / ".env", generated_dir / "private_config.h")
except ValueError as error:
    print("Configuração privada: " + str(error))
    env.Exit(1)

env.Append(CPPPATH=[str(generated_dir)])
