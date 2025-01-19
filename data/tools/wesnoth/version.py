#! /usr/bin/env python3

major = 1
minor = 19
release = 8
dev_suffix = "+dev" # either use "+dev", "-dev", "" (empty string) or None

as_string = "{}.{}.{}{}".format(major,
                                minor,
                                release,
                                dev_suffix if dev_suffix else "")

if __name__ == "__main__":
    print("WML Tools version module")
    print("Version:", as_string)
