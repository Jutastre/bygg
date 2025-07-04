# bygg
### A minimalistic build tool built on top of make

bygg generates a Makefile with some generic boilerplate stuff, and then calls make, passing any arguments given on to make. It's configurable with its own config file which is a lot more minimalistic, but you can add additional custom rules for make in there too. An empty file is a viable option although you might want to configure things like the name of the binary or set a different optimization level. 

For tiny C projects it does the job with minimal configuration, and beats having to copy a Makefile from a different project every time.

There's no documentation yet, but its not even 200 lines of very straightforward C (It is its own usecase in a sense), and the defaults are listed in byggfile.example (The other byggfile is for building bygg).

### TODO:
Some way of configuring the defaults system-wide perhaps; the current ones are just my own c23 preference, and the defaults are a large part of the whole concept, so this is probably necessary to make it useful for anyone else. 

Perhaps an argument could load an entirely different set of defaults; for example a release mode.

Should probably intercept some make arguments; to avoid things breaking, or to mirror behaviour of ones like -s and -f.
