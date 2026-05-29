#!/usr/bin/env bash
set -euo pipefail

echo "Installing build dependencies for DevilutionX (Linux)..."

sudo apt-get update
sudo apt-get install -y \
    cmake \
    g++ \
    pkg-config \
    libsdl2-dev \
    libsdl2-image-dev \
    libsodium-dev \
    libpng-dev \
    libbz2-dev \
    libfmt-dev \
    smpq \
    gettext

echo "Dependencies installed."
