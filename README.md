# kirk-engine-webassembly
Implementation of the Kirk cryptography hardware on the PSP, compiled into webassembly

This code is copied from the PPSSPP emulator: https://github.com/hrydgard/ppsspp which seems to be the most up-to-date version of this lib that I can find.

The philosophy when choosing what code to copy over has been to favour making minimal changes to the code. This results in copying over much more code than is strictly needed for what we're trying to do, but it hopefully makes it easier to update the code in the future. We rely on `emcc` to strip out any code that isn't needed from the final build.

Search the code for `EMCC_CHANGE` to find the places that were changed when importing the code from PPSSPP.

If new functions are added that you want to be able to call from Javascript, they need to be added to the `makefile`. Don't forget an `extern "C"` if they're in C++ (to prevent C++ name mangling)!

Suggestions are welcome!

## Exported functions
- `init_kirk()` *** Be sure to call this first! ***
- `decrypt_buffer()` Decrypts a buffer containing a save file

## Instructions

- Go to the Releases tab and download the latest `.js` and `.wasm` files
- Copy the files into your project in an appropriate directory
- Call the code like this:

```
import createModule from 'path/kirk-engine-webassembly';

const moduleOverrides = {
  locateFile: (s) => `relative/path/from/package.json/${s}`,
};

const moduleInstance = await createModule(moduleOverrides);

const result = moduleInstance._functionName(param1, param2);
```

To see other methods of calling the functions (using `cwrap()` and `ccall()`), check out: https://emscripten.org/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html#interacting-with-code-ccall-cwrap

Note that all of the parameters are pointers, so you'll need to `malloc()` and `free()` memory from javascript side, write and read to/from the pointer using `setValue()` and `getValue()`, and pass in the pointers you receive from `malloc()`

In a browser environment, you may need to do some configuration of Webpack to package the `.wasm` file, since it's loaded asynchronously.

Here's what I had to do:
- Serving the file when running locally on the desktop: https://github.com/emscripten-core/emscripten/issues/10114#issuecomment-569561505 (bottom part)
- Making the file run when everything is served from a remote webserver: good luck! If you can find a better method than me, I'd love to hear it!

## Building it yourself

`brew install emscripten`
`brew install make` (the default version of `make` on MacOS doesn't support `.RECIPEPREFIX`, so requires tabs rather than spaces)
`gmake release` or `gmake debug`

Then 2 files will be written to the `/out` directory that can be copied into your project.

`gmake clean` to clean the `out/` directory

### Alternative builds

By default, the project compiles to web assembly with a Javascript harness. You can build the entirety of the C++ code to Javascript only or webassembly only by changing compiler flags in the `makefile`:

- `-s STANDALONE_WASM=1 --no-entry` to build everything to webassembly only
- `-s WASM=0` to build everything to Javascript only

See https://github.com/emscripten-core/emscripten/blob/main/src/settings.js for more details

Also, by default the build outputs both `cwrap()` and `ccall()` functions in the Javascript harness. You can omit one or both (if you want to call the C++ functions directly from Javascript) by changing the `EXPORTED_RUNTIME_METHODS` flag in the `makefile`
