# Build/Run in docker

## Preparation
Install [Docker](https://docs.docker.com/).
```
wget qO https://get.docker.com/ | sh
```

## Build metaverse image
```
git clone https://github.com/mvs-org/metaverse.git
cd metaverse
docker build -t metaverse -f Dockerfile .
```

Where is your built image? It’s in your machine’s local Docker image registry:
```bash
docker images
```

## Run && Test

### Start docker container
```bash
docker run -p 8820:8820 metaverse
```

### Test
```bash
curl -X POST --data '{"jsonrpc":"2.0","method":"getinfo","params":[],"id":25}' http://127.0.0.1:8820/rpc/v2
```

### Execute mvs-cli commands
Run `mvs-cli` commands via `docker exec` command. Example:
```bash
docker exec metaverse mvs-cli getinfo
```
