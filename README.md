# Gitman Lite
Gitman Lite is a faster, more lightweight version of [gitman](https://github.com/pyrod3v/gitman), written in C

## Features
- Repository initialization
- .gitignore selection
- Custom gitignore templates

## Configuration
The application's configuration is located at `$USER/.config/gitman` on unix-like systems and at `%appdata%\Roaming\gitman` on Windows.
To add custom .gitignore templates, put any `<name>.gitignore` file in the gitignore directory in your config.

## Installing
To install the application, simply clone this repository and run the `build.sh` script

## How to use
The compiled binary's name is `gm`. You can use the following flags with it:
- **-d** or **--directory**: the directory to use. Defaults to the current running directory.
- **-n** or **--name**: the repository's name. If this is provided it will initialize a repository in the specified directory.
- **-g** or **--gitignore**: the gitignore template to use.
- **--user.name**: git username to use
- **--user.email**: git email to use

## Contributing
All sorts of contributions are welcome. To contribute:
1. Fork this repository
2. Create your feature branch
3. Commit and push your changes
4. Submit a pull request

Please use meaningful commit messages
