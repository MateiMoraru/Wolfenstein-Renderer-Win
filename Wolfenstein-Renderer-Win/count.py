import os
from pathlib import Path

c_files = h_files = 0
c_lines = h_lines = 0

for file in Path('.').rglob('*'):
    if file.suffix == '.c':
        c_files += 1
        with open(file, 'r', encoding='utf-8', errors='ignore') as f:
            c_lines += len(f.readlines())
        print(f"C: {file}")
    elif file.suffix == '.h':
        h_files += 1
        with open(file, 'r', encoding='utf-8', errors='ignore') as f:
            h_lines += len(f.readlines())
        print(f"H: {file}")

print(f"\n.C: {c_files} files, {c_lines} lines")
print(f".H: {h_files} files, {h_lines} lines")
print(f"Total: {c_files + h_files} files, {c_lines + h_lines} lines")