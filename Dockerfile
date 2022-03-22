FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
	build-essential \
	git \
	libcurl4-openssl-dev \
	&& rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/cogmasters/concord.git /app/concord
WORKDIR /app/concord
RUN make && make install

WORKDIR /app/cbalc
COPY . .
RUN make cbalc

CMD [ "./cbalc", "configdev.json" ]
