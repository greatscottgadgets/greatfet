# Sandbox test environment for GreatFET
FROM ubuntu:22.04
USER root

# Override interactive installations and install prerequisite programs
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    cmake \
    curl \
    gcc-arm-none-eabi \
    git \
    libusb-1.0-0-dev \
    python3 \
    python3-pip \
    python3-venv \
    python3-yaml \
    software-properties-common \
    && rm -rf /var/lib/apt/lists/*
RUN pip3 install git+https://github.com/CapableRobot/CapableRobot_USBHub_Driver --upgrade

# Inform Docker that the container is listening on port 8080 at runtime
EXPOSE 8080
