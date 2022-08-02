import Jimp from 'jimp'
import * as tilemap from './tilemap'
import type { Tilemap } from './tilemap'
import type { ImagesDict } from './images'

type Side = 'left' | 'right'

const tileImageSize = 72

const imageSize = (map: Tilemap) => {
  const { tilemapWidth, tilemapHeight } = tilemap.size(map)

  const imageHeight = tilemapHeight * tileImageSize
  const imageWidth = tileImageSize * tilemapWidth - (tilemapWidth - 1) * 18

  return { width: imageWidth, height: imageHeight }
}

const getTileImageCoordenates = (tileX: number, tileY: number) => {
  const imageX = tileX * tileImageSize - 18 * tileX
  const imageY = tileX % 2 === 0
    ? tileY * tileImageSize
    : tileY * tileImageSize - tileImageSize / 2

  return [imageX, imageY]
}

const producePainters = (output: Jimp, images: ImagesDict, leftPadding: number) => {
  const paintTile = (x: number, y: number, baseCode: string) => {
    const baseImage = images.tile[baseCode]
    output.composite(baseImage, x + leftPadding, y)
  }

  // todo: we should use the defaultBase correctly
  const paintMisc = (x: number, y: number, baseCode: string, miscCode: string) => {
    if (!miscCode) {
      return
    }

    const misctileImage = images.tile[`${baseCode}^${miscCode}`] || images.tile[`^${miscCode}`]
    output.composite(misctileImage, x + leftPadding, y)
  }

  // todo: the flag color should follow the player number
  const paintPlayer = (x: number, y: number, player: string) => {
    if (!player) {
      return
    }

    output.composite(images.flag, x + leftPadding, y)
  }

  return { paintTile, paintMisc, paintPlayer }
}

const paint = async (
  oldTilemap: Tilemap,
  newTilemap: Tilemap,
  outputFilename: string,
  images: ImagesDict
) => {
  const { height, width } = imageSize(oldTilemap)

  const diffImageWidth = width * 2 + tileImageSize
  const diffImageHeight = height

  new Jimp(diffImageWidth, diffImageHeight, (_err, output) => {
    const paintTilemap = (map: Tilemap, side: Side) => {  
      const leftPadding = side === 'left'
        ? 0
        : width + tileImageSize

      const { paintTile, paintMisc, paintPlayer } = producePainters(output, images, leftPadding)

      tilemap.walkthrough(map, ({ x, y, baseCode, miscCode, player }) => {
        const [imageX, imageY] = getTileImageCoordenates(x, y)

        paintTile(imageX, imageY, baseCode)
        paintMisc(imageX, imageY, baseCode, miscCode)
        paintPlayer(imageX, imageY, player)
      })

      tilemap.diff(oldTilemap, newTilemap).forEach(([x, y]) => {
        const [imageX, imageY] = getTileImageCoordenates(x, y)

        output.composite(images.focus, imageX + leftPadding, imageY)
      })
    }

    paintTilemap(oldTilemap, 'left')
    paintTilemap(newTilemap, 'right')

    output.write(outputFilename)
  })
}

export default paint
