# Contributing to Wesnoth

Wesnoth was built with the hard work of volunteers all over the world! Everyone is welcome to come and contribute. TODO: list the different types of contributions that people can make.

## Contacting Us

The best place to get in touch with the development team is on our [official Discord community server](https://discord.gg/battleforwesnoth) or on the [Wesnoth forums](https://forums.wesnoth.org/).

The Discord server is mirrored to freenode IRC, channels: [`#wesnoth`](https://webchat.freenode.net/#wesnoth) (general discussions), [`#wesnoth-umc-dev`](https://webchat.freenode.net/#wesnoth-umc-dev) (questions about creating add-ons), [`#wesnoth-dev`](https://webchat.freenode.net/#wesnoth-dev)  (development of wesnoth mainline).

## Art and Music

Art and music submissions are accepted usually to fill in missing or outdated assets. If you are interested in contributing, we recommend that you contact us first to determine the best resources for you to work on based on need and interest. We also commission larger projects such as character portraits, story art, and music tracks.

## Bug Reports

Please report any bugs here on GitHub (preferred) or on the forums.

### Bugs in User-Made Content

If you encounter an engine bug such as a crash, scripting error, etc., report it here. Otherwise, issues with user-made content should be reported to their respective creators on the forums. You can usually find a thread for the add-on in question in the [Scenario & Campaign Development](http://www.wesnoth.org/forum/viewforum.php?f=8), [Faction & Era Development](http://www.wesnoth.org/forum/viewforum.php?f=19) or [Multiplayer Development](http://www.wesnoth.org/forum/viewforum.php?f=15) sections.

### Feature Requests

We accept suggestions for campaign improvements, WML or Lua API changes, and other game enhancements here on GitHub. In general, we recommend that you attempt to implement your idea yourself and submit a pull request containing relevant information to your feature.

### Information to Include

We have several issue templates to choose from when opening a bug report. Please choose the one that best fits the bug. You do not need to include everything (we don't need screenshots for a compiling issue, for example), but the more information you can provide, the better. We need at least enough information to replicate the bug before we can track down the root cause.

## Pull Requests

TODO: stuff about PRs.

### Code Formatting

If your pull request touches the engine's C++ source code, we recommend (but don't require) you run `.clang-format` on your changes before submission (Visual Studio Code gives you a handy context menu option to do so). This ensures that your code remains formatted according to our conventions.

Generally, we follow a few rules:

- We use modern C++11 and later features. Use standard library APIs whenever possible over hand-rolled or third-party libraries.
- No spaces after `if` - ie, use `if()` and `while()`, not `if ()` and `while ()`.
- Keep opening brackets on the same line for conditional and control blocks. Put them on new lines for class, struct, and namespace declarations.
- Avoid C-style code, like casts (`(int)1.0`), arrays (`int[] foo;`), or function pointers (`void (*foo)()`). Use `static_cast`, `std::array`, or `std::function`, respectively.
- Use `nullptr`, not `NULL` or `0`.
- Do not use macros for constants. Use `constexpr` or `static`.
- Use `const` as much as possible, where applicable.
- End *non-public* class data members with an underscore.
