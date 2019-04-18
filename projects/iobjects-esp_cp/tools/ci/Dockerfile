FROM jenkins/jenkins:lts

USER root

RUN \
  apt-get update && apt-get install -y gcc git wget make libncurses-dev flex bison gperf python python-serial cmake genromfs xxd

RUN \
  mkdir -p /home/esp && cd /home/esp && \
  wget https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz && \
  tar -xzf xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz && \
  rm xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz && \
  git clone https://github.com/espressif/esp-idf.git -b release/v3.1 --recursive

COPY ./ci.pem /home/esp/
RUN chown jenkins:jenkins /home/esp/ci.pem && chmod 600 /home/esp/ci.pem
RUN chown -R jenkins:jenkins /home/esp/esp-idf

USER jenkins

COPY plugins.txt /home/jenkins/
RUN /usr/local/bin/install-plugins.sh < /home/jenkins/plugins.txt

ENV PATH="$PATH:/home/esp/xtensa-esp32-elf/bin"
ENV IDF_PATH="/home/esp/esp-idf"

# docker build -t linkline/ci:esp .
# docker run --rm -p 49042:8080 -p 50000:50000 -v jenkins_home:/var/jenkins_home linkline/ci:esp
# chromium-browser --app='http://localhost:49042'
