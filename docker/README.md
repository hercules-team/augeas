Building the image
------------------

First clone augeas repository then, build the image:

```shell
git clone https://github.com/hercules-team/augeas.git
cd augeas
docker build -t augeas -f docker/Dockerfile .
```

Using the image
---------------

Here is an exemple to print ssh configuration:

```shell
docker container run -ti --rm -v /etc/ssh/:/etc/ssh augeas augtool print /files/etc/ssh
```
