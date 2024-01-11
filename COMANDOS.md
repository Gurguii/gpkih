## [NEW]

#### [CLIENT/SERVER] Key + request + CA signed certificate

*Create request + key*

```bash
openssl req \
-newkey rsa:2048 \
-out req.pem \
-keyout key.pem \
-subj '/CN=test' \
-noenc
```

*Create signed certificate*

```bash
openssl ca \
-config gopenssl.cnf \
-in req.pem \
-out crt.pem \
```
