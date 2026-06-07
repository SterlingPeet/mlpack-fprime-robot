FROM ubuntu:24.04

# Suppress interactive prompts from apt
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    # Build tools
    build-essential \
    cmake \
    ninja-build \
    git \
    # Project dependencies
    libarmadillo-dev \
    # aarch64 cross-compiler (RPi 4 target)
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    binutils-aarch64-linux-gnu \
    # Python (required by F Prime autocoder and fprime-util)
    python3 \
    python3-pip \
    python3-venv \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /project

# Install F Prime Python dependencies into a venv that mirrors the host setup.
# Requirements are installed at image-build time; the source tree is mounted
# at runtime so the image does not need to be rebuilt on code changes.
RUN python3 -m venv /fprime-venv \
    && /fprime-venv/bin/pip install --upgrade pip

# Copy only the requirements files so this layer is cached as long as
# dependencies don't change.
COPY requirements.txt fprime/requirements.txt* ./
RUN /fprime-venv/bin/pip install -r requirements.txt

ENV PATH="/fprime-venv/bin:$PATH"

CMD ["/bin/bash"]
