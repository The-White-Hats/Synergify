#! /usr/bin/bash
# this bash script is meant to run on ubuntu
#? to run it on windows, use a bash command line like git bash
ImageName=synergify
flag=$(docker images | grep $ImageName-image)

# make sure the this file exist for valid working of the container
touch processes.txt

# Check if the 'flag' variable is empty
# that means that there is no an image with such a name
if [[ -z $flag ]]; then
  docker build -f ./docker/Dockerfile -t synergify-image .
fi
docker compose -f ./docker/docker-compose.yml up -d
docker attach $ImageName-container