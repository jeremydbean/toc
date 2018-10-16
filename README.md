Please see the WIKI for building documents.

CURRENT REPORTED BUGS:
	
- Game crashes at login if pfile is set with a (NULL) password.
	WORKAROUND: RESET PFILE PASSWORDS TO:  Kyz2D/BNiZB8Q~  (which translates to toc123)
	
- Game currently allows players to log in multiple times.  (Many copies of the character can be loaded.)  It appears this is related to 	(at least) failing to check code at line 2544 in comm.c.  






RASPBERRY PI PREREQUIREMENTS:


- sudo apt install csh
- sudo apt-get install build-essential
- sudo apt-get install telnet
- sudo apt update
- sudo apt upgrade



COPYING OVER TOC FROM GITHUB:
- sudo git clone http://www.github.com/jeremydbean/ToC.git
- sudo git config credential.helper store
- sudo git pull
	[Login is your GitHub account: username is email.)
	
- Should only be prompted once for login and password due to credential.helper




STARTING THE MUD:
- sudo chown -R pi:pi ToC
- sudo chmod -R 755 ToC
- cd ToC
- cd src
- rm *.o
- make
- cp merc ../area
- cd ../area
- ./merc 9000 &

	
	
TESTING THE MUD:
- telnet localhost 9000


WAYS TO MAKE MUD OPEN EASILY:
- sudo nano ~/.bashrc
- Add the following lines to the bottom
	- cd /home/pi/ToC/area
	- ./merc 9000 &
- Ctrl + X, Y, ENTER
- Every time you open a terminal window, you will be in the areas directory, and the MUD will attempt to start if not already running.

OTHER COMMANDS:
- ps ux : views all running processes.  First number listed after pi is the 'Process ID'  last line is the description.  The MUD is: 	- './merc 9000'
- If you need to force kill the MUD, use 'ps ux' to find the process ID for './merc 9000', then do 'kill <number>'
