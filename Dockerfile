FROM ubuntu:latest

RUN apt-get update && apt-get install -y git cmake make
RUN apt-get install -y build-essential libgl1-mesa-dev
RUN apt-get install -y libsoil-dev libglm-dev libassimp-dev
RUN apt-get install -y libglew-dev libglfw3-dev libxinerama-dev 
RUN apt-get install -y libxcursor-dev libxi-dev mesa-common-dev mesa-utils libxxf86vm-dev libfreetype6-dev

RUN apt-get update && apt-get install -y \
    cmake \
    make \
    g++ \
    libglfw3 \
    libglfw3-dev \
    libgl1-mesa-dev \
    xorg-dev \
    libglu1-mesa \
    libglu1-mesa-dev \
    && rm -rf /var/lib/apt/lists/*
    
WORKDIR /app

COPY . /app

RUN cmake . && make

CMD ["./project_base"]

