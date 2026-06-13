#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$repo_root"

if ! command -v clang-format >/dev/null 2>&1; then
    echo "error: clang-format not found in PATH" >&2
    exit 1
fi

files=()
while IFS= read -r file; do
    files+=("$file")
done < <(
    git ls-files 'source/*.cpp' 'source/*.h' \
        'test/*.cpp' 'test/*.h' \
        'platform/**/*.cpp' 'platform/**/*.h' \
        | grep -v '^test/doctest\.h$'
)

if [ "${#files[@]}" -eq 0 ]; then
    echo "error: no first-party C++ files found" >&2
    exit 1
fi

mode="${1:-check}"
case "$mode" in
    check)
        clang-format --dry-run --Werror "${files[@]}"
        echo "clang-format: all ${#files[@]} first-party files are formatted"
        ;;
    format)
        clang-format -i "${files[@]}"
        echo "clang-format: formatted ${#files[@]} first-party files"
        ;;
    *)
        echo "usage: $0 [check|format]" >&2
        exit 1
        ;;
esac
