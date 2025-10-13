#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MANIFEST="$REPO_ROOT/.install-manifest"

info()  { printf '%s\n' "$*"; }
err()   { printf 'ERROR: %s\n' "$*" >&2; exit 1; }
prompt(){ printf '%s' "$1"; read -r REPLY; }
yn()    { prompt "$1 [y/N]: " ; case "${REPLY:-}" in [Yy]*) return 0 ;; *) return 1 ;; esac }

[ -f "$MANIFEST" ] || err "Manifest not found at $MANIFEST; cannot proceed. If you did not save a manifest you must remove installed files manually."

mapfile -t PATHS < "$MANIFEST"

HOME="${HOME:-$LOGNAME}"
USER_BIN="$HOME/bin"
USER_SHARE="$HOME/.local/share"
USER_APPS="$USER_SHARE/applications"

info "This uninstall script removes user-local files listed in $MANIFEST."
yn "Continue?" || err "Aborted by user."

removed=0

trim() {
  local s="$1"
  s="${s#"${s%%[![:space:]]*}"}"
  s="${s%"${s##*[![:space:]]}"}"
  printf '%s' "$s"
}

is_user_local() {
  local p="$1"
  case "$p" in
    "$USER_BIN/"* | "$USER_APPS/"* | "$USER_SHARE/"* | "$HOME/"*) return 0 ;;
    *) return 1 ;;
  esac
}

prune_empty_parents() {
  local dir="$1"
  while [ -n "$dir" ] && [ "$dir" != "$HOME" ] && [ "$dir" != "/" ] && [ -d "$dir" ]; do
    if [ -z "$(ls -A -- "$dir" 2>/dev/null)" ]; then
      rmdir -- "$dir" || break
      dir="$(dirname -- "$dir")"
    else
      break
    fi
  done
}

for raw in "${PATHS[@]}"; do
  p="$(trim "$raw")"
  [ -z "$p" ] && continue

  if is_user_local "$p"; then
    if [ -f "$p" ] || [ -L "$p" ]; then
      info "Removing: $p"
      rm -f -- "$p"
      removed=$((removed+1))
    else
      info "Not present, skipping: $p"
    fi
    prune_empty_parents "$(dirname -- "$p")"
  else
    info "Skipping non-user-local path: $p"
  fi
done

if [ "$removed" -gt 0 ]; then
  info "Removed $removed file(s)."
else
  info "No user-local files were removed."
fi

info "Removing manifest: $MANIFEST"
rm -f -- "$MANIFEST"

info "Uninstall complete."
exit 0
