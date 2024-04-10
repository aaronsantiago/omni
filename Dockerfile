# syntax=docker/dockerfile:1

FROM ubuntu:jammy

# CMD python --version

# ARG DEBIAN_FRONTEND=noninteractive
RUN apt update && apt upgrade -y
# RUN apt install -y software-properties-common
# RUN add-apt-repository ppa:deadsnakes/ppa
RUN apt install build-essential git curl cmake-curses-gui libjsoncpp-dev libx264-dev libx265-dev libjpeg-dev libpng-dev libcurl4-openssl-dev libiec61883-dev libavc1394-dev libavcodec-dev libavutil-dev libavformat-dev libswscale-dev libglew-dev libsdl1.2-dev libsdl2-dev libboost-all-dev -y
RUN apt install wget -y
WORKDIR /app
COPY dicaffeine_get_ndi5.sh .

# RUN python -m venv venv
# # RUN venv/bin/pip install -r requirements.txt

# RUN wget https://dicaffeine.com/_media/bin:scripts:dicaffeine_get_ndi5.sh
RUN chmod +x "./dicaffeine_get_ndi5.sh"
RUN "./dicaffeine_get_ndi5.sh" install x86_64-linux-gnu

COPY build.sh .
