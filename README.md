Please see the WIKI for building documents and Raspberry Pi install info.

PREVIOUSLY REPORTED BUGS:

- Game crashes at login if pfile is set with a (NULL) password.
	WORKAROUND: RESET PFILE PASSWORDS TO:  Kyz2D/BNiZB8Q~  (which translates to toc123)

<strike>- Game currently allows players to log in multiple times.  (Many copies of the character can be loaded.)  It appears this is related to 	(at least) failing to check code at line 2544 in comm.c.  </strike>

sudo chmod -R 777 toc
sudo chmod a+rwx -R toc
