FROM ubuntu

# The main working dirctory is /home/synergify 
WORKDIR /home/synergify
# Copy the needed files and folders
COPY . /home/synergify/

RUN apt-get update && \
    apt-get install htop gcc make valgrind -y