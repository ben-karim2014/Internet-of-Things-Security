![Hits](https://hits.seeyoufarm.com/api/count/incr/badge.svg?url=https%3A%2F%2Fgithub.com%2Fkarim-benlghalia%2FInternet-of-Things-Security%2Fblob%2Fmain%2FREADME.md&count_bg=%2379C83D&title_bg=%23555555&icon=&icon_color=%23E7E7E7&title=hits&edge_flat=false)
# Internet-of-Things-Security
Embedded application in C language that interacts with a central control server with the aid of server-side logs. 
The application read data (Temperature) from an external sensor and reports back to a network server over both unencrypted and 
encrypted channels.

FILES: 

lab4c-tcp.c 
	Builds and runs on a Beaglebone. Based on a temperature sensor application.
	Accepts the following parameters:
	--period=#
	--scale= ('C' or 'F')
	--id=9-digit-number (mandatory)
	--host=name or address (mandatory)
	--log=filename (mandatory)
	port number (mandatory)

	1.Opens a TCP connection to the server at the specified address and port
	2.immediately send (and log) an ID terminated with a newline:
	ID=ID-number. This new report enables the server to keep track of which
	devices it has received reports from. 
	3.Send (and log) newline terminated temperature reports over the connections. 
	4.Process (and log) newline-terminated commands received over the connection. 
	If temperature reports are mis-formatted, the server will return a LOG command with a description 
	of the error. 
	5.The last command sent by the server will be an OFF.

lab4b-tls.c
	Builds and runs on a Beaglebone. 
	Based on the remote logging appliance built from lab4c-tcp.c, operates by:
	1. opening a TLS connection to the server at the specified address and port
	2. sending (and logging) a student ID followed by a newline
	3. sending (and logging) temperature reports over the connection
	4. processing (and logging) commands received over the connection

Makefile
	3 targets: 
	default ... build both versions of the program
	clean ... delete all programs and output created by the Makefile and restore the directory to its freshly untarred state.
	dist ... create the deliverable tarball.
