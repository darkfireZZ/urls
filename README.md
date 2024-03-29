
# urls

For each file given, `urls` prints the URLs in the file to standard output.

This is a very _simple_ and _minimal_ program. As such, it does not
implement a full URL parser / validator. It searches for knowns URL schemes,
followed by a colon (`:`), followed by a number of valid URL characters. This
works very well for most purposes and keeps the code clean and simple.

__Features:__

 - Minimal (~400 LOC), most of which is argument parsing and comments.
 - Fast
 - No libraries used other than the C standard library

__Limitations:__

 - May generate some false positives
 - Doesn't work for relative URLs as commonly used in HTML
   (e.g. `/images/example.png`). However, this is rather a limitation of the
   problem this program is trying to solve than one of the program itself.

## Usage

```
Usage: urls [OPTION]... [FILE]...

Extract URLs from every FILE and write them to standard out, separated
by newlines.
With no FILE, or when FILE is -, read standard input.

Options:
  -h, --help        Display this help and exit
  -s, --schemes     Comma-separated list of schemes to extract
```

See [documentation](#documentation) for more information.

## Documentation

The man page ([urls.1](./urls.1)) includes the full documentation of this
program. It can be viewed as follows:

```
git clone https://github.com/darkfirezz/urls
cd urls
man ./urls.1
```

## Installation

There is no installation script included here. If you want to install this
program, you'll have to do so manually. See [build](#build) for instructions on
how to build the program.

## Build

In order to build this program, you need to have the following programs
installed:

 - GNU make
 - a C compiler
 - an implementation of sed (e.g. GNU sed)

To build the program, clone the repository and run `make` in the source
directory:
```
git clone https://github.com/darkfirezz/urls
cd urls
make
```

This will create a `build/` directory and put the `urls` executable in that
directory.

### Development

To build a development version of the program, set the `DEBUG` variable when
running `make`.
```
make DEBUG=1
```

The following command will generate this README
```
make readme
```

## License

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
