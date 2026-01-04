# -*- mode: python ; coding: utf-8 -*-

block_cipher = None

import os
import sys

spec_dir = os.path.dirname(os.path.abspath(sys.argv[0]))
project_root = os.path.abspath(os.path.join(spec_dir, '..'))
cli_name = 'backup_system.exe' if sys.platform.startswith('win') else 'backup_system'
cli_binary = os.path.join(project_root, 'build', 'src', 'cli', cli_name)

if not os.path.exists(cli_binary):
    raise FileNotFoundError(f"CLI binary not found: {cli_binary}")

a = Analysis(
    ['main.py'],
    pathex=[spec_dir],
    binaries=[(cli_binary, '.')],  # keep execute permission
    datas=[],
    hiddenimports=[],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=block_cipher,
    noarchive=False,
)
pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
    a.datas,
    [],
    name='sd_backup',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=False,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False, 
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
    icon=None, 
)
