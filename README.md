# cbalc

Discord bot built with [concord](https://github.com/Cogmasters/concord).

## Build and Run

1. Follow the [concord](https://github.com/Cogmasters/concord) installation instructions.
2. Set bot token in [`config.json`](config.json).
3. Build:

```shell
make cbalc
```

4. Run:

```shell
./cbalc
```

## Run in Docker Container

1. Copy [`config.json`](config.json) into `configdev.json`:

```shell
cp config.json configdev.json
```

2. Set bot token in `configdev.json`.

3. Build image from [`Dockerfile`](Dockerfile):

```shell
docker build -t cbalc .
```

4. Run container:

```shell
docker run -d cbalc
```
