import * as fc from 'fast-check'

const keyChars = [
  ...Array.from(new Array(10), (_, index) => String.fromCharCode(48 + index)), // 0-9
  ...Array.from(new Array(24), (_, index) => String.fromCharCode(65 + index)), // A-Z
  ...Array.from(new Array(24), (_, index) => String.fromCharCode(97 + index)), // a-z
  '_',
]

const valueChars = [
  ...keyChars, '/', '-', '!', '@', '$', '%', '^', '&', '*', '(', ')', '[', ']', ' ',
]

const wmlName = () => fc.stringOf(
  fc.constantFrom(...keyChars), { minLength: 1 }
)

const wmlValue = () => fc.stringOf(
  fc.constantFrom(...valueChars), { minLength: 1 }
).filter((value) => (value.trimStart().length > 0) && (value.includes('_ ') === false))

const wmlValueString = () => fc.stringOf(
  fc.constantFrom(...valueChars, '\n', '"'), { minLength: 1 }
).filter((value) => (value.trimStart().length > 0) && (value.includes('_ ') === false)).map((value) => value.replace(/"/g, '""test quote""'))

const wmlValueTranslatable = (): fc.Arbitrary<string> => {
  return fc.convertFromNext(
    fc.convertToNext(wmlValue()).map(
      (t) => `_ ${t}`,
      (t) => (t as string).replace('_ ', '')
    )
  )
}

const fullAttributeUnmapper = (fullAttribute: string) => {
  const [key, keyPadding, valuePadding, value] = fullAttribute.split(/(\s*)=(\s*)/)
  return { key, keyPadding: keyPadding.length, valuePadding: valuePadding.length, value }
}

const fullAttributeStringUnmapper = (fullAttributeString: string) => {
  const [key, keyPadding, valuePadding, rawValue] = fullAttributeString.split(/(\s*)=(\s*)/)

  const value = rawValue.replace(/^"/, '').replace(/"$/, '')

  return { key, keyPadding: keyPadding.length, valuePadding: valuePadding.length, value }
}

const fullAttribute = (): fc.Arbitrary<string> => {
  return fc.convertFromNext(
    fc.convertToNext(
      fc.tuple(
        wmlName(),
        fc.nat({ max: 5 }),
        fc.nat({ max: 5 }),
        wmlValue()
      )
    ).map(
      ([key, keyPadding, valuePadding, value]) => `${key}${' '.repeat(keyPadding)}=${' '.repeat(valuePadding)}${value}`,
      (attribute) => {
        if (typeof attribute !== 'string') {
          throw new Error('Invalid type')
        }

        const segments = fullAttributeUnmapper(attribute)
        return [segments.key, segments.keyPadding, segments.valuePadding, segments.value]
      }
    )
  )
}

const fullAttributeString = (): fc.Arbitrary<string> => {
  return fc.convertFromNext(
    fc.convertToNext(
      fc.tuple(
        wmlName(),
        fc.nat({ max: 5 }),
        fc.nat({ max: 5 }),
        wmlValueString()
      )
    ).map(
      ([key, keyPadding, valuePadding, value]) => `${key}${' '.repeat(keyPadding)}=${' '.repeat(valuePadding)}"${value}"`,
      (attribute) => {
        if (typeof attribute !== 'string') {
          throw new Error('Invalid type')
        }

        const segments = fullAttributeStringUnmapper(attribute)
        return [segments.key, segments.keyPadding, segments.valuePadding, segments.value]
      }
    )
  )
}

export {
  wmlName,
  wmlValue,
  wmlValueTranslatable,
  fullAttributeUnmapper,
  fullAttribute,
  fullAttributeString,
  fullAttributeStringUnmapper,
}
