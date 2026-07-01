#!/usr/bin/env bash
set -euo pipefail

repo_root=$(git rev-parse --show-toplevel)
test_root=$(mktemp -d "${TMPDIR:-/tmp}/wesnoth-schema-regression.XXXXXX")

cleanup() {
  git -C "$repo_root" worktree remove --force "$test_root" >/dev/null 2>&1 || true
  rm -rf "$test_root"
}
trap cleanup EXIT

git -C "$repo_root" worktree add --detach "$test_root" HEAD >/dev/null

gitdir=$(git -C "$test_root" rev-parse --git-dir)
printf 'wesnoth\n' >> "$gitdir/info/exclude"

cat > "$test_root/wesnoth" <<'EOF'
#!/usr/bin/env sh
exit 0
EOF
chmod +x "$test_root/wesnoth"

main_before=$(git -C "$repo_root" status --porcelain)
test_before=$(git -C "$test_root" status --porcelain)

(
  cd "$test_root"
  /usr/bin/env bash ./utils/CI/schema_validation.sh
)

main_after=$(git -C "$repo_root" status --porcelain)
test_after=$(git -C "$test_root" status --porcelain)

if [ "$main_before" != "$main_after" ]; then
  echo "main repository status changed"
  git -C "$repo_root" status --porcelain
  exit 1
fi

if [ "$test_before" != "$test_after" ]; then
  echo "temporary worktree status changed"
  git -C "$test_root" status --porcelain
  exit 1
fi