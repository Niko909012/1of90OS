#!/usr/bin/env bash
set -euo pipefail

ISO=iso/1of90os.iso
if [ ! -f "$ISO" ]; then
  echo "ISO not found, building..."
  make
fi

# Kill any running qemu instances started from this workspace (best effort)
pkill -f "qemu-system-i386.*$ISO" || true
sleep 0.2

echo "Starting QEMU with serial->stdio. Press Ctrl+C to stop."
qemu-system-i386 -cdrom "$ISO" -serial stdio -monitor none -m 256M -boot d -display none
