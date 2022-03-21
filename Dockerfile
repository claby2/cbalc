FROM ubuntu:latest

RUN apt-get update
RUN apt-get install -y \
	build-essential \
	git \
	libcurl4-openssl-dev

RUN git clone https://github.com/cogmasters/concord.git /app/concord
WORKDIR /app/concord
RUN make && make install

WORKDIR /app/cbalc
COPY . .
RUN make cbalc

CMD [ "./cbalc", "configdev.json" ]
