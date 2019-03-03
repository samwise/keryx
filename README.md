# Keryx
A high performance event distribution system.

Keryx is a event distribution system, that is a set of libraries and
daemons that can be used facilitate asynchronous event based
interaction between software components. Its main goals are:
  * Distribution transparency
  * Locality awareness
  * Sub-micro latency
  * Support for arbitrary overlap of publisher and subscriber life times 
  * Support for both publish/subscribe and request/reply interaction styles
  * Language independance
 
Some of Keryx's non-goals are:
  * Massive scalability. Keryx main niche are small to medium systems,
    that is systems with up to tens of nodes and thousands of event
    streams.
  * Secure operation. Keryx is intended for deployment within trusted environments.  
  
 

