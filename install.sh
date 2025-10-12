#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
MANIFEST="$REPO_ROOT/.install-manifest"
SRC_DIR="$REPO_ROOT/src"
BIN_SRC="$SRC_DIR/bin"
LOCAL_SRC="$SRC_DIR/local"

info()  { printf '%s\n' "$*"; }
err()   { printf 'ERROR: %s\n' "$*" >&2; exit 1; }
prompt(){ printf '%s' "$*"; read -r REPLY; printf '\n'; }
yn()    { prompt "$1 [y/N]: " ; case "${REPLY:-}" in [Yy]*) return 0 ;; *) return 1 ;; esac }

XDG_BIN_HOME="${XDG_BIN_HOME:-$HOME/bin}"
XDG_DATA_HOME="${XDG_DATA_HOME:-$HOME/.local/share}"
BINDIR="$XDG_BIN_HOME"
DATADIR="$XDG_DATA_HOME"

ensure_dir() {
  local d="$1"
  [ -d "$d" ] || mkdir -p -- "$d"
}

install_file() {
  local src="$1" dst="$2" mode="${3:-644}"
  if [ ! -f "$src" ]; then
    err "Source file not found: $src"
  fi
  install -Dm"$mode" "$src" "$dst"
  INSTALLED+=("$dst")
}

map_relpath_to_dst() {
  local rel="$1"
  if [[ "$rel" == share/* ]]; then
    rel="${rel#share/}"
  fi
  if [[ "$rel" == local/* ]]; then
    rel="${rel#local/}"
  fi
  printf '%s\n' "$DATADIR/$rel"
}

do_install() {
  INSTALLED=()

  if [ -d "$BIN_SRC" ]; then
    for f in "$BIN_SRC"/*; do
      [ -f "$f" ] || continue
      dst="$BINDIR/$(basename "$f")"
      info "Installing executable: $dst"
      ensure_dir "$(dirname "$dst")"
      install_file "$f" "$dst" 755
    done
  fi

  if [ -d "$LOCAL_SRC" ]; then
    while IFS= read -r -d '' srcfile; do
      rel="${srcfile#$LOCAL_SRC/}"
      rel="${rel#./}"
      dst="$(map_relpath_to_dst "$rel")"
      info "Installing data file: $dst"
      ensure_dir "$(dirname "$dst")"
      install_file "$srcfile" "$dst" 644
    done < <(find "$LOCAL_SRC" -type f -print0)
  fi

  info ""
  info "Installed files (preview):"
  for p in "${INSTALLED[@]}"; do
    printf '  %s\n' "$p"
  done
}

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
  tmp="$(mktemp)"
  for p in "${INSTALLED[@]}"; do printf '%s\n' "$p" >> "$tmp"; done
  install -m644 "$tmp" "$MANIFEST"
  rm -f "$tmp"
  info "Manifest written to $MANIFEST"
else
  info "Manifest not saved."
  info "Manual uninstall will be required: remove the installed files listed above if you later want to uninstall."
  info "To enable uninstall.sh you can save those paths into $MANIFEST manually."
fi

info "Installation complete."
exit 0