### Miscellaneous and Bug Fixes
   * Fixed several audio regressions from the SDL3 migration (issue #11395):
      * Restored positional [sound_source] fading, so sounds attenuate with distance again.
      * Fixed reading back and scaling of the sound, music, bell and UI volume levels.
      * Fixed one-time sounds alternating at half volume.
   * Fixed a looping [sound_source] jumping abruptly to full volume when scrolled into range instead of fading in.
   * Sound sources now update their volume when zooming while the view is against a map edge.
   * [sound_source] volume now fades smoothly with pixel distance rather than in whole-hex steps.
