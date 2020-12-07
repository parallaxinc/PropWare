# This image contains all of the tools necessary to compile PropWare and
# assemble the install packages for Linux and Windows


FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install --yes \
        cmake \
        doxygen \
        dpkg-dev \
        file \
        graphviz \
        make \
        nsis \
        pdf2svg \
        rpm \
        wget

# PropGCC
ENV PROPGCC_PREFIX=/opt/parallax.gcc4_1.9.0
RUN wget "https://ci.zemon.name/repository/download/PropGCC5_Gcc4linuxX64/3620:id/propellergcc-alpha_v1_9_0-gcc4-linux-x64.tar.gz?guest=1" \
       --quiet \
       -O /tmp/propgcc4.tar.gz \
    && tar -xf /tmp/propgcc4.tar.gz --directory /tmp \
    && mv /tmp/parallax "${PROPGCC_PREFIX}" \
    && rm /tmp/propgcc4.tar.gz

# Spin2cpp
RUN wget "https://ci.zemon.name/repository/download/Spin2Cpp_Linux/8902:id/spin2cpp.tar.gz!/spin2cpp?guest=1" \
       --quiet \
       -O /usr/local/bin/spin2cpp \
   && chmod +x /usr/local/bin/spin2cpp
