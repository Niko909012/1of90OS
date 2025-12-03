#!/bin/sh
# Create a bootable ISO using grub-mkrescue or xorriso as fallback.
# Usage: mkiso.sh output.iso iso-dir

OUT="$1"
ISO_DIR="$2"

if command -v grub-mkrescue >/dev/null 2>&1; then
    # grub-mkrescue sometimes returns non-zero even if xorriso created the image
    if grub-mkrescue -o "$OUT" "$ISO_DIR"; then
        exit 0
    else
        if [ -s "$OUT" ]; then
            echo "grub-mkrescue returned non-zero, but $OUT exists — continuing"
            exit 0
        else
            echo "grub-mkrescue failed and did not produce $OUT" >&2
            exit 1
        fi
    fi
elif command -v xorriso >/dev/null 2>&1; then
    xorriso -as mkisofs -R -J -l -o "$OUT" "$ISO_DIR"
else
    echo "install grub-mkrescue or xorriso to create ISO" >&2
    exit 1
fi
