expect.extend({
  parserGot (received, value) {
    try {
      expect(received).toMatchObject({
        status: true,
        value,
      })

      return {
        pass: true,
        message: () => `Expects for ${JSON.stringify(value)}`,
      }
    } catch (e) {
      return {
        pass: false,
        message: () => e.matcherResult.message,
      }
    }
  },

  parserFailedWith (received, expected) {
    try {
      expect(received).toMatchObject({
        status: false,
        expected,
      })

      return {
        pass: true,
        message: () => `Expects for ${JSON.stringify(expected)}`,
      }
    } catch (e) {
      return {
        pass: false,
        message: () => e.matcherResult.message,
      }
    }
  },
})
