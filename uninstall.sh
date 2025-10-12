#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
MANIFEST="$REPO_ROOT/.install-manifest"

info()  { printf '%s\n' "$*"; }
err()   { printf 'ERROR: %s\n' "$*" >&2; exit 1; }
prompt(){ printf '%s' "$*"; read -r REPLY; printf '\n'; }
yn()    { prompt "$1 [y/N]: " ; case "${REPLY:-}" in [Yy]*) return 0 ;; *) return 1 ;; esac }

[ -f "$MANIFEST" ] || err "Manifest not found at $MANIFEST; cannot proceed. If you did not save a manifest you must remove installed files manually."

mapfile -t PATHS < "$MANIFEST"

USER_BIN="$HOME/bin"
USER_SHARE="$HOME/.local/share"
USER_APPS="$USER_SHARE/applications"

info "This uninstall script removes user-local files listed in $MANIFEST."
if ! yn "Continue?"; then
  err "Aborted by user."
fi

removed=0

for p in "${PATHS[@]}"; do
  p="${p%"${p##*[![:space:]]}"}"
  p="${p#"${p%%[![:space:]]*}"}"
  [ -z "$p" ] && continue

  if [[ "$p" == "$USER_BIN/"* || "$p" == "$USER_APPS/"* || "$p" == "$USER_SHARE/"* || "$p" == "$HOME/"* ]]; then
    if [[ -f "$p" || -L "$p" ]]; then
      info "Removing: $p"
      rm -f -- "$p"
      removed=$((removed+1))
    else
      info "Not present, skipping: $p"
    fi
  else
    info "Skipping non-user-local path: $p"
  fi
done

prune() {
  local dir="$1"
  while [[ "$dir" != "$HOME" && "$dir" != "/" && -d "$dir" && -z "$(ls -A "$dir" 2>/dev/null)" ]]; do
    rmdir -- "$dir" || break
    dir="$(dirname "$dir")"
  done
}

for p in "${PATHS[@]}"; do
  p="${p%"${p##*[![:space:]]}"}"
  p="${p#"${p%%[![:space:]]*}"}"
  [ -z "$p" ] && continue
  if [[ "$p" == "$USER_BIN/"* || "$p" == "$USER_APPS/"* || "$p" == "$USER_SHARE/"* || "$p" == "$HOME/"* ]]; then
    prune "$(dirname "$p")"
  fi
done

if [[ $removed -gt 0 ]]; then
  info "Removed $removed file(s)."
else
  info "No user-local files were removed."
fi

info "Removing manifest: $MANIFEST"
rm -f -- "$MANIFEST"

info "Uninstall complete."
exit 0