#!/usr/bin/env bash
set -euo pipefail

# =============================================================
# build-release.sh - Build for Linux + Windows (MinGW cross)
# =============================================================

RED='\033[0;31m'; GREEN='\033[0;32m'
YELLOW='\033[1;33m'; BLUE='\033[0;34m'
NC='\033[0m'

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_LINUX="build-linux"
BUILD_WINDOWS="build-windows"
OUTPUT_DIR="release-output"
JOBS=$(nproc)
SKIP_DEPS=false
LINUX_ONLY=false
WINDOWS_ONLY=false
CLEAN=false

usage() {
	cat <<EOF
Usage: $0 [options]

Options:
  --skip-deps        Skip apt-get install (if deps already installed)
  --linux-only       Build only for Linux
  --windows-only     Build only for Windows
  --clean            Clean build dirs before starting
  --help             Show this help
EOF
	exit 0
}

while [[ $# -gt 0 ]]; do
	case "$1" in
		--skip-deps)   SKIP_DEPS=true; shift ;;
		--linux-only)  LINUX_ONLY=true; shift ;;
		--windows-only) WINDOWS_ONLY=true; shift ;;
		--clean)       CLEAN=true; shift ;;
		--help)        usage ;;
		*) echo "Unknown option: $1"; usage ;;
	esac
done

