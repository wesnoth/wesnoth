### UI
   * Restored the original footprint images as the `default` set, used by units that do not set the `footprints` key.
   * The `footprints` key can now also be set under `[race]`, applying to every unit of that race. A `footprints` key on the unit type takes precedence over the race one.
   * Footprints are drawn darker and at a stronger opacity as the map is zoomed out, so they stay readable when the hexes are small.
