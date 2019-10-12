# Contributing to Wesnoth

Wesnoth was built with the hard work of volunteers all over the world! Everyone is welcome to contribute.

## Contacting Us

The best place to get in touch with the development team is on our [Discord server](https://discord.gg/battleforwesnoth) or on the [Wesnoth forums](https://forums.wesnoth.org/).

## Art and Music

We generally accept art and music submissions where needed. There isn't any definitive list of assets we require, so we recommend you talk to us first before you start. We're also open to commissions for larger projects (character portraits, story art, music tracks, etc). Definitely get in touch if you're interested!

## Bug Reports

Please report all bugs you find here on GitHub (preferably) or on the forums.

### Bugs in User-Made Content

If you encounter an engine bug such as a crash, scripting error, etc., do report it here. Otherwise, problems with user-made content should be reported to the respective creators on the forums. You can usually find a thread for the add-on in question in the [Scenario & Campaign Development](http://www.wesnoth.org/forum/viewforum.php?f=8), [Faction & Era Development](http://www.wesnoth.org/forum/viewforum.php?f=19) or [Multiplayer Development](http://www.wesnoth.org/forum/viewforum.php?f=15) sections.

### Information to Include

We've put together several issue templates to choose from when opening a bug report. Please choose the one that best fits the bug. They'll outline any info we might need. You don't need to include *everything* (we don't need screenshots for a compiling issue, for example), but generally, the more you can include, the better! We can't fix a bug we can't track down.

### Feature Requests

We also accept suggestions for campaign improves, WML or Lua API changes, and general game enhancements here on GitHub. However, keep in mind it may take awhile before your idea is implemented, if at all.

## Pull Requests

TODO: stuff about PRs.

### Code Formatting

If your pull request touches the engine's C++ source code, we recommend (but don't require) you run `.clang-format` on your changes before submitting them (Visual Studio Code gives you a handy context menu option to do so). This ensures your code remains formatted according to our conventions.

Generally, we follow a few rules:

- We use modern C++11 and later features. Use standard library APIs whenever possible over hand-rolled or third-party libraries.
- No spaces after `if` - ie, use `if()` and `while()` not `if ()` and `while ()`.
- Keep opening brackets on the same line for conditional and control blocks. Put them on new lines for class, struct, and namespace declarations.
- Avoid C-style code, like casts (`(int)1.0`), arrays (`int[] foo;`), or function pointers (`void (*foo)()`). Use `static_cast`, `std::array`, or `std::function`, respectively.
- Use `nullptr`, not `NULL` or `0`.
- Do not use macros for constants. Use `constexpr` or `static`.
- Use `const` as much as possible, where applicable.
- End *non-public* class data members with an underscore.
