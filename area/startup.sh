#!/bin/csh
# Written by Furey.
# With additions from Tony and Alander.

# Set the port number.
set port = 9000
if ( "$1" != "" ) set port="$1"

# Set limits.
if ( -e shutdown.txt ) rm -f shutdown.txt

while ( 1 )
    # If you want to have logs in a different directory,
    #   change the 'set logfile' line to reflect the directory name.
    set index = 1000
    while ( 1 )
	set logfile = ../log/$index.log
	if ( ! -e $logfile ) break
	@ index++
    end




    # Restart, giving old connections a chance to die.
    if ( -e shutdown.txt ) then
	rm -f shutdown.txt
	exit 0
    endif
    sleep 15
end




    # Run rom.
    # Check if already running
    set matches = `ps ux | grep ".merc 9000" | grep -c grep`
    if ( $matches >= 1 ) then
        # Already running
        echo MUD Already running.
        exit 0
    endif
	
	# Run rom.
	cd /mnt/e/toc/area/
    immortal ./merc $port >&! $logfile
