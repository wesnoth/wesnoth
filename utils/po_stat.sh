#!/bin/sh

set -euo pipefail

# Mac fallback
which realpath >/dev/null || realpath() {
	python -c "import os; print(os.path.realpath('$1'))"
}

WESNOTH_SRC_DIR=$(realpath "$(dirname $0)/..")
DA_LANGS=$(cat ${WESNOTH_SRC_DIR}/po/LINGUAS)

for FILE in ${DA_LANGS}; do
	DA_LANG=$(basename ${FILE%.*})
	python ${WESNOTH_SRC_DIR}/utils/po_stat.py ${DA_LANG} ${WESNOTH_SRC_DIR}/data/languages
done
