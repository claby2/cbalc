FROM alpine:3.12

RUN apk add --update --no-cache curl-dev git make gcc musl-dev

RUN git clone https://github.com/cogmasters/concord.git /app/concord
WORKDIR /app/concord
RUN make && make install

WORKDIR /app/cbalc
COPY . .
RUN make cbalc

CMD [ "./cbalc", "configdev.json" ]
