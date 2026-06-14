### WML Engine
   * Because overwrite_specials cannot be applied to other abilities and its design and functionality leave something to be desired, an [overwrite_specials] subtag will be added to eventually replace it.
     * Add to [overwrite_specials] an affect attribute to suppress special abilities affecting a unit applied to a particular side (apply_to=self or opponent) and which discriminates special abilities that affect self/student from special abilities belonging to the opponent, even if apply_to=attacker/defender is used.
   * The use of 'overwrite_specials' attribute and [overwrite] subtag in specials are deprecated
