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
openssl ca -in .\client.req -out .\client-crt -extfile .\x509\client -subj '/CN=gurguiCLIENT' -config .\gopenssl.cnf
```bash
openssl ca \
-config gopenssl.cnf \
-extfile 
-in req.pem \
-out crt.pem \
-subj '/CN=gurguito' \
-extfile x509/<client|server> \

```

### Revoking certificates  
*Revoke a certificate*
```bash
openssl ca \
-config gopenssl.cnf \
-revoke crl.pem \
-crl_reason "bad client" \
-crldays <days> \
-crlhours <hours> \
-crlsec <secs>
```  
*Generate a new crl (certificate revocation list)*
```bash
openssl ca \
-config gopenssl.cnf \
-gencrl \
-out crl.pem
```
