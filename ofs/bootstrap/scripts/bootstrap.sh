#!/usr/bin/env bash
# Official OFS bootstrap entrypoint.
# bootstrap-minimal.sh remains as a compatibility target for old docs/releases.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec bash "$SCRIPT_DIR/bootstrap-minimal.sh" "$@"
