#### Get server root certificate

```bash
openssl s_client -showcerts -verify 5 -connect www.myurl.com:443 < /dev/null
```

Copy second certificate to `www_myurl_com.pem`

#### GitHub Rate Limiting



https://developer.github.com/v3/#rate-limiting