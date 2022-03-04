import fs from 'fs'

type Tilemap = {
  baseCode: string,
  miscCode: string,
  player: string,
}[][]

const parseRawMap = (rawMap: string) => {
  return rawMap
    .slice(0, -1)
    .split('\n')
    .map(
      row =>
        row
          .split(', ')
          .map((rawTile) => {
            const [baseCode, miscCode] = rawTile.replace(/\d\s/, '').split('^')
            const [player] = /\d/.test(rawTile) ? rawTile.match(/\d/)! : []

            return { baseCode, miscCode, player }
          })
    )
}

const parseFile = (path: string) => {
  return new Promise<Tilemap>((resolve, reject) => {
    fs.readFile(path, (err, data) => {
      if (err !== null) {
        reject(err)
        return
      }

      const raw = data.toString()
      const parsed = parseRawMap(raw)
      resolve(parsed)
    })
  })
}

const size = (map: Tilemap) => ({ tilemapWidth: map[0].length, tilemapHeight: map.length })

type WalkthroughCallback =
  (
    {
      x,
      y,
      baseCode,
      miscCode,
      player,
    }: {
      x: number,
      y: number,
      baseCode: string,
      miscCode: string,
      player: string,
    }
  ) => void
const walkthrough = (tilemap: Tilemap, callback: WalkthroughCallback) => {
  let x = 0
  let y = 0

  for (let i = 0; i < tilemap.length * tilemap[0].length; i += 1) {
    const tile = tilemap[y][x]
    
    callback({
      x,
      y,
      baseCode: tile.baseCode,
      miscCode: tile.miscCode,
      player: tile.player,
    })

    x += 1
    if (x === tilemap[0].length) {
        y += 1
        x = 0
    }
  }
}

const diff = (left: Tilemap, right: Tilemap) => {
  const diffTiles: Array<[number, number]> = []

  walkthrough(left, ({ x, y }) => {
    if (
      left[y][x].baseCode !== right[y][x].baseCode
      || left[y][x].miscCode !== right[y][x].miscCode
      || left[y][x].player !== right[y][x].player
    ) {
      diffTiles.push([x, y])
    }
  })

  return diffTiles
}

export { Tilemap, parseFile, size, walkthrough, diff }
