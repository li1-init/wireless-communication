# Third-party notices

`User/tweetnacl.c` and `User/tweetnacl.h` come from TweetNaCl
(<https://tweetnacl.cr.yp.to/>), a compact public-domain implementation of
NaCl's Curve25519-XSalsa20-Poly1305 primitives.

The source files are kept unmodified. Project-specific random-number,
packet-protocol and storage code is in `link_crypto.c`, `link_protocol.c`
and the peer-store modules.
