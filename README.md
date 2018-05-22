# Networking_SlidingWindowProtocol

An implementation of the sliding window protocol using UDP. The sender and receiver are implemented seperately. This implementation can handle any network enviroment with its ability to detect packet loss and resend based on the network environment. 

Here a list of its essential performance:

• Send 50KB in 6 seconds on a normal link with no competition.

• Handle 5% loss: sent 1MB in 1.11790800094604 seconds.

• Handle 25% loss: sent 1MB in 6.75028300285339 seconds.

• Achieve a certain healthy fraction of ideal throughput on a normal link with no competition.

• Two instances can share a link roughly evenly (without taking
too long to converge).

• TCP friendliness.
