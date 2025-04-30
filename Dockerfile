FROM ubuntu:22.04
LABEL authors="TehPig"

# Install necessary packages
RUN apt-get update && \
    apt-get install -y build-essential wget python3 python3-pip cmake git libgl1-mesa-dev libglib2.0-dev

# Install aqtinstall for managing Qt installations
RUN pip3 install aqtinstall

# Install specific Qt version with aqtinstall
RUN aqt install-qt linux desktop 6.8.1 linux_gcc_64 --outputdir /opt/Qt

# Set environment variables for CMake and Qt
ENV PATH="/opt/Qt/6.8.1/gcc_64/bin:$PATH"
ENV QT_PLUGIN_PATH="/opt/Qt/6.8.1/gcc_64/plugins"
ENV LD_LIBRARY_PATH="/opt/Qt/6.8.1/gcc_64/lib"

# Copy the application source code into the container
COPY . /app
WORKDIR /app

# Configure and build the application with CMake
RUN cmake -Bbuild -H. -DCMAKE_PREFIX_PATH=/opt/Qt/6.8.1/gcc_64 && \
    cmake --build build -- -j$(nproc)

# Define the entry point to run the Qt application
CMD ["./build/MindLeap"]
