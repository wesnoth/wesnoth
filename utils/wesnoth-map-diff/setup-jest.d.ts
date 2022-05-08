interface CustomMatchers<R = unknown> {
  parserGot(value: unknown): R,
  parserFailedWith(expected: string[]): R,
}

declare global {
  namespace jest {
    interface Expect extends CustomMatchers {}
    interface Matchers<R> extends CustomMatchers<R> {}
    interface InverseAsymmetricMatchers extends CustomMatchers {}
  }
}

export {}
