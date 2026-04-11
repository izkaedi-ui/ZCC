#!/bin/sh
# Fix Kali apt NO_PUBKEY ED65462EC8D5E4C5 so you can install gcc/clang.
# Run:  chmod +x fix_kali_apt.sh && ./fix_kali_apt.sh

set -e
echo "Downloading new Kali archive keyring..."
sudo wget -q https://archive.kali.org/archive-keyring.gpg -O /usr/share/keyrings/kali-archive-keyring.gpg || \
  sudo curl -fsSL https://archive.kali.org/archive-keyring.gpg -o /usr/share/keyrings/kali-archive-keyring.gpg

echo "Updating sources.list to use the keyring..."
printf '%s\n' "deb [signed-by=/usr/share/keyrings/kali-archive-keyring.gpg] http://http.kali.org/kali kali-rolling main contrib non-free non-free-firmware" | sudo tee /etc/apt/sources.list

echo "Running apt update..."
sudo apt update

echo "Done. Install a compiler with:"
echo "  sudo apt install build-essential   # gcc"
echo "  sudo apt install clang             # clang"
