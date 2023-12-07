

# BRP - Brian's/Bad Retransmission Protocol


## Client Model

User threads call `read` and `write` to put raw bytes into the connection.
These bytes are then split into packets and sent off. Each packet is considered "sent"
when the peer acknowledges them by their sequence number.

## Resources

<https://blog.cloudflare.com/everything-you-ever-wanted-to-know-about-udp-sockets-but-were-afraid-to-ask-part-1/>
