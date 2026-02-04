#!/usr/bin/env bash
# Open env_gen_test.RPP (Reaper project) in the default app for .RPP files.
# Usage: ./open_env_gen_test.sh   (or: bash open_env_gen_test.sh)

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_FILE="${SCRIPT_DIR}/env_gen_test.RPP"

if [[ ! -f "$PROJECT_FILE" ]]; then
  echo "Project file not found: $PROJECT_FILE"
  exit 1
fi

case "$(uname -s)" in
  Darwin)
    open "$PROJECT_FILE"
    ;;
  Linux)
    if command -v xdg-open >/dev/null 2>&1; then
      xdg-open "$PROJECT_FILE"
    else
      echo "xdg-open not found. Open manually: $PROJECT_FILE"
      exit 1
    fi
    ;;
  MINGW*|MSYS*|CYGWIN*)
    start "$(cygpath -w "$PROJECT_FILE")"
    ;;
  *)
    echo "Unsupported OS. Open manually: $PROJECT_FILE"
    exit 1
    ;;
esac
