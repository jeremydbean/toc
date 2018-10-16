Please see the WIKI for building documents.

CURRENT REPORTED BUGS:
	
- Game crashes at login if pfile is set with a (NULL) password.
	WORKAROUND: RESET PFILE PASSWORDS TO:  Kyz2D/BNiZB8Q~  (which translates to toc123)
	
- Game currently allows players to log in multiple times.  (Many copies of the character can be loaded.)  It appears this is related to 	(at least) failing to check code at line 2544 in comm.c.  






RASPBERRY PI PREREQUIREMENTS:


- sudo apt install csh
- sudo apt-get install build-essential
- sudo apt update
- sudo apt upgrade



COPYING OVER TOC FROM GITHUB:
- sudo git clone http://www.github.com/jeremydbean/ToC.git
- sudo git config credential.helper store
- sudo git pull
	[Login is your GitHub account: username is email.)
	
- Should only be prompted once for login and password due to credential.helper




STARTING THE MUD:
- sudo chown -hR pi ToCsudo 
- cd ToC
- cd src
- rm *.o
- make
- cp merc ../area
- cd ../area
- ./startup &
	- NOTE: If errors, you can try typing './merc 9000 &' from areas folder.
	
	
TESTING THE MUD:
- telnet localhost 9000
