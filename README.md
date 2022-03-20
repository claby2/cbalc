# cbalc

Discord bot built with [concord](https://github.com/Cogmasters/concord).

## Build and Run

1. Follow the [concord](https://github.com/Cogmasters/concord) installation instructions.
2. Set bot token in [config.json](config.json).
3. Build:

```shell
make cbalc
```

4. Run:

```shell
./cbalc
```

## Run in Docker Container

```shell
docker build -t cbalc .

docker run -d cbalc
```
