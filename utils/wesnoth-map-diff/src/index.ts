import getImages from './images'
import * as tilemap from './tilemap'
import paint from './paint'

const main = async () => {
  const [oldMapPath, newMapPath, outputFilename] = [process.argv[2], process.argv[3], process.argv[4]]

  const [oldTilemap, newTilemap, images] = await Promise.all([
    tilemap.parseFile(oldMapPath),
    tilemap.parseFile(newMapPath),
    getImages(),
  ])

  await paint(oldTilemap, newTilemap, outputFilename, images)
}

main()
