import type { WmlParsed } from './parser'

const rootByTagName = (parsed: WmlParsed) => {
  return parsed.reduce((acc, [tagName, tagAttrs]) => {
    if (acc[tagName] === undefined) {
      acc[tagName] = []
    }

    acc[tagName].push(tagAttrs)
    return acc
  }, {} as { [tagName: string]: Array<{ [attributeName: string]: { translatable: boolean, value: string } }> })
}

export default rootByTagName
