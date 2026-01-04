#!/usr/bin/env python3
from PyQt6.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout,
    QLabel, QLineEdit, QPushButton, QFileDialog, QTextEdit,
    QCheckBox, QComboBox, QTabWidget
)
from PyQt6.QtCore import QProcess
import sys
import shutil
import os


def find_cli_binary():
    candidates = [
        os.path.join('build', 'src', 'cli', 'backup_system'),
        os.path.join('build', 'bin', 'backup_system'),
        os.path.join('build', 'backup_system'),
        'backup_system'
    ]
    for p in candidates:
        if os.path.isabs(p):
            if os.path.exists(p) and os.access(p, os.X_OK):
                return p
        else:
            full = os.path.abspath(p)
            if os.path.exists(full) and os.access(full, os.X_OK):
                return full
    # try PATH
    which = shutil.which('backup_system')
    if which:
        return which
    return None


class BackupGUI(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle('数据备份')
        self.resize(700, 480)
        self.cli = find_cli_binary()

        layout = QVBoxLayout()

        # 选项卡：备份 | 压缩/解压 | 还原
        tabs = QTabWidget()

        # --- 备份 选项卡 ---
        backup_tab = QWidget()
        b_layout = QVBoxLayout()

        # Source / Backup selectors
        src_layout = QHBoxLayout()
        src_layout.addWidget(QLabel('源目录:'))
        self.src_edit = QLineEdit()
        src_layout.addWidget(self.src_edit)
        btn_src = QPushButton('浏览')
        btn_src.clicked.connect(self.browse_source)
        src_layout.addWidget(btn_src)
        b_layout.addLayout(src_layout)

        dst_layout = QHBoxLayout()
        dst_layout.addWidget(QLabel('备份目录:'))
        self.dst_edit = QLineEdit()
        dst_layout.addWidget(self.dst_edit)
        btn_dst = QPushButton('浏览')
        btn_dst.clicked.connect(self.browse_backup)
        dst_layout.addWidget(btn_dst)
        b_layout.addLayout(dst_layout)

        # Options
        opts_layout = QHBoxLayout()
        self.mirror_cb = QCheckBox('镜像（删除目标中不存在的文件）')
        opts_layout.addWidget(self.mirror_cb)
        opts_layout.addWidget(QLabel('压缩算法:'))
        self.comp_box = QComboBox()
        self.comp_box.addItems(['none', 'huffman', 'lz77'])
        opts_layout.addWidget(self.comp_box)
        self.encrypt_cb = QCheckBox('启用 AES 加密')
        self.encrypt_cb.stateChanged.connect(self.toggle_key)
        opts_layout.addWidget(self.encrypt_cb)
        self.key_edit = QLineEdit()
        self.key_edit.setPlaceholderText('加密密码')
        self.key_edit.setEnabled(False)
        opts_layout.addWidget(self.key_edit)
        b_layout.addLayout(opts_layout)

        run_layout = QHBoxLayout()
        self.run_btn = QPushButton('Run Backup')
        self.run_btn.clicked.connect(self.run_backup)
        run_layout.addWidget(self.run_btn)
        self.cancel_btn = QPushButton('取消')
        self.cancel_btn.clicked.connect(self.cancel_process)
        self.cancel_btn.setEnabled(False)
        run_layout.addWidget(self.cancel_btn)
        b_layout.addLayout(run_layout)

        backup_tab.setLayout(b_layout)
        tabs.addTab(backup_tab, '备份')

        # --- 压缩/解压 选项卡 ---
        comp_tab = QWidget()
        c_layout = QVBoxLayout()

        # Input / Output
        in_layout = QHBoxLayout()
        in_layout.addWidget(QLabel('输入路径(文件/文件夹):'))
        self.in_edit = QLineEdit()
        in_layout.addWidget(self.in_edit)
        btn_in = QPushButton('浏览')
        btn_in.clicked.connect(self.browse_input_file)
        in_layout.addWidget(btn_in)
        btn_in_dir = QPushButton('选文件夹')
        btn_in_dir.clicked.connect(self.browse_input_dir)
        in_layout.addWidget(btn_in_dir)
        c_layout.addLayout(in_layout)

        out_layout = QHBoxLayout()
        out_layout.addWidget(QLabel('输出路径:'))
        self.out_edit = QLineEdit()
        out_layout.addWidget(self.out_edit)
        btn_out = QPushButton('浏览')
        btn_out.clicked.connect(self.browse_output_file)
        out_layout.addWidget(btn_out)
        btn_out_dir = QPushButton('选文件夹')
        btn_out_dir.clicked.connect(self.browse_output_dir)
        out_layout.addWidget(btn_out_dir)
        c_layout.addLayout(out_layout)

        algo_layout = QHBoxLayout()
        algo_layout.addWidget(QLabel('算法:'))
        self.algo_box = QComboBox()
        self.algo_box.addItems(['huffman', 'lz77'])
        algo_layout.addWidget(self.algo_box)
        self.comp_encrypt_cb = QCheckBox('使用 AES 加密')
        self.comp_encrypt_cb.stateChanged.connect(self.toggle_comp_key)
        algo_layout.addWidget(self.comp_encrypt_cb)
        self.comp_key_edit = QLineEdit()
        self.comp_key_edit.setPlaceholderText('密码')
        self.comp_key_edit.setEnabled(False)
        algo_layout.addWidget(self.comp_key_edit)
        c_layout.addLayout(algo_layout)

        c_run_layout = QHBoxLayout()
        self.compress_btn = QPushButton('压缩')
        self.compress_btn.clicked.connect(self.run_compress)
        c_run_layout.addWidget(self.compress_btn)
        self.decompress_btn = QPushButton('解压')
        self.decompress_btn.clicked.connect(self.run_decompress)
        c_run_layout.addWidget(self.decompress_btn)
        c_layout.addLayout(c_run_layout)

        comp_tab.setLayout(c_layout)
        tabs.addTab(comp_tab, '压缩/解压')

        # --- 还原 选项卡 ---
        restore_tab = QWidget()
        r_layout = QVBoxLayout()

        rb_layout = QHBoxLayout()
        rb_layout.addWidget(QLabel('备份目录:'))
        self.restore_backup_edit = QLineEdit()
        rb_layout.addWidget(self.restore_backup_edit)
        btn_rb = QPushButton('浏览')
        btn_rb.clicked.connect(self.browse_restore_backup)
        rb_layout.addWidget(btn_rb)
        r_layout.addLayout(rb_layout)

        rr_layout = QHBoxLayout()
        rr_layout.addWidget(QLabel('还原到:'))
        self.restore_to_edit = QLineEdit()
        rr_layout.addWidget(self.restore_to_edit)
        btn_rr = QPushButton('浏览')
        btn_rr.clicked.connect(self.browse_restore_to)
        rr_layout.addWidget(btn_rr)
        r_layout.addLayout(rr_layout)

        r_opts = QHBoxLayout()
        r_opts.addWidget(QLabel('加密密钥:'))
        self.restore_key = QLineEdit()
        r_opts.addWidget(self.restore_key)
        r_layout.addLayout(r_opts)

        r_run = QHBoxLayout()
        self.restore_btn = QPushButton('开始还原')
        self.restore_btn.clicked.connect(self.run_restore)
        r_run.addWidget(self.restore_btn)
        r_layout.addLayout(r_run)

        restore_tab.setLayout(r_layout)
        tabs.addTab(restore_tab, '还原')

        layout.addWidget(tabs)

        # Log and status
        self.log = QTextEdit()
        self.log.setReadOnly(True)
        layout.addWidget(self.log)

        self.status_label = QLabel('命令行工具: ' + (self.cli or '未找到'))
        layout.addWidget(self.status_label)

        self.setLayout(layout)

        self.process = QProcess(self)
        self.process.readyReadStandardOutput.connect(self.on_stdout)
        self.process.readyReadStandardError.connect(self.on_stderr)
        self.process.finished.connect(self.on_finished)

    def browse_source(self):
        d = QFileDialog.getExistingDirectory(self, '选择源目录')
        if d:
            self.src_edit.setText(d)

    def browse_backup(self):
        d = QFileDialog.getExistingDirectory(self, '选择备份目录')
        if d:
            self.dst_edit.setText(d)

    def toggle_key(self, state):
        self.key_edit.setEnabled(bool(state))

    def append_log(self, text):
        self.log.append(text)

    def on_stdout(self):
        data = self.process.readAllStandardOutput().data().decode()
        if data:
            self.append_log(data)

    def on_stderr(self):
        data = self.process.readAllStandardError().data().decode()
        if data:
            self.append_log('[错误] ' + data)

    def on_finished(self):
        code = self.process.exitCode()
        self.append_log(f'进程结束，退出码 {code}')
        # re-enable controls
        try:
            self.cancel_btn.setEnabled(False)
        except Exception:
            pass
        try:
            self.run_btn.setEnabled(True)
        except Exception:
            pass
        try:
            self.compress_btn.setEnabled(True)
            self.decompress_btn.setEnabled(True)
            self.restore_btn.setEnabled(True)
        except Exception:
            pass

    def run_backup(self):
        if not self.cli:
            self.append_log('未找到备份二进制文件，请先构建项目。')
            return

        src = self.src_edit.text().strip()
        dst = self.dst_edit.text().strip()
        if not src or not dst:
            self.append_log('请同时选择源目录和备份目录。')
            return

        args = ['backup', src, dst]
        if self.mirror_cb.isChecked():
            args.append('mirror')
        comp = self.comp_box.currentText()
        if comp and comp != 'none':
            args.append(f'compress={comp}')
        if self.encrypt_cb.isChecked():
            key = self.key_edit.text()
            if not key:
                self.append_log('已启用加密，但密码为空。')
                return
            args.extend(['-W', key])

        self.append_log('开始执行: ' + ' '.join([self.cli] + args))
        # disable controls while running
        self.run_btn.setEnabled(False)
        try:
            self.cancel_btn.setEnabled(True)
        except Exception:
            pass
        self.process.start(self.cli, args)

    def cancel_process(self):
        if self.process.state() != QProcess.ProcessState.NotRunning:
            self.process.kill()
            self.append_log('进程已被用户终止')

    # --- Compression helpers ---
    def browse_input_file(self):
        p, _ = QFileDialog.getOpenFileName(self, '选择输入文件')
        if p:
            self.in_edit.setText(p)

    def browse_input_dir(self):
        p = QFileDialog.getExistingDirectory(self, '选择输入文件夹')
        if p:
            self.in_edit.setText(p)

    def browse_output_file(self):
        p, _ = QFileDialog.getSaveFileName(self, '选择输出文件')
        if p:
            self.out_edit.setText(p)

    def browse_output_dir(self):
        p = QFileDialog.getExistingDirectory(self, '选择输出文件夹')
        if p:
            self.out_edit.setText(p)

    def toggle_comp_key(self, state):
        self.comp_key_edit.setEnabled(bool(state))

    def run_compress(self):
        if not self.cli:
            self.append_log('未找到备份二进制文件。')
            return
        inp = self.in_edit.text().strip()
        out = self.out_edit.text().strip()
        algo = self.algo_box.currentText()
        if not inp or not out:
            self.append_log('请选择用于压缩的输入路径和输出文件。')
            return
        args = ['compress', inp, out, algo]
        if self.comp_encrypt_cb.isChecked():
            key = self.comp_key_edit.text()
            if not key:
                self.append_log('已启用加密，但密码为空。')
                return
            args.extend(['-W', key])
        self.append_log('开始执行: ' + ' '.join([self.cli] + args))
        self.compress_btn.setEnabled(False)
        self.decompress_btn.setEnabled(False)
        try:
            self.cancel_btn.setEnabled(True)
        except Exception:
            pass
        self.process.start(self.cli, args)

    def run_decompress(self):
        if not self.cli:
            self.append_log('未找到备份二进制文件。')
            return
        inp = self.in_edit.text().strip()
        out = self.out_edit.text().strip()
        algo = self.algo_box.currentText()
        if not inp or not out:
            self.append_log('请选择用于解压的输入文件和输出路径（若输入是目录压缩包，则输出应为目录）。')
            return
        args = ['decompress', inp, out, algo]
        if self.comp_encrypt_cb.isChecked():
            key = self.comp_key_edit.text()
            if not key:
                self.append_log('已启用加密，但密码为空。')
                return
            args.extend(['-W', key])
        self.append_log('开始执行: ' + ' '.join([self.cli] + args))
        self.compress_btn.setEnabled(False)
        self.decompress_btn.setEnabled(False)
        try:
            self.cancel_btn.setEnabled(True)
        except Exception:
            pass
        self.process.start(self.cli, args)

    # --- Restore helpers ---
    def browse_restore_backup(self):
        d = QFileDialog.getExistingDirectory(self, '选择备份目录')
        if d:
            self.restore_backup_edit.setText(d)

    def browse_restore_to(self):
        d = QFileDialog.getExistingDirectory(self, '选择还原目标目录')
        if d:
            self.restore_to_edit.setText(d)

    def run_restore(self):
        if not self.cli:
            self.append_log('未找到备份二进制文件。')
            return
        b = self.restore_backup_edit.text().strip()
        r = self.restore_to_edit.text().strip()
        key = self.restore_key.text().strip()
        if not b or not r:
            self.append_log('请同时选择备份目录和还原目标。')
            return
        args = ['restore', b, r]
        if key:
            args.extend(['-W', key])
        self.append_log('开始执行: ' + ' '.join([self.cli] + args))
        self.restore_btn.setEnabled(False)
        try:
            self.cancel_btn.setEnabled(True)
        except Exception:
            pass
        self.process.start(self.cli, args)


def main():
    app = QApplication(sys.argv)
    w = BackupGUI()
    w.show()
    sys.exit(app.exec())


if __name__ == '__main__':
    main()
