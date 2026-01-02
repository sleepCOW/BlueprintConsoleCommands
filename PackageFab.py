import json
import subprocess
from pathlib import Path

VersionsToPackage = ["5.4.0", "5.5.0", "5.6.0", "5.7.0"]

PLUGIN_FILE = Path("BlueprintCMD.uplugin")
RESOURCES_DIR = Path("Resources")
SOURCE_DIR = Path("Source")

ARCHIVE_BASENAME = "BlueprintConsoleCommands_FAB"  # output will be BlueprintCMD_<version>.zip

def run_7z(args: list[str]) -> None:
    """Run 7z with given args, raising a helpful error on failure."""
    try:
        subprocess.run(["C:\\Program Files\\7-Zip\\7z.exe", *args], check=True)
    except FileNotFoundError as e:
        raise RuntimeError(
            "7z executable not found. Install 7-Zip and ensure '7z' is in PATH "
            "(or change the script to point to 7z.exe)."
        ) from e
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"7z failed with exit code {e.returncode}") from e

# Basic validation
if not PLUGIN_FILE.exists():
    raise FileNotFoundError(f"{PLUGIN_FILE} not found in current directory")
if not RESOURCES_DIR.exists():
    raise FileNotFoundError(f"{RESOURCES_DIR} folder not found in current directory")
if not SOURCE_DIR.exists():
    raise FileNotFoundError(f"{SOURCE_DIR} folder not found in current directory")

# Read JSON once
with PLUGIN_FILE.open("r", encoding="utf-8") as f:
    data = json.load(f)

for version in VersionsToPackage:
    # Update EngineVersion
    data["EngineVersion"] = version

    # Write back
    with PLUGIN_FILE.open("w", encoding="utf-8") as f:
        json.dump(data, f, indent='\t', ensure_ascii=False)

    # Create archive
    archive_name = f"{ARCHIVE_BASENAME}_{version}.zip"

    # 7z add/create zip: no password by default
    # -tzip: zip format
    # -mx=9: max compression (optional)
    # -y: assume Yes on queries (optional)
    run_7z([
        "a",
        "-tzip",
        "-mx=9",
        "-y",
        archive_name,
        str(RESOURCES_DIR),
        str(SOURCE_DIR),
        str(PLUGIN_FILE),
    ])

    print(f"EngineVersion set to {version}; created {archive_name}")