![image-alt-name](./logo.png)

[![Continuous Integration](https://github.com/khbminus/fastTyping/actions/workflows/ci.yml/badge.svg)](https://github.com/khbminus/fastTyping/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
![GitHub commit activity](https://img.shields.io/github/commit-activity/m/khbminus/fastTyping)

# Typing Simulator with Competitive Gameplay

## About the Project

FASTYTPIGN - a client-server application that allows you to practice typing,
measure WPM and compete with friends in fast text typing.

## Dependencies

- In this project, Qt version 6 or higher is used for the client. Development and testing were done on Qt 6.2.0.
- For the server, you will need Boost.
- Additionally, the server will require [libpqxx](https://github.com/jtv/libpqxx).
- The last dependency is [nlohmann/json](https://github.com/nlohmann/json).

## Installation

### Installer

You can use the bundled installer for client installation (Linux).

### Manual Installation

```sh
git clone https://github.com/khbminus/fastTyping.git
mkdir build
cd build
cmake .. -DBUILD_ALL=ON -DBUILD_CLIENT=OFF -DBUILD_SERVER=OFF # Depending on what you want to build, set the flags
make server
make client
make install # Optional
```

### Necessary Files

To run the client correctly, layout files must be located in the project's working directory: `qwerty.json`,
`colemak.json`, `dvorak.json`.

To ensure the server works correctly, you also need to build and execute the dictionaries-interface target, which initializes dictionaries. All text dictionaries should be in the working folder in dicts/, and DLL dictionaries should simply be in the root folder.