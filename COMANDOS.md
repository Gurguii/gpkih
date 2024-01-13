## [Self signed CA]
*Create key+cert*
```bash
openssl req \
-config gopenssl.cnf \
-new \
-x509 \
-out pki/ca/ca-crt.pem \
-keyout pki/ca/ca-key.pem \
-subj '/CN=gurguiCA' \
-noenc
```

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

*Sign request with CA and create signed certificate*

```bash
openssl ca \
-config gopenssl.cnf \
-in req.pem \
-out crt.pem \
```
