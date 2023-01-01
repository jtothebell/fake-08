FROM ubuntu:20.04

ADD https://github.com/FunKey-Project/FunKey-OS/releases/download/FunKey-OS-2.3.0/FunKey-sdk-2.3.0.tar.gz /root/
RUN mkdir -p /opt && cd /opt && tar jxvf /root/FunKey-sdk-2.3.0.tar.gz

RUN echo 'export PATH="$PATH:/opt/FunKey-sdk-2/usr/bin"' >> /root/.bashrc

RUN apt-get update && apt-get install -y build-essential gcc-multilib

CMD /bin/bash