### WML Engine
   * Add a 'priority' attribute to abilities and weapon specials with values (`[chance_to_hit]`, `[damage]`, etc).
     * Values set by lower priority specials are considered as the base value for higher priority specials' calculations.
   * Using `[chance_to_hit]` with a negative priority is now preferred over giving weapons `parry` and `accuracy` attributes, as the sidebar UI shows it.
