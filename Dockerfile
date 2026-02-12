FROM ubuntu:22.04
LABEL authors="TehPig"

# Install system dependencies
RUN apt-get update && \
    apt-get install -y build-essential wget python3 python3-pip cmake git \
    libgl1-mesa-dev libglib2.0-dev libpulse-dev libdbus-1-dev libxkbcommon-dev \
    libfontconfig1-dev libicu-dev

# Install aqtinstall
RUN pip3 install aqtinstall

# Install Qt 6.8.1 + required modules
RUN aqt install-qt linux desktop 6.8.1 linux_gcc_64 --outputdir /opt/Qt \
    --modules qtmultimedia

# Set environment variables
ENV PATH="/opt/Qt/6.8.1/gcc_64/bin:$PATH"
ENV QT_PLUGIN_PATH="/opt/Qt/6.8.1/gcc_64/plugins"
ENV LD_LIBRARY_PATH="/opt/Qt/6.8.1/gcc_64/lib"
ENV CMAKE_PREFIX_PATH="/opt/Qt/6.8.1/gcc_64"

# Copy source
COPY . /app
WORKDIR /app

# Build
RUN cmake -Bbuild -S. && cmake --build build -- -j$(nproc)

# Run the app
CMD ["./build/MindLeap"]
