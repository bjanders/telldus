FROM debian

RUN apt-get update -y && apt-get upgrade -y
RUN apt-get install -y build-essential
RUN apt-get install -y libftdi-dev libconfuse-dev
RUN apt-get install -y cmake ninja-build
