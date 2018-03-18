VERSION=1.13.12
IN_FILENAME="Wesnoth_${VERSION} rw.dmg"
OUT_FILENAME="Wesnoth_${VERSION}.dmg"
VOLUME="/Volumes/Wesnoth ${VERSION}"
CODESIGN_IDENTITY='Wesnoth, Inc'

set -euo pipefail

echo "Mounting"
hdiutil detach "${VOLUME}" || true
hdiutil attach "${IN_FILENAME}"

echo "Signing app"
# https://stackoverflow.com/questions/29724720/unable-to-sign-my-app-with-codesign
codesign --deep --sign "Developer ID Application: $CODESIGN_IDENTITY" -v "${VOLUME}/Wesnoth.app"

#echo "Setting icon"
#cp "${VOLUME}/Wesnoth.app/Contents/Resources/icon.icns" .
#mv icon.icns "${VOLUME}/.VolumeIcon.icns"

hdiutil detach "${VOLUME}"

echo "Converting to R/O DMG"
rm -f "${OUT_FILENAME}"
hdiutil convert "${IN_FILENAME}" -format UDBZ -o "${OUT_FILENAME}"

#echo "Signing DMG" - we cannot sign DMG and don't have to. 
#codesign --force --sign "Developer ID Installer: $CODESIGN_IDENTITY" "${OUT_FILENAME}" "${OUT_FILENAME}.signed"

shasum -a 256 "${OUT_FILENAME}" > "${OUT_FILENAME}.sha256"
