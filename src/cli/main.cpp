#include <iostream>
#include "backup/BackupManager.h"
#include "filesystem/FileScanner.h"

int main() {
    std::cout << "Hello, Backup System!" << std::endl;

    BackupManager manager;
    manager.hello();

    FileScanner scanner;
    scanner.scan();

    return 0;
}
