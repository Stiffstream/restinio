#!/bin/bash
cat <(echo -n $'{\n\t"random_data": "') \
<(tr -dc "A-Za-z0-9 " </dev/urandom | head -c 1024 ) \
<(echo -n $'",\n\t"purpose": "This JSON was generated to test RESTinio decompression"\n}') \
| gzip -cf \
| curl -v \
	-H "Content-Type: application/json" -H "Content-Encoding: gzip" \
	-X POST --data-binary @- http://127.0.0.1:8080/


