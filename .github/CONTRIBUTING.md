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

We accept suggestions for campaign improvements, WML or Lua API changes, and other game enhancements here on GitHub. We encourage you to attempt to implement your idea yourself and submit a pull request containing relevant information to your feature.

### Information to Include

We have several issue templates to choose from when opening a bug report. Please choose the one that best fits the bug. You do not need to include everything (we don't need screenshots for a compiling issue, for example), but the more information you can provide, the better. We need at least enough information to replicate the bug before we can track down the root cause.

## Pull Requests

TODO: stuff about PRs.

### Code Standard

Wesnoth's engine conforms to the C++14 standard. We encourage the use of standard library APIs over third-party libraries whenever possible. However, third-party libraries are preferred over adding new, custom in-engine APIs, when appropriate.

### Code Formatting

If your pull request touches the engine's C++ source code, we recommend (but don't require) you run `clang-format` on your changes before submission (Visual Studio Code gives you a handy context menu option to do so). This ensures that your code remains formatted according to our conventions.

Generally, we follow these conventions in our C++ code:

```cpp
// Use pragma once instead of an include guard. Those are clumsy.
#pragma once

// Use angle brackets for system and external includes.
// Includes should also be sorted alphabetically.
#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <vector>

// Classes should have scope specifiers (public, protected, private), but structs can omit them.
struct my_struct
{
    // Public members do not need a trailing underscore.
    // Inline initialization is acceptable over a constructor.
    bool member = false;
};

// Put braces on new lines after class and struct declarations.
class my_class
{
public:
    // Use using directives over typedefs. They're easier to read.
    using alias_t = std::vector<my_struct>;

    // Use leading commas in the ctor list
    // Use the T& foo or T* foo reference and pointer styles, not T &foo or T *foo.
    explicit my_class(alias_t& ref)
        : the_array_of_doom_()
        , vec_ptr_(nullptr) // Use nullptr instead of NULL or 0
    {
        // Use C++ casts (static_cast and dynamic_cast) instead of C-style casts.
        // Do try and avoid reinterpret_cast and const_cast if at all possible.
        const float cast_test = static_cast<float>(how_far_to_mount_doom_);

        // Don't put a space after conditional keywords, and keep their opening brackets on the same line.
        if(!ref.empty()) {
            vec_ptr_ = &ref;

            // Use lambdas for short functions like this.
            // We also encourage the use of auto in lambdas and other places where
            // type names are long and can be inferred.
            std::sort(ref.begin(), ref.end(), [](const auto& a, const auto& b) {
                return a.member && !b.member;
            });
        }
    }

    /* Keep class method brackets on their own line, and always utilize const for methods and
     * variables when possible.
     *
     * For documenting functions, we loosely follow Doxygen conventions. You don't need to document
     * every single function, but important ones should optimally have at least a one-line comment
     * explaining what it does.
     *
     * @param speaker        The person speaking
     */
    void exclaim(const std::string& speaker) const
    {
        std::cerr << speaker ": They're taking the Hobbits to Isengard!" << std::endl;
    }

private:
    // End private class members with an underscore. Additionally, use C++ standard
    // like std::array as opposed to C equivalents (such as int[])
    std::array<int, 8> the_array_of_doom_;

    alias_t* vec_ptr_;

    // Use static or constexpr for constants. Don't use macros.
    static const int how_far_to_mount_doom_ = 1000;
};
```
