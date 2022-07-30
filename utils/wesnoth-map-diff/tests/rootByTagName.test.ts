import { testProp, fc } from 'jest-fast-check'
import type { WmlParsed } from '../src/wml/parser'
import rootByTagName from '../src/wml/rootByTagName'
import { wmlName } from './arbitraries'

describe('#RootByTagName', () => {
  describe('when there is no tags', () => {
    it('returns empty', () => {
      const parsed: WmlParsed = []

      const rooted = rootByTagName(parsed)

      expect(rooted).toStrictEqual({})
    })
  })

  describe('when has only one tag', () => {
    testProp(
      'includes it on the rooted object',
      [
        wmlName(),
        fc.dictionary(wmlName(), fc.record({ translatable: fc.boolean(), value: fc.string() })),
      ],
      (tagName, attributes) => {
        const parsed: WmlParsed = [
          [tagName, attributes],
        ]

        const rooted = rootByTagName(parsed)

        expect(rooted).toStrictEqual({
          [tagName]: [attributes],
        })
      }
    )
  })

  describe('when has two tags', () => {
    describe('with the same name', () => {
      testProp(
        'merge it on the rooted object',
        [
          wmlName(),
          fc.dictionary(wmlName(), fc.record({ translatable: fc.boolean(), value: fc.string() })),
          fc.dictionary(wmlName(), fc.record({ translatable: fc.boolean(), value: fc.string() })),
        ],
        (tagName, firstAttributes, secondAttributes) => {
          const parsed: WmlParsed = [
            [tagName, firstAttributes],
            [tagName, secondAttributes],
          ]
  
          const rooted = rootByTagName(parsed)
  
          expect(rooted).toStrictEqual({
            [tagName]: [firstAttributes, secondAttributes],
          })
        }
      )
    })

    describe('with different names', () => {
      testProp(
        'includes both on the rooted object',
        [
          fc.tuple(wmlName(), wmlName()).filter(([first, second]) => first !== second),
          fc.dictionary(wmlName(), fc.record({ translatable: fc.boolean(), value: fc.string() })),
          fc.dictionary(wmlName(), fc.record({ translatable: fc.boolean(), value: fc.string() })),
        ],
        ([firstTagName, secondTagName], firstAttributes, secondAttributes) => {
          const parsed: WmlParsed = [
            [firstTagName, firstAttributes],
            [secondTagName, secondAttributes],
          ]
  
          const rooted = rootByTagName(parsed)
  
          expect(rooted).toStrictEqual({
            [firstTagName]: [firstAttributes],
            [secondTagName]: [secondAttributes],
          })
        }
      )
    })
  })
})
