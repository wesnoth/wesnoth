### UI
   * Footprints have been reworked:
     * The footprint path system now gives immediate strong visual feedback when multi-turn movement is assigned.
     * It does this by flashing any such footprints and fades them out over 2 seconds as the unit is deselected.
     * Changed how the red tint that indicated how movement costs works behind the scene. (cleanup)
     * Custom footprints is now possible with the new key `footprints` under `[unit_type]`, `[male]`, `[female]` and `[variation]` like `footprints="large_clawed_paw"`.
     * 8 new footprint variations added.
