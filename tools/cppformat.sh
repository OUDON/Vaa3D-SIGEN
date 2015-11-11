#!/bin/zsh
ls sigen_core/**/*.{cpp,h} sigen_main/**/*.{cpp,h} vaa3d/*.{cpp,h} | xargs clang-format -i -style="{ BasedOnStyle: LLVM, Standard: Cpp03, ColumnLimit: 0 }"
