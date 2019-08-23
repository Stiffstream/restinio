# CA's certificate

Generation of CA:

```sh
openssl req -newkey rsa:2048 -keyform PEM -keyout ca.key -x509 -days 3650 -outform PEM -out ca.cer
```

# DH-parameters for the server

Generation of DH-parameters for the server:

```sh
openssl dhparam -out dh2048.pem 2048
```

# A certificate for the server

Generation of a key for the server:

```sh
openssl genrsa -out server.key 2048
```

Generation of a request for the server certificate:

```sh
openssl req -new -key server.key -out server.req -sha256
```

Signing of server certificate:

```sh
openssl x509 -req -in server.req -CA ca.cer -CAkey ca.key -set_serial 100 -extensions server -days 3650 -outform PEM -out server.cer -sha256
```

The removal of server's request:

```sh
rm server.req
```

# Clients' certificates

## Alice's certificate

Generation of a key for Alice:

```sh
openssl genrsa -out alice.key 2048
```

Generation of a request for Alice certificate:

```sh
openssl req -new -key alice.key -out alice.req
```

Signing of Alice client certificate:

```sh
openssl x509 -req -in alice.req -CA ca.cer -CAkey ca.key -set_serial 101 -extensions client -days 3650 -outform PEM -out alice.cer
```

## Bob's certificate

Generation of a key for Bob:

```sh
openssl genrsa -out bob.key 2048
```

Generation of a request for Bob certificate:

```sh
openssl req -new -key bob.key -out bob.req
```

Signing of Bob client certificate:

```sh
openssl x509 -req -in bob.req -CA ca.cer -CAkey ca.key -set_serial 101 -extensions client -days 3650 -outform PEM -out bob.cer
```

# Running the example

## Launching the server

The server can be started by a command like that:

```sh
./target/release/sample.tls_inspector sample/tls_inspector/certs
```

**NOTE!** It's important to pass a valid path to CA and server certificates.

## Issuing a client's request

A request from a client can be simulated via `curl`:

```sh
curl --cacert sample/tls_inspector/certs/ca.cer --cert sample/tls_inspector/certs/bob.cer --key sample/tls_inspector/certs/bob.key https://localhost:8080/limited

curl --cacert sample/tls_inspector/certs/ca.cer --cert sample/tls_inspector/certs/alice.cer --key sample/tls_inspector/certs/alice.key https://localhost:8080/limited
```

