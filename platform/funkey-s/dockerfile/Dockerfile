FROM ubuntu:20.04

RUN apt-get update && apt-get install -y build-essential

ADD https://github.com/FunKey-Project/FunKey-OS/releases/download/FunKey-OS-2.3.0/FunKey-sdk-2.3.0.tar.gz /root/
RUN mkdir -p /opt && cd /opt && tar -xzvf /root/FunKey-sdk-2.3.0.tar.gz

RUN echo 'export PATH="$PATH:/opt/FunKey-sdk-2.3.0/usr/bin"' >> /root/.bashrc

CMD /bin/bash