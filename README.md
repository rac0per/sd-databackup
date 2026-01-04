## 项目简介

一个支持目录备份/还原、文件压缩/解压的工具集，提供 CLI 与 PyQt6 GUI。备份可选镜像删除、哈夫曼/Huffman 或 LZ77 压缩、AES-256-CBC 加密，并在目标目录生成 `.backupmeta` 元数据以支撑恢复。

## 功能概览

- 备份：比较源目录与目标备份目录，生成新增/修改/删除计划，按需压缩并加密文件写入备份目录，记录 `.backupmeta`。
- 还原：读取 `.backupmeta`，逐文件解密、解压恢复，保留权限和时间戳。
- 压缩/解压：单文件或目录（目录会先打包为自定义 `SDPK` 容器）支持 Huffman 或 LZ77；可选 AES 加密/解密。
- GUI：`gui/main.py` 基于 PyQt6，封装备份、压缩/解压、还原操作，通过 `QProcess` 调用编译后的 `backup_system`。

## 依赖

- C++17、CMake 3.22+、OpenSSL (AES 加解密)
- Python 3.10+、PyQt6（仅 GUI）

## 构建与运行 CLI

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
# 可执行文件示例路径：build/src/cli/backup_system 或 build/bin/backup_system
```

### CLI 用法

```bash
# 压缩
backup_system compress <输入路径> <输出文件> <huffman|lz77> [-W <密码>]

# 解压
backup_system decompress <输入文件> <输出路径> <huffman|lz77> [-W <密码>]

# 备份
backup_system backup <源目录> <备份目录> [mirror] [compress=none|huffman|lz77] [-W <密码>]

# 还原
backup_system restore <备份目录> <还原目录> [-W <密码>]
```

说明：
- `mirror` 开启镜像模式，删除目标中源已删除的文件。
- `-W` 传入密码，启用 AES-256-CBC；未提供则不加密。
- 压缩目录时会先打包为单文件（魔数 `SDPK`），解压阶段若检测到该格式会自动解包到输出目录。

## 运行 GUI

```bash
pip install -r gui/requirements.txt
python gui/main.py
```

GUI 会自动寻找 `backup_system` 可执行文件（优先项目内 build 产物），提供备份、压缩/解压、还原的图形界面并显示日志。

## 元数据 `.backupmeta`

- 备份完成后写入备份根目录，记录源根路径、创建时间、压缩/加密算法、全部文件/目录条目及 mtime/size。
- 还原时据此决定是否解压/解密并恢复目录结构。
