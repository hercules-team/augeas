FROM alpine

RUN apk add --no-cache libgcc libxml2 readline bash

RUN set -xe ; \
    apk add --no-cache --virtual=.build gcc libc-dev make bison flex readline-dev libxml2-dev git automake autoconf libtool pkgconf coreutils \
 && git clone git://github.com/hercules-team/augeas \
 && cd augeas \
 && ./autogen.sh && make && make install ; \
    cd - \
 && rm -fr /augeas ; \
    apk del .build

RUN set -x ; \
    cd /usr/local/bin \
 && for TOOL in augcheck auggrep augload augloadone augparsediff augsed ; \
    do \
      wget https://raw.githubusercontent.com/raphink/augeas-sandbox/master/"$TOOL" ; \
      chmod +x "$TOOL" ; \
    done

CMD augtool
