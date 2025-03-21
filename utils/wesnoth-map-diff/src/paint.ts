import path from 'path'
import Jimp from 'jimp'
import * as tilemap from './tilemap'
import type { Tile, Tilemap } from './tilemap'
import type { ImagesGetter } from './images'

type Side = 'left' | 'right'

const tileImageSize = 72
const axisMargin = 80
const maxLinesOnNotes = 15

const imageSize = (map: Tilemap, totalDiffs: number) => {
  const { tilemapWidth, tilemapHeight } = tilemap.size(map)

  const imageHeight =
    axisMargin
    + totalDiffs * 36
    + tilemapHeight * tileImageSize

  const imageWidth =
    axisMargin
    + tileImageSize * tilemapWidth
    - (tilemapWidth - 1) * 18

  return { width: imageWidth, height: imageHeight }
}

const getTileImageCoordenates = (tileX: number, tileY: number) => {
  const imageX = tileX * tileImageSize - 18 * tileX
  const imageY = tileX % 2 === 0
    ? tileY * tileImageSize
    : tileY * tileImageSize - tileImageSize / 2

  return [axisMargin + imageX, axisMargin + imageY]
}

const producePainters = (output: Jimp, images: ImagesGetter, leftPadding: number) => {
  const paintTile = (x: number, y: number, baseCode: string, miscCode?: string) => {
    const tileImage = images.getTile({ baseCode, miscCode })
    output.composite(tileImage, x + leftPadding, y)
  }

  // todo: the flag color should follow the player number
  const paintPlayer = (x: number, y: number, player?: string) => {
    if (!player) {
      return
    }

    output.composite(images.flag, x + leftPadding, y)
  }

  return { paintTile, paintPlayer }
}

const paint = async (
  oldTilemap: Tilemap,
  newTilemap: Tilemap,
  outputFilename: string,
  images: ImagesGetter
) => {
  const diffs = tilemap.diff(oldTilemap, newTilemap)

  const { height, width } = imageSize(oldTilemap, Math.min(diffs.length, maxLinesOnNotes))

  const diffImageWidth = width * 2 + tileImageSize
  const diffImageHeight = height

  new Jimp(diffImageWidth, diffImageHeight, async (_err, output) => {
    output.opaque()

    const fontPath = path.resolve(__dirname, '../font/DejaVuSansMono.fnt')
    const font = await Jimp.loadFont(fontPath)

    const paintAxis = (map: Tilemap, side: Side) => {
      const leftPadding = side === 'left'
        ? 0
        : width + axisMargin

      const { tilemapWidth, tilemapHeight } = tilemap.size(map)

      for (let i = 0; i < tilemapWidth; i++) {
        const [imageX] = getTileImageCoordenates(i, 0)

        output.print(font, leftPadding + imageX, 0, i)
      }

      for (let i = 0; i < tilemapHeight; i++) {
        const [, imageY] = getTileImageCoordenates(0, i)

        output.print(font, leftPadding, imageY, i)
      }
    }

    const paintNotes = () => {
      const getTileCode = (tile: Tile) => {
        const player = tile.player ? `${tile.player} ` : ''
        const miscCode = tile.miscCode ? `^${tile.miscCode}` : ''

        return `${player}${tile.baseCode}${miscCode}`
      }

      const getLineY = (line: number) => (diffImageHeight - slicedDiffs.length * 36) + line * 36

      const slicedDiffs = diffs.slice(0, maxLinesOnNotes - 1)

      slicedDiffs.forEach(([x, y], i) => {
        const previousTile = oldTilemap[y][x]
        const newTile = newTilemap[y][x]

        const xString = `${x}`.padStart(2, '0')
        const yString = `${y}`.padStart(2, '0')
        const diffLine = `[${xString}:${yString}] ${getTileCode(previousTile).padStart(8)} -> ${getTileCode(newTile)}`

        output.print(font, 0, getLineY(i), diffLine)
      })

      if (diffs.length !== slicedDiffs.length) {
        output.print(font, 0, getLineY(maxLinesOnNotes - 1), '... and other changes')
      }
    }

    const paintTilemap = (map: Tilemap, side: Side) => {
      const leftPadding = side === 'left'
        ? 0
        : width + tileImageSize

      const { paintTile, paintPlayer } = producePainters(output, images, leftPadding)

      tilemap.walkthrough(map, ({ x, y, baseCode, miscCode, player }) => {
        const [imageX, imageY] = getTileImageCoordenates(x, y)

        paintTile(imageX, imageY, baseCode, miscCode)
        paintPlayer(imageX, imageY, player)
      })

      diffs.forEach(([x, y]) => {
        const [imageX, imageY] = getTileImageCoordenates(x, y)

        output.composite(images.focus, imageX + leftPadding, imageY)
      })
    }

    paintAxis(oldTilemap, 'left')
    paintAxis(oldTilemap, 'right')
    paintNotes()

    paintTilemap(oldTilemap, 'left')
    paintTilemap(newTilemap, 'right')

    output.write(outputFilename)
  })
}

export default paint
