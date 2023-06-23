# Arcanoid game

Build Platform          |
------------------------|
Linux x64(gcc, clang)   |
Windows x64(MSVC, LLVM) |

![Arcanoid](help-imgs/game.png)

## Build steps for Linux

1. Run the following sets of commands in terminal:

```
git clone --recurse-submodules https://github.com/ArthurBandaryk/LestaGamesCourse.git
cd LestaGamesCourse/arcanoid
cmake -B build -G "Ninja" -S .
cmake --build build

```

2. To run the game just do the following:

```
cd build && ./arcanoid

```

## Build steps for Windows

### Using LLVM compiler infrastructure

1. First of all make sure you have installed llvm. Use [`this`](https://github.com/llvm/llvm-project/releases/tag/llvmorg-15.0.7) link to download exe needed.

2. You may also need for [`ninja`](https://github.com/ninja-build/ninja/releases).
3.  And of course [`cmake`](https://cmake.org/download/#latest).

4. Run the following set of commands to build project:

```
git clone --recurse-submodules https://github.com/ArthurBandaryk/LestaGamesCourse.git
cd LestaGamesCourse/arcanoid
cmake -B build -G "Ninja" -S .
cmake --build build

```

5. To `run` this game just do the following:

```
cd build && arcanoid.exe

```

### Using MSVC

1. First of all clone [`this`](https://github.com/ArthurBandaryk/LestaGamesCourse) repo:

```
git clone --recurse-submodules https://github.com/ArthurBandaryk/LestaGamesCourse.git

```

2. Then you can open arcanoid folder with Microsoft Visual Studio:

![Open with MSVC](help-imgs/open.png)

3. Set the workspace root to `arcanoid/CMakeLists.txt` file. 

4. After that you should press `CTRL + S` on the root `CMakeLists.txt` (arcanoid/CMakeLists.txt) to generate build files.

5. Then just press `Build` button:

![Build with MSVC](help-imgs/build.png)

6. Then click `Run` button and enjoy it:)
