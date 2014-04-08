Brief description
-----------------

Clang-based standalone tool that checks that self include for each of passed
source is the first one in the file.

Building
--------

1. Clone the repository to `llvm/tools/clang/tools/extra`.
2. Add `add_subdirectory(self-inc-first)` to `CMakeLists.txt` in the same
   directory.
3. Run `ninja self-inc-first` inside your LLVM's `build/` directory.
