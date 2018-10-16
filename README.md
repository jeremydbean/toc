Please see the WIKI for building documents.

CURRENT REPORTED BUGS:
	
- Game crashes at login if pfile is set with a (NULL) password.
	WORKAROUND: RESET PFILE PASSWORDS TO:  Kyz2D/BNiZB8Q~  (which translates to toc123)
	
- Game currently allows players to log in multiple times.  (Many copies of the character can be loaded.)  It appears this is related to 	(at least) failing to check code at line 2544 in comm.c.  




Raspberry Pi:

- sudo apt update
- sudo apt upgrade
- sudo apt install csh
- sudo apt-get install build-essential

- sudo git clone http://www.github.com/jeremydbean/ToC.git
- sudo git config credential.helper store
	- sudo git pull
	- sudo git push
