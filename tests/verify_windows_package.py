"""Check x64 PE imports in a package without accessing account data."""

import os
import pathlib
import sys

import pefile


def verify(root, executable):
    system = pathlib.Path(os.environ["SystemRoot"]) / "System32"
    pending = [root / executable, *root.glob("plugins/**/*.dll")]
    checked = set()
    failures = []
    while pending:
        path = pending.pop()
        if path in checked:
            continue
        checked.add(path)
        with pefile.PE(str(path), fast_load=True) as pe:
            if pe.FILE_HEADER.Machine != 0x8664:
                failures.append(f"non-x64 module: {path.name}")
            pe.parse_data_directories(
                directories=[
                    pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_IMPORT"],
                    pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT"],
                ]
            )
            imports = getattr(pe, "DIRECTORY_ENTRY_IMPORT", [])
            imports += getattr(pe, "DIRECTORY_ENTRY_DELAY_IMPORT", [])
            for entry in imports:
                name = entry.dll.decode("ascii")
                if name.lower().startswith(("api-ms-", "ext-ms-")):
                    continue
                local = root / name
                if local.is_file():
                    pending.append(local)
                elif not (system / name).is_file():
                    failures.append(f"missing {name} required by {path.name}")
    if failures:
        raise RuntimeError("\n".join(failures))
    print(f"x64 package imports resolved for {len(checked)} local modules")


if __name__ == "__main__":
    verify(pathlib.Path(sys.argv[1]).resolve(), sys.argv[2])
