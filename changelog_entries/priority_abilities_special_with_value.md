### WML Engine
   * Add a 'priority' attribute to abilities and weapon specials with values (`[chance_to_hit]`, `[damage]`, etc).
     * Values set by lower priority specials are considered as the base value for higher priority specials' calculations.
   * Using `[chance_to_hit]` with a negative priority is now preferred over giving weapons `parry` and `accuracy` attributes, as the sidebar UI shows it.
   * Add a [erase_lower_priority] subtag to abilities and specials with value for erase abilities/specials of lower priority who matches.
     * Add to [erase_lower_priority] an affect_side attribute for erase specials applied to a particular side(self or opponent)(for specials only)
