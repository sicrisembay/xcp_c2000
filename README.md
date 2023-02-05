# XCP Slave Example on C2000
This is a sample project of [XCP](https://en.wikipedia.org/wiki/XCP_(protocol)) slave.  TMS320F28335 microcontroller is used as a target slave.  Unlike most MUC, this MCU has a byte definition of 16-bit.

As a starting point, the XCP slave protocol implementation is based from XcpBasic (basic version) from Vector Informatik GmbH.  It is modified so it supports C2000 architecture and address granularity of WORD.  