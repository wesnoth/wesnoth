This document lists some general guideline on taking the screenshots for the
manual. Please adopt it to the correct version of the time you are taking the
screenshot and make sure to point the entry in the .po file to the correct
screenshot.

The examples we give are based on the availability of the [ImageMagick](https://www.imagemagick.org/script/index.php) tools
used for converting the images to the JPEG format.


Here is a list of the general howto for the screenshots:

1) Take the "original" version as PNG or BMP.

2) Take the fullscreen screenshots in 1024x768.

3) Afterwards, convert the fullscreen screenshots to JPEG and change the
resolution to 640x480 and set quality level to 80%. This is the command to use
when using ImageMagick (change file name accordingly for the titlescreen
screenshot):
```sh
convert -resize 640x480 -quality 80 game-screen-1.13.11+dev.bmp game-screen-1.13.11+dev.jpg
```

4) For those "parts of the screen" screenshots just copy out the relevant part
from the full versions. All "non fullscreen" screenshots should be stored as
PNG and in the original size. The ImageMagick command would look like this:
```sh
convert multiplayer-1.13.11+dev.bmp multiplayer-1.13.11+dev.png
```

5) Send the files in once you translated the strings in the po file. In the
translation use your locale as prefix/foldername. So if you were eg translating
to German, you would use 'de' as foldername where the images are in and the
filename for the game-screen screenshot would be this:
    * de/game-screen-1.13.11+dev.jpg
    * de/multiplayer-1.13.11+dev.png

    Please send the screenshots in a separate archive.


As a hint where you could create the screenshots:
* `game-screen-1.13.11+dev.jpg`: 3rd scenario of AToTB, just jump there with
  :debug and :n and then let the AI play some turns with :droid
* `right_pane-1.13.11+dev.png`: right part from the "game-screen" screenshot
* `top_pane-1.13.11+dev.png`: top part from the "game-screen" screenshot
* `main-menu-1.13.11+dev.jpg`: the main menu (obvious, isn't it?)
* `multiplayer-1.13.11+dev.png`: just the dialog that pops up when clicking on
  'multiplayer' in the main menu, take care to set the player name to the
  default value 'player' (or whatever it is in your lang, since this one is a
  translatable string)
* `recruit-1.13.11+dev.png`: start a skirmish game and select default era and
  rebels as your race, click on recruit and take a screenshot of the dialog
  popping up
