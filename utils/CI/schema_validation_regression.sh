set -euo pipefail

repo_root=$(git rev-parse --show-toplevel)
tmpdir=$(mktemp -d "${TMPDIR:-/tmp}/wesnoth-schema-test.XXXXXX")
cleanup() {
  rm -rf "$tmpdir"
}
trap cleanup EXIT

cat > "$repo_root/wesnoth" <<'EOF'
#!/usr/bin/env sh
exit 0
EOF
chmod +x "$repo_root/wesnoth"

before=$(git -C "$repo_root" status --porcelain)
/usr/bin/env bash "$repo_root/utils/CI/schema_validation.sh" || true
after=$(git -C "$repo_root" status --porcelain)

rm -f "$repo_root/wesnoth"

if [ "$before" != "$after" ]; then
  echo "repo status changed"
  git -C "$repo_root" status --porcelain
  exit 1
fi