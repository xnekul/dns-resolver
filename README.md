# Description:

This project implements client for dns protocol. It can send query about message based on the arguments to selected dns server and display answer for that query. This program implements all basic functionalities for dns resolver. That is: quering and understanding A and AAAA resource types, being able to conduct reverse query.

# Implemented extensions:

As extensions program is able to query and understand following resource types: MX, NS, CNAME, TXT, HINFO

# Known limitatios:

no/limited IPv6 support
possible program failure upon receiving malformed packet
sufficient but not perfect argument parsing
there are sometimes false negative tests due to the not perfect comparison of outcome and expected outcome

# Example of program use:

./dns -r -s [dns-server] [query]

# Example of program answer:

```
Authoritative: Yes, Recursive: No, Truncated: No,
Question section (1)
www.fit.vut.cz. A, IN,

Answer section (1)
www.fit.vut.cz. A, IN, 14400, 147.229.9.26

Authority section (0)

Additional section (0)
```

# Help page:

```
Usage: dns [-r] [type] -s server [-p port] address

Order of arguments is not set. Description of arguments:

    -r: Recursion Desired, else without recursion.
    type: specifies type of the query, by default A type record is queried
        -6 for AAAA,
        -x for reverse IPv4 query
        -mx for MX
        -cname for CNAME
        -ns for NS
        -txt for TXT
        -hinfo for HINFO
    -s: IP address or domain name of server where should be query send.
    -p port: Destination port number, default is 53
    address: Queried address.

```
