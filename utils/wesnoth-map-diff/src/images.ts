import path from 'path'
import Jimp from 'jimp'
import { parseWmlFile } from './wml/parser'
import rootByTagName from './wml/rootByTagName'

type ImagesDict = { tile: { [baseCode: string]: Jimp }, focus: Jimp, flag: Jimp }

const getDictTerrainType2ImagesPath = async () => {
  const terratinCfgPath = path.resolve(__dirname, '../../../data/core/terrain.cfg')

  const terrain = await parseWmlFile(terratinCfgPath)

  const dictTerrainType2ImagesPath = rootByTagName(terrain)
    .terrain_type
    .reduce((acc, terrainType) => {
      if (terrainType.symbol_image === undefined) {
        return acc
      }

      acc[terrainType.string.value] = terrainType.symbol_image.value
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

  const result = Object.fromEntries(await Promise.all(promises))

  return result
}

const getImages = async (): Promise<ImagesDict> => {
  const focusPath = path.resolve(__dirname, '../../../images/editor/brush.png')
  const flagPath = path.resolve(__dirname, '../../../data/core/images/flags/flag-1.png')

  const mapTileCodeToImage = readTerrainImages()
  const focus = Jimp.read(focusPath)
  const flag = Jimp.read(flagPath)

  return {
    tile: await mapTileCodeToImage,
    focus: await focus,
    flag: await flag,
  }
}

export { ImagesDict }
export default getImages
