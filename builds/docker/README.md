# Run in docker

Simple and fast setup of Metaverse on Docker is also available. 

## Install Dependencies
 - [Docker](https://docs.docker.com)

## Build eos image

```bash
git clone https://github.com/mvs-org/metaverse.git
cd metaverse/builds
docker build -t metaverse -f docker/Dockerfile .
```
## Look docker
Where is your built image? It’s in your machine’s local Docker image registry:
```bash
docker images
```
## Start docker

```bash
docker run -p 8820:8820 metaverse
```
## Test

```bash
curl http://127.0.0.1:8820
```

## Execute mvs-cli commands
 - [docker exec](https://docs.docker.com/engine/reference/commandline/exec/)


You can run the `mvs-cli` commands via `docker exec` command. Example:
```bash
docker exec metaverse mvs-cli getbestblockhash
```

## Access Shell of Running EOS Container

```bash
sudo docker exec -i -t metaverse /bin/bash
```
