# wesnoth-map-diff

> 🗺 Print the diff between two maps

## Setup

1 - Make sure you have Node on your system. If you don't have it, install it. If you are using macOS or Linux, you can install Node using [nvm](https://github.com/nvm-sh/nvm). On Windows, use [nvm-windows](https://github.com/coreybutler/nvm-windows).

2 - Open the terminal on `utils/wesnoth-map-diff` folder and run the following command to install the project dependencies:

```
npm i
```

3 - Then, run the following command to build it:

```
npm run build:dev
```

4 - Finally, run it using `node ./build/index.js [path for the old map] [path for the new map] [output filename]`. For instance:

```
node ./build/index.js old.map new.map output.png
```

## Contributing

Run tests:

```
npm run test
```

Run lint:

```
npm run lint
```
