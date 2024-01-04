Term - Terminal window and user interface library for C++
=========================================================

Term is (yet another) library for creating windowed user interfaces on terminals.

[![License](https://img.shields.io/badge/License-Apache-lightgray.svg?style=plastic)](https://www.apache.org/licenses/LICENSE-2.0.txt)


Table of contents
-----------------

* [Introduction](#introduction)
* [Installation](#installation)
* [Usage](#usage)
* [Known issues and limitations](#known-issues-and-limitations)
* [Getting help](#getting-help)
* [Contributing](#contributing)
* [License](#license)
* [Authors and history](#authors-and-history)
* [Acknowledgments](#acknowledgments)


Introduction
------------

The library is functionally divided into two domains:

- Creating and maintaining windows on terminals
- Creating user interfaces in windows

Both parts can be used together, or separately, because they share the same
mechanism for maintaining rectangles on surfaces and their output mechanism can
easily be adapted to any device. For example, the "output device" for the user
interface part is a terminal window.


Features
--------

- Overlapping windows
- Rich color and attribute support
- Mouse support
- Auto-expanding and auto-contracting desktop
- Flex-based layout engine


Prerequisites
-------------

- Conan
- Meson
- ninja
- g++


Installation
------------

- Install prerequisites (brew is recommended, but you can use your package manager)

```bash
a command-line command here
```


Usage
-----

TBD: This section explains the principles behind this README file.  If this repository were for actual _software_, this [Usage](#usage) section would explain more about how to run the software, what kind of output or behavior to expect, and so on.


### Basic operation

TBD


Known issues and limitations
----------------------------

At the moment the terminal output layer is only issuing control sequences for ANSI terminals, such as xterm.


Getting help
------------

TBD: Inform readers of how they can contact you, or at least how they can report problems they may encounter.  This may simply be a request to use the issue tracker on your repository, but many projects have associated chat or mailing lists, and this section is a good place to mention those.


Contributing
------------

TDB: Mention how people can offer contributions, and point them to your guidelines for contributing.


License
-------

Licensed under the Apache License, Version 2.0 (the "License"); you may not use
this software except in compliance with the License. You may obtain a copy of the
License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.


Authors and history
-------------------

- Johnny Wezel (http://github.com/jwezel)


Acknowledgments
---------------

* https://github.com/mhucka/readmine
