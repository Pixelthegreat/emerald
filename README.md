# Emerald Programming Language
Emerald is a simple python-like programming language. It has three primary goals:
- Friendliness: Emerald is easy to read and write
- Speed: The Emerald interpreter is fast despite being in a relatively slow class of interpreters
- Lightweight: Emerald is extremely portable and very cheap to embed

## Example
```
for i = 1 to 101 then
    let s = ''
    if i % 3 == 0 then let s = s + 'Fizz' end
    if i % 5 == 0 then let s = s + 'Buzz' end
    if not s then let s = toString(i) end
    puts s
end
```

## Documentation
There is currently no documentation for the language yet. I plan to add documentation in the future, but for now, please refer to the [language overview](doc/overview.md).

## Building
Emerald's only dependency is the C standard library. The build process requires [Premake 5](https://premake.github.io). To build on Linux, run the following:
```
premake5 gmake
make
```
When building on BSD, you will need to use `gmake` (GNU Make) instead of `make` (BSD Make), as the Makefiles that Premake 5 generates are not compatible with BSD Make.

Windows support is underway, but not very well tested at the moment. Mac support is indeterminate. Emerald is tested and fully functional on both Linux and FreeBSD.

### Build Options
When running `premake5`, you can pass a few options to control the build output:
- `--build-static`: Build a static library instead of a shared library for libemerald
- `--disable-modules=1,2,...`: Disable the building of certain standard library modules
- `--enable-modules=1,2,...`: Enable the building of ONLY specific standard library modules
- `--enable-asan`: Enable address sanitization (A debug feature)

## Installing
There is currently no way to install Emerald. However, I plan to add an install script in the near future.

## More
A few example programs exist in the `examples` directory, though it is quite sparse as of now. Some more programs exist in `test`, though they are not so much intended as examples.
