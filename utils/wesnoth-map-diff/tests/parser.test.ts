import { testProp, fc } from 'jest-fast-check'
import { parseWml } from '../src/wml/parser'
import { wmlName, fullAttributeUnmapper, fullAttribute, fullAttributeString, fullAttributeStringUnmapper } from './arbitraries'

describe('#WML Parser', () => {
  describe('when source is empty', () => {
    it('returns empty', () => {
      const source = ''

      const parsed = parseWml(source)

      expect(parsed).parserGot([])
    })
  })

  describe('when tag is empty', () => {
    testProp('returns an empty object for the tag', [wmlName()], (tagName) => {
      const source = `
        [${tagName}]
        [/${tagName}]
      `

      const parsed = parseWml(source)
  
      expect(parsed).parserGot([[tagName, {}]])
    })
  })

  describe('when tag has one node', () => {
    describe('which has a text value', () => {
      testProp('returns an object with its attributes', [wmlName(), fullAttribute()], (tagName, attribute) => {
        const source = `
          [${tagName}]
            ${attribute}
          [/${tagName}]
        `

        const parsed = parseWml(source)
        const segments = fullAttributeUnmapper(attribute)

        expect(parsed).parserGot(
          [[tagName, { [segments.key]: { translatable: false, value: segments.value.trim() } }]]
        )
      })
    })

    describe('which has a string value', () => {
      testProp('returns an object with its attributes', [wmlName(), fullAttributeString()], (tagName, attribute) => {
        const source = `
          [${tagName}]
            ${attribute}
          [/${tagName}]
        `

        const parsed = parseWml(source)
        const segments = fullAttributeStringUnmapper(attribute)

        expect(parsed).parserGot(
          [[tagName, { [segments.key]: { translatable: false, value: segments.value } }]]
        )
      })
    })
  })

  describe('when there are two tags', () => {
    describe('with the same name', () => {
      testProp(
        'returns both tags with its attributes',
        [wmlName(), fullAttribute(), fullAttribute()],
        (tagName, firstAttribute, secondAttribute) => {
          const source = `
            [${tagName}]
              ${firstAttribute}
            [/${tagName}]
  
            [${tagName}]
              ${secondAttribute}
            [/${tagName}]
          `
  
          const parsed = parseWml(source)
          const firstSegments = fullAttributeUnmapper(firstAttribute)
          const secondSegments = fullAttributeUnmapper(secondAttribute)
  
          expect(parsed).parserGot(
            [
              [tagName, { [firstSegments.key]: { translatable: false, value: firstSegments.value.trim() } }],
              [tagName, { [secondSegments.key]: { translatable: false, value: secondSegments.value.trim() } }],
            ]
          )
        }
      )
    })

    describe('with different names', () => {
      testProp(
        'returns both tags with its attributes',
        [
          fc.tuple(wmlName(), wmlName()).filter(([first, second]) => first !== second),
          fullAttribute(),
          fullAttribute(),
        ],
        ([firstTagName, secondTagName], firstAttribute, secondAttribute) => {
          const source = `
            [${firstTagName}]
              ${firstAttribute}
            [/${firstTagName}]
  
            [${secondTagName}]
              ${secondAttribute}
            [/${secondTagName}]
          `
  
          const parsed = parseWml(source)
          const firstSegments = fullAttributeUnmapper(firstAttribute)
          const secondSegments = fullAttributeUnmapper(secondAttribute)
  
          expect(parsed).parserGot(
            [
              [firstTagName, { [firstSegments.key]: { translatable: false, value: firstSegments.value.trim() } }],
              [secondTagName, { [secondSegments.key]: { translatable: false, value: secondSegments.value.trim() } }],
            ]
          )
        }
      )
    })
  })

  describe('when tag is not closed', () => {
    describe('because of missing the the closed one', () => {
      testProp('raises an error', [wmlName(), fc.oneof(fullAttribute(), fullAttributeString(), fc.constant(''))], (tagName, tagBody) => {
        const source = `
          [${tagName}]
            ${tagBody}
        `

        const parsed = parseWml(source)

        expect(parsed).parserFailedWith([`Close tag for [${tagName}]`, 'Tag body'])
      })
    })

    describe('because of unmatched pair name', () => {
      testProp(
        'raises an error',
        [
          fc.tuple(wmlName(), wmlName()).filter(([first, second]) => first !== second),
          fc.oneof(fullAttribute(), fullAttributeString(), fc.constant('')),
        ],
        ([firstTagName, secondTagName], tagBody) => {
          const source = `
            [${firstTagName}]
              ${tagBody}
            [${secondTagName}]
          `

          const parsed = parseWml(source)

          expect(parsed).parserFailedWith([`Close tag for [${firstTagName}]`, 'Tag body'])
        }
      )
    })
  })
})
