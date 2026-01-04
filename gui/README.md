# SD Databackup GUI

This is a minimal PyQt6 GUI wrapper for the project's `backup_system` CLI.

Quick start:

1. Install dependencies (prefer a virtualenv):

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r gui/requirements.txt
```

2. Build the C++ project so `backup_system` exists (from repository root):

```bash
cmake -S . -B build
cmake --build build -- -j2
```

3. Run the GUI:

```bash
python3 gui/main.py
```

The GUI will try to locate the `backup_system` binary under `build/src/cli/backup_system`,
`build/bin/backup_system`, or in your `PATH`.

Notes:
- This GUI simply invokes the existing CLI and streams its stdout/stderr into the log view.
- You can expand the GUI to support compress/decompress, restore, or finer-grained options.
