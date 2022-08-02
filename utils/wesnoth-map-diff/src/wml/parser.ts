import { createLanguage, seqMap, alt, regex, string, whitespace, Result } from 'parsimmon'
import fs from 'fs'

const WmlLang = createLanguage({
  File: (r) => {
    return r
      .TopLevel
      .many()
      .map((nodes) => {
        const tagsList = nodes.filter(node => typeof node !== 'string')
        return tagsList
      })
  },

  TopLevel: (r) => {
    return alt(whitespace, r.Comment, r.FullTag)
  },

  Comment: () => {
    return regex(/\s*#.*\n/)
  },

  FullTag: (r) => {
    return r
      .OpenTag
      .chain((tagName) => {
        return alt(
          whitespace,
          r.FullAttribute,
          r.Comment
        )
        .desc('Tag body')
        .many()
        .map((lines) => {
          const attrsArray = lines.filter(line => typeof line !== 'string')
          const attrsObj = attrsArray.reduce((acc, attr) => {
            acc[attr.key] = attr.value
            return acc
          }, {})
          return [tagName, attrsObj]
        })
        .skip(
          string(`[/${tagName}]`)
          .desc(`Close tag for [${tagName}]`)
        )
      })
  },

  OpenTag: () => {
    return seqMap(
      string('['),
      regex(/[a-zA-Z0-9_]+/),
      string(']'),
      (_, tagName) => tagName
    )
  },

  FullAttribute: (r) => {
    return seqMap(
      r.AttributeKey,
      r.AttributeValue,
      (key, value) => (
        { key, value }
      )
    )
  },

  AttributeKey: () => {
    return seqMap(
      regex(/[a-zA-Z0-9_]+/),
      regex(/\s*=\s*/),
      (key) => key
    )
  },

  AttributeValue: (r) => {
    return alt(
      r.AttributeValueString,
      r.AttributeValueText
    )
  },

  AttributeValueText: () => {
    return seqMap(
      regex(/(_ )?/),
      regex(/[^+\n]*/),
      (translatable, value) => (
        {
          value: value.trim(),
          translatable: Boolean(translatable),
        }
      )
    )
  },

  AttributeValueString: () => {
    return seqMap(
      regex(/(_ )?/),
      string('"'),
      regex(/([^"]|"")*/),
      string('"'),
      (translatable, _, value) => (
        {
          value,
          translatable: Boolean(translatable),
        }
      )
    )
  },
})

type WmlParsed = Array<
  [
    string,
    { [attributeName: string]: { translatable: boolean, value: string } }
  ]
>

const parseWml = (source: string) =>
  WmlLang.File.parse(source) as Result<WmlParsed>

const tryParseWml = (source: string) =>
  WmlLang.File.tryParse(source) as WmlParsed

const parseWmlFile = (path: string) => {
  return new Promise<WmlParsed>((resolve, reject) => {
    fs.readFile(path, (err, data) => {
      if (err !== null) {
        reject(err)
        return
      }

      const rawTerrainCfg = data.toString()
      const parsed = tryParseWml(rawTerrainCfg)
      resolve(parsed)
    })
  })
}

export { WmlParsed, parseWml, tryParseWml, parseWmlFile }
