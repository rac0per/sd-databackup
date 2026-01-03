import re
import os

# 正则表达式模式：匹配单行注释和多行注释
# 注意：这个模式会优先匹配多行注释，然后是单行注释
COMMENT_PATTERN = re.compile(r'(/\*[\s\S]*?\*/)|(//.*$)', re.MULTILINE)

def remove_comments(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 移除所有注释
    cleaned_content = COMMENT_PATTERN.sub('', content)
    
    # 移除空行（可选，根据需要）
    cleaned_content = '\n'.join([line for line in cleaned_content.splitlines() if line.strip()])
    
    # 写入清理后的内容
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(cleaned_content)
    
    print(f"已处理文件: {file_path}")

if __name__ == "__main__":
    # 定义需要处理的文件路径
    files = [
        # compression目录文件
        "src/core/compression/Compression.cpp",
        "src/core/compression/Huffman.cpp",
        "src/core/compression/LZ77.cpp",
        "src/core/compression/Compression.h",
        "src/core/compression/Huffman.h",
        "src/core/compression/LZ77.h",
        
        # encryption目录文件
        "src/core/encryption/Encryption.cpp",
        "src/core/encryption/MD5.cpp",
        "src/core/encryption/Encryption.h",
        "src/core/encryption/MD5.h",
        
        # 测试用例文件
        "tests/CompressionTest.cpp",
        "tests/EncryptionTest.cpp",
        "tests/FileTreeDiffTest.cpp",
        "tests/FileNodeTest.cpp",
        "tests/FileTreeTest.cpp",
        "tests/BackupManagerTest.cpp"
    ]
    
    # 遍历所有文件并处理
    for file in files:
        full_path = os.path.join(os.getcwd(), file)
        if os.path.exists(full_path):
            remove_comments(full_path)
        else:
            print(f"文件不存在: {full_path}")
    
    print("所有文件处理完成！")
