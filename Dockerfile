ARG BUILD_PATH=/tmp/propware-build
ARG EXPECTED_PROPGCC_PREFIX=/opt/parallax.gcc4_1.9.0

FROM davidzemon/pwbuilder:latest as builder

# Assert PROPGCC_PREFIX is as we expect
ARG EXPECTED_PROPGCC_PREFIX
RUN echo "EXPECTED_PROPGCC_PREFIX   = ${EXPECTED_PROPGCC_PREFIX}" && \
    echo "PROPGCC_PREFIX            = ${PROPGCC_PREFIX}" && \
    test "x${EXPECTED_PROPGCC_PREFIX}" = "x${PROPGCC_PREFIX}"

# Default level of compilation parallelism. Can be overriden with `--build-arg MAKE_JOBS=<X>`
ARG MAKE_JOBS=4

ENV SOURCE_PATH=/tmp/propware-source

COPY . "${SOURCE_PATH}"
RUN cmake -S "${SOURCE_PATH}" -B "${BUILD_PATH}" -D PACKAGE_LINUX=ON \
    && cmake --build "${BUILD_PATH}" --parallel ${MAKE_JOBS} \
    && cd "${BUILD_PATH}" && \
    cpack -G DEB

###################################################

FROM ubuntu:20.04 as propware

ARG EXPECTED_PROPGCC_PREFIX
ENV PROPGCC_PREFIX="${EXPECTED_PROPGCC_PREFIX}"

ENV DEBIAN_FRONTEND=noninteractive
COPY --from=builder "${BUILD_PATH}"/*.deb .
COPY --from=builder "${EXPECTED_PROPGCC_PREFIX}" "${EXPECTED_PROPGCC_PREFIX}"
COPY --from=builder "/usr/local/bin/spin2cpp" "/usr/local/bin/spin2cpp"

RUN apt-get update \
    && apt-get install --yes \
        cmake \
        make \
    && dpkg -i "${BUILD_PATH}"/*.deb \
    && rm "${BUILD_PATH}"/*.deb \
    && rm -rf /var/lib/apt/lists/*
