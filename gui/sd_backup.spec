# -*- mode: python ; coding: utf-8 -*-

block_cipher = None

# 直接指定CLI二进制文件路径
import os

# 确保CLI二进制文件存在
cli_binary = '../build/src/cli/backup_system.exe'
if not os.path.exists(cli_binary):
    raise FileNotFoundError(f"CLI二进制文件不存在: {cli_binary}")

a = Analysis(
    ['main.py'],
    pathex=['.'],
    binaries=[],
    datas=[(cli_binary, '.')],  # 将CLI二进制文件打包到根目录
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
    name='数据备份工具',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False,  # 调试时设置为True，方便查看输出
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
    icon=None,  # 可选：添加应用程序图标
)
