FROM ubuntu:24.04

RUN apt update && apt install build-essential git -y
RUN apt install python3-venv curl -y
RUN apt install vim -y

RUN userdel -r ubuntu && useradd -u 1000 -ms /bin/bash ian
RUN mkdir /home/ian/workspace && chown -R ian:ian /home/ian

RUN usermod -a -G dialout ian
RUN usermod -a -G plugdev ian

USER ian
RUN curl -fsSL -o /tmp/get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
RUN python3 /tmp/get-platformio.py
RUN echo 'export PATH="$PATH:$HOME/.platformio/penv/bin"' >> /home/ian/.bashrc
