FROM ubuntu:25.04

RUN apt update && apt install build-essential cmake gcc g++ libstdc++-14-dev make libboost-all-dev openssl libpq-dev libasio-dev postgresql-client -y

WORKDIR /malphas

COPY . .

RUN rm -rf CMakeCache.txt CMakeFiles
RUN cmake -DNOCODEGEN=1 . && make -j 4
RUN rm -f config/db.cfg && mv config/db-docker.cfg config/db.cfg

ENTRYPOINT ["bash", "-c", "until pg_isready -h malphas-postgres -p 5432 -U malphas; do echo 'Waiting for database...'; sleep 2; done; ./malphas"]

