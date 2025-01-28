FROM ubuntu:24.04

RUN apt update && apt install build-essential git -y
RUN apt install python3-venv curl -y

RUN userdel -r ubuntu && useradd -u 1000 -ms /bin/bash ian
RUN mkdir /home/ian/workspace && chown -R ian:ian /home/ian

USER ian
