FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    wget \
    gnupg \
    lsb-release \
    software-properties-common \
    cmake \
    make \
    ninja-build \
    git \
    openjdk-17-jre-headless \
    && rm -rf /var/lib/apt/lists/*

# Install LLVM/Clang 18 and libc++ 18
RUN wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc && \
    echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-18 main" > /etc/apt/sources.list.d/llvm.list && \
    apt-get update && apt-get install -y \
    clang-18 \
    libc++-18-dev \
    libc++abi-18-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

COPY . .

RUN cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang-libc++.cmake \
    && cmake --build build -j$(nproc)

ENTRYPOINT ["/workspace/build/typechecker/cli/typechecker"]
CMD ["--help"]
