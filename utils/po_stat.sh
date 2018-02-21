cd ../po/
#LANGS=$(ls ../data/languages/*.cfg)
DA_LANGS=$(cat LINGUAS)
WML_TAG=translation_stats

echo "[${WML_TAG}]"

for FILE in ${DA_LANGS}; do
	DA_LANG=$(basename ${FILE%.*})
	python3 ../utils/po_stat.py ${DA_LANG}
done

echo "[/${WML_TAG}]"