log()  { echo -e "${GREEN}[BUILD]${NC} $1"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
err()  { echo -e "${RED}[ERR]${NC} $1" >&2; exit 1; }
step() { echo -e "\n${BLUE}━━━ $1 ━━━${NC}"; }

cd "$ROOT_DIR"

if $CLEAN; then
	step "Cleaning previous builds"
	rm -rf "$BUILD_LINUX" "$BUILD_WINDOWS" "$OUTPUT_DIR"
fi

mkdir -p "$OUTPUT_DIR"

# ─── Dependencies ────────────────────────────────────────────
if ! $SKIP_DEPS; then
	if ! $WINDOWS_ONLY; then
		step "Installing Linux native dependencies"
		sudo apt-get update
		sudo apt-get install -y \
			cmake g++ pkg-config \
			libsdl2-dev libsdl2-image-dev \
			libsodium-dev libpng-dev libbz2-dev libfmt-dev \
			smpq gettext

		if ! command -v smpq &>/dev/null; then
			warn "smpq not found — devilutionx.mpq won't be built (assets from loose files)"
		fi
	fi

	if ! $LINUX_ONLY; then
		log "Windows build will use Docker (Ubuntu 22.04 + MinGW GCC 10) — skipping host MinGW deps"
	fi
fi

# ─── Linux build ─────────────────────────────────────────────
build_linux() {
	step "Building Linux (Release)"
	cmake -S. -B"$BUILD_LINUX" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCPACK=ON \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DBUILD_TESTING=OFF
	cmake --build "$BUILD_LINUX" -j"$JOBS" --target package

	step "Collecting Linux artifacts"
	mkdir -p "$OUTPUT_DIR/linux"

	pkg7z=$(find "$BUILD_LINUX/_CPack_Packages/Linux/7Z" -maxdepth 1 -type d -name 'devilutionx-*' 2>/dev/null | head -1)
	if [[ -n "$pkg7z" ]]; then
		cp "$pkg7z/bin/devilutionx" "$OUTPUT_DIR/linux/devilutionx"
		if [[ -f "$pkg7z/share/diasurgical/devilutionx/devilutionx.mpq" ]]; then
			cp "$pkg7z/share/diasurgical/devilutionx/devilutionx.mpq" "$OUTPUT_DIR/linux/"
		fi
	fi

	deb=$(find "$BUILD_LINUX" -maxdepth 1 -name 'devilutionx*.deb' 2>/dev/null | head -1)
	if [[ -n "$deb" ]]; then
		cp "$deb" "$OUTPUT_DIR/linux/devilutionx.deb"
		log "  .deb → $OUTPUT_DIR/linux/devilutionx.deb"
	fi

	if [[ ! -f "$OUTPUT_DIR/linux/devilutionx" ]]; then
		cp "$BUILD_LINUX/devilutionx" "$OUTPUT_DIR/linux/"
	fi

	log "Linux build done."
}

# ─── Windows (MinGW) build (inside Docker) ───────────────────
build_windows() {
	DOCKER_IMAGE="devilutionx-mingw:latest"

	step "Building Windows (MinGW, RelWithDebInfo) in Docker"

	if ! command -v docker &>/dev/null; then
		err "Docker is required for Windows builds. Install it: https://docs.docker.com/engine/install/"
	fi

	if ! docker image inspect "$DOCKER_IMAGE" &>/dev/null; then
		log "Building Docker image: $DOCKER_IMAGE"
		docker build -t "$DOCKER_IMAGE" -f Dockerfile.mingw .
		log "Docker image built."
	fi

	docker run --rm \
		-v "$ROOT_DIR:/work" \
		-w /work \
		--user "$(id -u):$(id -g)" \
		-e TERM="$TERM" \
		"$DOCKER_IMAGE" \
		bash -c "
			set -euo pipefail

			# Remove stale caches
			rm -f $BUILD_WINDOWS/CMakeCache.txt
			rm -rf $BUILD_WINDOWS/CMakeFiles

			# Preserve downloaded sources but purge stale build dirs
			for d in $BUILD_WINDOWS/_deps/*/; do
				base=\"\$(basename \"\$d\")\"
				case \"\$base\" in
					*-src) ;;
					*) rm -rf \"\$d\" ;;
				esac
			done

			cmake -S. -B$BUILD_WINDOWS \
				-DCMAKE_BUILD_TYPE=RelWithDebInfo \
				-DCMAKE_TOOLCHAIN_FILE=../CMake/platforms/mingwcc64.toolchain.cmake \
				-DDEVILUTIONX_SYSTEM_BZIP2=OFF \
				-DDEVILUTIONX_SYSTEM_ZLIB=OFF \
				-DDEVILUTIONX_STATIC_LIBSODIUM=ON \
				-DBUILD_TESTING=OFF

			cmake --build $BUILD_WINDOWS -j\$(nproc)
		"

	step "Collecting Windows artifacts"
	mkdir -p "$OUTPUT_DIR/windows"

	if [[ -f "$BUILD_WINDOWS/devilutionx.exe" ]]; then
		cp "$BUILD_WINDOWS/devilutionx.exe" "$OUTPUT_DIR/windows/"
		log "  devilutionx.exe → $OUTPUT_DIR/windows/"
	else
		warn "devilutionx.exe not found in build dir"
	fi

	if [[ -f "$BUILD_WINDOWS/devilutionx.mpq" ]]; then
		cp "$BUILD_WINDOWS/devilutionx.mpq" "$OUTPUT_DIR/windows/"
		log "  devilutionx.mpq → $OUTPUT_DIR/windows/"
	fi

	docker run --rm \
		-v "$ROOT_DIR:/work" \
		-v "$OUTPUT_DIR:/work/_out" \
		"$DOCKER_IMAGE" \
		bash -c "
			MINGW_DLL_DIR=/usr/x86_64-w64-mingw32/bin
			for dll in SDL2.dll libsodium-*.dll; do
				found=\$(find \"\$MINGW_DLL_DIR\" -maxdepth 1 -name \"\$dll\" 2>/dev/null | head -1)
				if [[ -n \"\$found\" ]]; then
					cp \"\$found\" /work/_out/windows/
				fi
			done
		" 2>/dev/null || warn "Could not copy DLLs from Docker image"

	log "Windows build done."
}

# ─── Run builds ──────────────────────────────────────────────

if ! $WINDOWS_ONLY; then build_linux; fi
if ! $LINUX_ONLY;  then build_windows; fi

# ─── Summary ─────────────────────────────────────────────────
step "Build complete"
echo -e "${GREEN}Output in:${NC} $OUTPUT_DIR/"
echo ""
if [[ -d "$OUTPUT_DIR/linux" ]]; then
	echo -e "  ${BLUE}Linux:${NC}"
	ls -lh "$OUTPUT_DIR/linux/" 2>/dev/null | awk '{print "    " $NF "  (" $5 ")"}'
fi
if [[ -d "$OUTPUT_DIR/windows" ]]; then
	echo -e "  ${BLUE}Windows:${NC}"
	ls -lh "$OUTPUT_DIR/windows/" 2>/dev/null | awk '{print "    " $NF "  (" $5 ")"}'
fi
echo ""
echo -e "${YELLOW}Test Linux binary:${NC} $OUTPUT_DIR/linux/devilutionx --version"
