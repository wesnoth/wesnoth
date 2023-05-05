import path from 'path'
import Jimp from 'jimp'
import { parseWmlFile } from './wml/parser'
import rootByTagName from './wml/rootByTagName'

type ImagesGetter = {
  getTile: ({ baseCode, miscCode }: { baseCode: string, miscCode?: string }) => Jimp,
  focus: Jimp,
  flag: Jimp,
}

const getDictTerrainType2ImagesPath = async () => {
  const terratinCfgPath = path.resolve(__dirname, '../../../data/core/terrain.cfg')

  const terrain = await parseWmlFile(terratinCfgPath)

  const dictTerrainType2ImagesPath = rootByTagName(terrain)
    .terrain_type
    .reduce((acc, terrainType) => {
      if (terrainType.symbol_image) {
        acc[terrainType.string.value] = terrainType.symbol_image.value
      }
      if (!acc[terrainType.string.value] && terrainType.editor_image) {
        acc[terrainType.string.value] = terrainType.editor_image.value 
      }
      return acc
  }, {} as { [terrainType: string]: string })

  return dictTerrainType2ImagesPath
}

const readTerrainImages = async () => {
  const imageBasepath = path.resolve(__dirname, '../../../data/core/images/terrain')

  const dictTerrainType2ImagesPath = await getDictTerrainType2ImagesPath()

  const promises = Object
    .entries(dictTerrainType2ImagesPath)
    .map(async ([terrainType, imageName]) => {
      const image = await Jimp.read(`${imageBasepath}/${imageName}.png`)
      return [terrainType, image]
    })

  const result: { [baseCode: string]: Jimp } = Object.fromEntries(await Promise.all(promises))

  return result
}

const produceImagesGetter = async (): Promise<ImagesGetter> => {
  const focusPath = path.resolve(__dirname, '../../../images/editor/brush.png')
  const flagPath = path.resolve(__dirname, '../../../data/core/images/flags/flag-1.png')
  const tileFallbackPath = path.resolve(__dirname, '../../../data/core/images/misc/blank-hex.png')
  const miscFallbackPath = path.resolve(__dirname, '../../../data/core/images/misc/red-x.png')

  const [images, focus, flag, tileFallback, miscFallback] = await Promise.all([
    readTerrainImages(),
    Jimp.read(focusPath),
    Jimp.read(flagPath),
    Jimp.read(tileFallbackPath),
    Jimp.read(miscFallbackPath),
  ])

  focus.color([
    { apply: 'hue', params: [-90] },
  ])

  return {
    getTile: ({ baseCode, miscCode }) => {
      if (miscCode && images[`${baseCode}^${miscCode}`]) {
        const compoundImage = images[`${baseCode}^${miscCode}`]
        return compoundImage
      }

      let baseImage = images[baseCode]
      if (!baseImage) {
        console.warn(`Missing image for "${baseCode}". Using a fallback`)
        baseImage = tileFallback
      }

      // TODO: we should use the defaultBase correctly
      if (miscCode) {
        let miscImage = images[`^${miscCode}`]
        if (!miscImage) {
          console.warn(`Missing image for "^${miscCode}". Using a fallback`)
          miscImage = miscFallback
        }

        return baseImage.clone().composite(miscImage, 0, 0)
      }

      return baseImage
    },
    focus,
    flag,
  }
}

export { ImagesGetter }
export default produceImagesGetter
