#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat <<'EOF'
Usage: compare-pico8.sh [options] <cart.p8>

Run a test cart on real PICO-8 and fake-08, then compare printh output.

Options:
  --pico8 PATH       PICO-8 executable (default: $PICO8 or macOS app path)
  --fake08 PATH      fake-08 SDL2 binary (default: $FAKE08 or platform/SDL2Desktop/FAKE08)
  --frames N         Frames to run in fake-08 after _init (default: 1)
  --keep-temp        Keep captured output files on mismatch
  -h, --help         Show this help

Environment:
  PICO8              Override default PICO-8 path
  FAKE08             Override default fake-08 binary path

Exit codes:
  0  Output matches
  1  Output differs or a runner failed
  2  Invalid usage
EOF
}

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"

pico8="${PICO8:-/Applications/PICO-8.app/Contents/MacOS/pico8}"
fake08="${FAKE08:-${repo_root}/platform/SDL2Desktop/FAKE08}"
frames=1
keep_temp=0
cart=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --pico8)
            [[ $# -ge 2 ]] || { echo "missing value for --pico8" >&2; exit 2; }
            pico8="$2"
            shift 2
            ;;
        --fake08)
            [[ $# -ge 2 ]] || { echo "missing value for --fake08" >&2; exit 2; }
            fake08="$2"
            shift 2
            ;;
        --frames)
            [[ $# -ge 2 ]] || { echo "missing value for --frames" >&2; exit 2; }
            frames="$2"
            shift 2
            ;;
        --keep-temp)
            keep_temp=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            cart="${1:-}"
            break
            ;;
        -*)
            echo "unknown option: $1" >&2
            usage >&2
            exit 2
            ;;
        *)
            cart="$1"
            shift
            break
            ;;
    esac
done

if [[ -z "${cart}" ]]; then
    echo "missing cart path" >&2
    usage >&2
    exit 2
fi

if [[ ! -f "${cart}" ]]; then
    echo "cart not found: ${cart}" >&2
    exit 1
fi

cart="$(cd "$(dirname "${cart}")" && pwd)/$(basename "${cart}")"

if [[ ! -x "${pico8}" ]]; then
    echo "PICO-8 executable not found or not executable: ${pico8}" >&2
    echo "Set PICO8 or pass --pico8 PATH" >&2
    exit 1
fi

if [[ ! -x "${fake08}" ]]; then
    echo "fake-08 binary not found: ${fake08}" >&2
    echo "Build it with: make sdl2" >&2
    exit 1
fi

tmpdir="$(mktemp -d)"
trap '[[ "${keep_temp}" -eq 0 ]] && rm -rf "${tmpdir}"' EXIT

pico8_raw="${tmpdir}/pico8.raw"
pico8_out="${tmpdir}/pico8.out"
fake08_out="${tmpdir}/fake08.out"

echo "Running PICO-8: ${cart}" >&2
if ! "${pico8}" -x "${cart}" >"${pico8_raw}" 2>&1; then
    echo "PICO-8 failed:" >&2
    cat "${pico8_raw}" >&2
    exit 1
fi

# PICO-8 prints a RUNNING: banner before printh output.
grep -v '^RUNNING:' "${pico8_raw}" > "${pico8_out}" || true
if [[ ! -s "${pico8_out}" && -s "${pico8_raw}" ]]; then
    cp "${pico8_raw}" "${pico8_out}"
fi

echo "Running fake-08: ${cart} (frames=${frames})" >&2
if ! "${fake08}" -x -f "${frames}" "${cart}" >"${fake08_out}" 2>"${tmpdir}/fake08.err"; then
    echo "fake-08 failed:" >&2
    cat "${tmpdir}/fake08.err" >&2
    exit 1
fi

if diff -u "${pico8_out}" "${fake08_out}"; then
    echo "Match: PICO-8 and fake-08 output are identical." >&2
    exit 0
fi

echo "Mismatch: PICO-8 and fake-08 output differ." >&2
echo "PICO-8 output:" >&2
cat "${pico8_out}" >&2
echo "fake-08 output:" >&2
cat "${fake08_out}" >&2

if [[ "${keep_temp}" -eq 1 ]]; then
    echo "Saved outputs in ${tmpdir}" >&2
    trap - EXIT
fi

exit 1
