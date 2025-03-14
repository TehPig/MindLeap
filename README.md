# Compiling
This guide will walk you through the process of compiling and running the application.

## Prerequisites

Make sure the following tools are installed:

1. **C++ Compiler** (e.g., GCC, Clang, MSVC)
2. **CMake** (for managing the build process)
3. **Qt** (version 6.8.1 or higher)

## Installing Dependencies

1. **Install CMake**:
    - On **Ubuntu/Debian**:
      `sudo apt-get install cmake`
    - On **macOS**:
      `brew install cmake`
    - On **Windows**, download and install from [CMake official site](https://cmake.org/download/).

2. **Install Qt**:
    - On **Ubuntu/Debian**:
      `sudo apt-get install qt6-qmake qtbase6-dev`
    - On **macOS**:
      `brew install qt`
    - On **Windows**, download and install Qt from [Qt official site](https://www.qt.io/download).

3. **Install a C++ Compiler**:
    - **Linux**: GCC or Clang should be pre-installed or can be installed with sudo apt-get install build-essential (for GCC).
    - **macOS**: Install Xcode command line tools by running xcode-select --install.
    - **Windows**: You can install MSVC or MinGW.

## Setting Up the Project

Clone the repository or download the source files:
```
git clone https://github.com/TehPigYT/anki-project.git
```
Alternatively, download the ZIP file of the repository and extract it.

## CMake Build Process

1. **Navigate to the project folder**:
   ```
   cd anki-project
   ```

3. **Create a build directory** (This keeps the source and build files separate):
   ```
   mkdir build
   cd build
   ```

5. **Generate build files using CMake**:
   CMake will detect your system configuration and generate the appropriate build files (e.g., Makefiles for Linux/macOS or Visual Studio project files for Windows).

   - For **Linux/macOS**:
     ```
     cmake ..
     ```

   - For **Windows** (optional but recommended to specify your generator, e.g., Visual Studio):
     ```
     cmake -G "Visual Studio 16 2019" ..
     ```

6. **Build the Project**:
   - For **Linux/macOS**:
     ```
     make
     ```

   - For **Windows**:
     - Open the generated .sln file in Visual Studio and build the solution.

7. **Build Debug/Release** (optional):
    If you want to build in Debug or Release mode, you can specify this when running cmake:
    - For **Debug**:
      ```
      cmake -DCMAKE_BUILD_TYPE=Debug ..
      ```
    - For **Release**:
      ```
      cmake -DCMAKE_BUILD_TYPE=Release ..
      ```

## Running the Application

After building, you can run the application.

- **Linux/macOS**:
  ```
  ./MindLeap
  ```

- **Windows**:
  On PowerShell
  ```
  ./MindLeap.exe
  ```
  On Command Prompt
  ```
  MindLeap.exe
  ```

## Docker

#### Prerequisites
1. **Docker**: Install Docker from https://www.docker.com/get-started.

#### Steps to Compile

1. **Clone the Repository**
   - `git clone https://github.com/TehPigYT/anki-project.git`
   - `cd anki-project`

2. **Build the Docker Image**
   - `docker build -t MindLeap .`

3. **Run the Docker Container**
   - `docker run --rm -it MindLeap`

4. **Optional - Using Volume Mounts**
   - `docker run --rm -it -v $(pwd):/app MindLeap`

5. **Running the Application**
   - Follow the Docker container instructions to run the application.

#### Manual Compilation (Without Docker)
1. Create a build directory:
   - `mkdir build && cd build`

2. Run CMake:
   - `cmake ..`

3. Compile the application:
   - `make`

4. Run the application:
   - `./MindLeap`

## Troubleshooting

- **Qt not found**: Make sure the qmake and Qt libraries are correctly installed. You may need to set the CMAKE_PREFIX_PATH to the Qt installation directory:
  cmake -DCMAKE_PREFIX_PATH=/path/to/qt ..

- **Missing dependencies**: If your application requires additional libraries (e.g., SQLite), make sure they are installed, and check that CMakeLists.txt correctly links them.

## Additional Notes

- The minimum required Qt version for this project is **Qt 6.8.1**.
- It's recommended to use **GCC 9.3** or newer to compile the project.
