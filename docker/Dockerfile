FROM alpine AS build

RUN apk add --no-cache --virtual=.build gcc libc-dev make bison flex readline-dev libxml2-dev git automake autoconf libtool pkgconf coreutils bash
COPY . /augeas
RUN set -ex ; \
    cd augeas \
 && ./autogen.sh --prefix=/opt/augeas \
 && make \
 && make install

FROM alpine
RUN apk add --no-cache libgcc libxml2 readline bash
COPY --from=build /opt/augeas /opt/augeas
ENV PATH=$PATH:/opt/augeas/bin
RUN set -x ; \
    cd /opt/augeas/bin \
 && for TOOL in augcheck auggrep augload augloadone augparsediff augsed ; \
    do \
      wget https://raw.githubusercontent.com/raphink/augeas-sandbox/master/"$TOOL" ; \
      chmod +x "$TOOL" ; \
    done
CMD augtool
