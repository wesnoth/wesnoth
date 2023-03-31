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
        console.log(`No symbol_image defined for the tile "${terrainType.string.value}"; trying editor_image next`)
        acc[terrainType.string.value] = terrainType.editor_image.value 
      }
      if (!acc[terrainType.string.value]) {
        console.log(`No image defined for the tile "${terrainType.string.value}"; falling back to default`)
        acc[terrainType.string.value] = 'grid'
      }
      return acc
  }, {} as { [terrainType: string]: string })

  return dictTerrainType2ImagesPath
}

const readTerrainImages = async () => {
  const imageBasepath = path.resolve(__dirname, '../../../data/core/images/terrain')
  const localImageBasepath = path.resolve(__dirname, './images')

  const dictTerrainType2ImagesPath = await getDictTerrainType2ImagesPath()

  const promises = Object
    .entries(dictTerrainType2ImagesPath)
    .map(async ([terrainType, imageName]) => {
      let image = await Jimp.read(`${imageBasepath}/${imageName}.png`)
      // Try one more place:
      if (!image) {
        console.log(`"${imageName}" not found in "${imageBasepath}"; trying "${localImageBasepath}" next`)
        image = await Jimp.read(`${localImageBasepath}/${imageName}.png`)
      }
      if (!image) {
        console.log(`"${imageName}" not found anywhere; falling back to a sensible default`)
        image = await Jimp.read(`../../../data/core/images/misc/blank-hex.png`)
      }
      return [terrainType, image]
    })

  const result: { [baseCode: string]: Jimp } = Object.fromEntries(await Promise.all(promises))

  return result
}

const produceImagesGetter = async (): Promise<ImagesGetter> => {
  const focusPath = path.resolve(__dirname, '../../../images/editor/brush.png')
  const flagPath = path.resolve(__dirname, '../../../data/core/images/flags/flag-1.png')

  const [images, focus, flag] = await Promise.all([
    readTerrainImages(),
    Jimp.read(focusPath),
    Jimp.read(flagPath),
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

      const baseImage = images[baseCode]
      if (!baseImage) {
        throw new Error(`Missing image for "${baseCode}"`)
      }

      // TODO: we should use the defaultBase correctly
      if (miscCode) {
        const miscImage = images[`^${miscCode}`]
        if (!miscImage) {
          throw new Error(`Missing image for "^${miscCode}"`)
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
