FROM alpine

RUN apk add --no-cache libgcc libxml2 readline

RUN set -xe ; \
    apk add --no-cache --virtual=.build gcc make bison flex readline-dev libxml2-dev git automake autoconf libtool pkgconf coreutils \
 && git clone git://github.com/hercules-team/augeas \
 && cd augeas \
 && ./autogen.sh && make && make install ; \
    cd - \
 && rm -fr /augeas ; \
    apk del .build

CMD augtool
