# psp-encryption-webassembly
A copy of the implementation of the Kirk cryptography hardware on the PSP, compiled into webassembly

This code is copied from the PPSSPP emulator: https://github.com/hrydgard/ppsspp which seems to be the most up-to-date implementation of PSP encryption/decryption that I can find.

It also incorporates fixes from SED: https://github.com/BrianBTB/SED-PC

And we may want to also include fixes from https://github.com/cielavenir/psp-savedata-endecrypter

The philosophy when choosing what code to copy over has been to favour making minimal changes to the code. This results in copying over much more code than is strictly needed for what we're trying to do, but it hopefully makes it easier to update the code in the future. We rely on `emcc` to strip out any code that isn't needed from the final build.

Search the code for `EMCC_CHANGE` to find the places that were changed when importing the code from the various repos listed above.

If new functions are added that you want to be able to call from Javascript, they need to be added to the `makefile`. Don't forget an `extern "C"` if they're in C++ (to prevent C++ name mangling)!

Suggestions are welcome!

## Exported functions
- `init_kirk()` *** Be sure to call this first! ***
- `init_kirk_deterministic()` Call this instead if you need your encryption to always give the same result (e.g. for tests). Set the seed value to be non-zero.
- `decrypt_save_buffer()` Decrypts a buffer containing a save file
- `encrypt_save_buffer()` Encrypts a buffer containing a save file
- `decrypt_executable()` Decrypts a buffer containing an executable

## Instructions

- Go to the Releases tab and download the latest `.js` and `.wasm` files
- Copy the files into your project in an appropriate directory
- Basic example of how to call the code:

```
import createModule from '<path>/psp-encryption-webassembly';

const moduleOverrides = {
  locateFile: (s) => `relative/path/from/package.json/${s}`,
};

const moduleInstance = await createModule(moduleOverrides);

const result = moduleInstance._functionName(param1, param2);
```

Check out https://github.com/euan-forrester/save-file-converter/blob/main/frontend/src/save-formats/PSP/Savefile.js for a more in-depth example of loading the `.wasm` file in tests, dev, and prod, and how to call the code and pass in pointers, etc. This is just an example, and the method of loading the `.wasm` file in particular is very hacky. Feedback and suggested improvements are very welcome!

I also had to do this to make the `.wasm` file load in desktop development mode: https://github.com/euan-forrester/save-file-converter/blob/main/frontend/vue.config.js

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
