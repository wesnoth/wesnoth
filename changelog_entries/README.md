This directory is where PR authors can add a file per PR containing the text that should be added to the main changelog.md during the release process for the next version. The format should match that of the main changelog as well as use an existing category defined there. For example, a file could contain:

    ### Terrain
    * Change codes "Irs, Ias, Icr, Ior, and Icn" to "Isr, Isa, Isc, Iwo, and Iwc", respectively

The purpose of this is to make it easier to add changelog entries with a PR while avoiding merge conflicts in the main changelog file if someone ends up merging their PR before yours.

The contents of this directory (aside from this README file) will then be deleted after each release.
