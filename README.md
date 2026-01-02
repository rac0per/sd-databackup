# DataBackup 

## 项目概述

增量备份系统。该系统通过比较源目录和备份目录的文件树差异，来生成高效的备份计划，并执行增量备份操作。系统支持镜像模式（删除备份中不存在的文件）和干运行模式（仅显示操作而不实际执行）。

项目采用模块化设计，分为核心逻辑（core）和命令行接口（CLI）两部分，使用 CMake 进行构建，并集成 Google Test 进行单元测试。

## 当前实现功能

目前实现了以下功能：

### 1. 增量备份
- 通过比较源目录和备份目录的文件树差异，仅备份发生变化的文件
- 支持首次完整备份和后续增量备份

### 2. 镜像模式备份
- 自动删除备份目录中源目录已不存在的文件和目录
- 保持备份目录与源目录完全同步

### 3. 干运行模式
- 支持预览备份操作而不实际修改文件系统
- 便于用户检查备份计划的正确性

### 4. 文件系统树构建
- 递归扫描目录结构，构建完整的文件树
- 支持文件和目录的属性记录（大小、修改时间等）

### 5. 差异检测
- 检测文件的添加、删除和修改
- 基于文件大小和修改时间判断文件是否相同

### 6. 备份计划生成
- 将文件差异转换为具体的备份动作
- 支持创建目录、复制文件、更新文件、删除路径四种操作

### 7. 备份执行
- 安全执行备份计划，支持原子性操作
- 自动创建必要的父目录

## 架构设计

项目架构分为以下层次：

1. **文件系统抽象层**：`FileNode` 和 `FileTree` 类负责构建和表示文件系统树结构。
2. **差异计算层**：`FileTreeDiff` 类计算两个文件树之间的差异。
3. **备份管理层**：`BackupManager` 类协调整个备份流程。
4. **用户接口层**：CLI 程序提供命令行交互。

这种分层设计使得各模块职责清晰，便于测试和维护。

## 核心模块详细说明

### FileNode 类

#### 设计目的
`FileNode` 类表示文件系统中的一个节点（文件或目录）。它封装了文件的基本属性，并提供树结构支持。

#### 主要成员变量
- `name_`: 文件或目录名称
- `relativePath_`: 相对于根目录的相对路径
- `type_`: 文件类型（File 或 Directory）
- `size_`: 文件大小（仅对文件有效）
- `mtime_`: 最后修改时间
- `children_`: 子节点列表（仅对目录有效）

#### 主要方法
- `isFile() / isDirectory()`: 判断节点类型
- `getName() / getRelativePath()`: 获取名称和路径
- `getSize() / getMTime()`: 获取文件大小和修改时间
- `addChild()`: 添加子节点
- `getChildren()`: 获取子节点列表


### FileTree 类

#### 设计目的
`FileTree` 类负责构建和遍历整个文件系统树。它从指定的根目录开始，递归扫描所有文件和目录。

#### 主要成员变量
- `rootPath_`: 根目录路径
- `root_`: 根节点指针

#### 主要方法
- `build()`: 构建整个文件树
- `getRoot() / getRootPath()`: 获取根节点和路径
- `traverseDFS()`: 深度优先遍历文件树

#### 私有方法
- `buildRecursive()`: 递归构建子树
- `traverseDFSRecursive()`: 递归遍历


### FileTreeDiff 类

#### 设计目的
`FileTreeDiff` 类计算两个 `FileTree` 之间的差异，返回文件变化列表。

#### 主要枚举和结构体
- `ChangeType`: 变化类型（Added, Removed, Modified）
- `FileChange`: 变化结构体，包含类型、相对路径、旧节点和新节点

#### 主要方法
- `diff()`: 静态方法，计算两个树之间的差异

#### 私有方法
- `flatten()`: 将树扁平化为路径到节点的映射
- `isSameFile()`: 判断两个文件节点是否相同（基于大小和修改时间）

### BackupManager 类

#### 设计目的
`BackupManager` 类是备份系统的核心，负责协调整个备份流程：扫描目录、计算差异、生成计划、执行备份。

#### 配置结构体
- `BackupConfig`: 备份配置
  - `sourceRoot`: 源目录
  - `backupRoot`: 备份目录
  - `deleteRemoved`: 是否删除备份中不存在的文件（镜像模式）
  - `dryRun`: 是否仅模拟执行

#### 动作枚举和结构体
- `ActionType`: 动作类型（CreateDirectory, CopyFile, UpdateFile, RemovePath）
- `BackupAction`: 备份动作，包含类型、源路径、目标路径

#### 主要方法
- `scan()`: 扫描源目录和备份目录，构建文件树
- `buildPlan()`: 计算差异并生成备份计划
- `executePlan()`: 执行备份计划

#### 私有方法
- `resolveSourcePath() / resolveBackupPath()`: 解析相对路径为绝对路径
- `translateChangesToActions()`: 将文件变化转换为备份动作
- `executeAction()`: 执行单个备份动作

## 构建和运行

### 依赖要求
- C++17 编译器
- CMake 3.16+
- Google Test (已包含在项目中)

### 构建步骤
```bash
# 在项目根目录
mkdir build
cd build
cmake ..
make
```

### 运行测试
```bash
# 构建后运行
ctest
# 或直接运行可执行文件
./src/cli/backup_system
```

### 使用示例
CLI 程序会自动创建测试数据并演示备份过程。输出显示备份动作类型和目标路径。

## 测试说明

测试数据位于 `test_data/` 目录，包含源目录和备份目录的模拟数据。
