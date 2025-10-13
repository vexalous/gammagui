#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MANIFEST="$REPO_ROOT/.install-manifest"
SRC_DIR="$REPO_ROOT/src"
BIN_SRC="$SRC_DIR/bin"
LOCAL_SRC="$SRC_DIR/local"

XDG_BIN_HOME="${XDG_BIN_HOME:-$HOME/bin}"
XDG_DATA_HOME="${XDG_DATA_HOME:-$HOME/.local/share}"
BINDIR="$XDG_BIN_HOME"
DATADIR="$XDG_DATA_HOME"

info()  { printf '%s\n' "$*"; }
err()   { printf 'ERROR: %s\n' "$*" >&2; exit 1; }
prompt(){ printf '%s' "$1"; read -r REPLY; }
yn()    { prompt "$1 [y/N]: " ; case "${REPLY:-}" in [Yy]*) return 0 ;; *) return 1 ;; esac }

ensure_dir() { local d="$1"; [ -d "$d" ] || mkdir -p -- "$d"; }

install_file() {
  local src="$1" dst="$2" mode="${3:-644}"
  [ -f "$src" ] || err "Source file not found: $src"
  install -Dm"$mode" -- "$src" "$dst"
  INSTALLED+=("$dst")
}

map_relpath_to_dst() {
  local rel="$1"
  rel="${rel#share/}"
  rel="${rel#local/}"
  printf '%s\n' "$DATADIR/$rel"
}

do_install() {
  INSTALLED=()

  if [ -d "$BIN_SRC" ]; then
    ensure_dir "$BINDIR"
    while IFS= read -r -d '' f; do
      [ -f "$f" ] || continue
      dst="$BINDIR/$(basename "$f")"
      info "Installing executable: $dst"
      install_file "$f" "$dst" 755
    done < <(find "$BIN_SRC" -maxdepth 1 -type f -print0)
  fi

  if [ -d "$LOCAL_SRC" ]; then
    while IFS= read -r -d '' srcfile; do
      rel="${srcfile#$LOCAL_SRC/}"
      dst="$(map_relpath_to_dst "$rel")"
      info "Installing data file: $dst"
      ensure_dir "$(dirname "$dst")"
      install_file "$srcfile" "$dst" 644
    done < <(find "$LOCAL_SRC" -type f -print0)
  fi

  info ""
  info "Installed files (preview):"
  for p in "${INSTALLED[@]:-}"; do printf '  %s\n' "$p"; done
}

cleanup_tmp() {
  [ -n "${TMPFILE:-}" ] && [ -f "$TMPFILE" ] && rm -f -- "$TMPFILE" || :
}
trap cleanup_tmp EXIT

info "This installer performs a user-local install to:"
info "  binaries -> $BINDIR"
info "  data files -> $DATADIR"
yn "Continue?" || err "Aborted by user."

if [ -f "$MANIFEST" ]; then
  info ""
  info "A previous .install-manifest exists at $MANIFEST."
  if yn "Uninstall the previous manifest-based install before continuing?"; then
    if [ -x "$REPO_ROOT/uninstall.sh" ]; then
      info "Running uninstall.sh (user-local) from repo root..."
      "$REPO_ROOT/uninstall.sh" || err "uninstall.sh failed; aborting."
    else
      err "uninstall.sh not found or not executable; aborting to avoid conflicts."
    fi
  fi
fi

do_install

info ""
if yn "Save an install manifest to $MANIFEST for future automatic uninstall?"; then
  TMPFILE="$(mktemp)" || err "mktemp failed"
  for p in "${INSTALLED[@]:-}"; do printf '%s\n' "$p" >> "$TMPFILE"; done
  install -m644 -- "$TMPFILE" "$MANIFEST"
  info "Manifest written to $MANIFEST"
else
  info "Manifest not saved."
  info "Manual uninstall will be required: remove the installed files listed above if you later want to uninstall."
fi

info "Installation complete."
exit 0
