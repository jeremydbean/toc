#!/bin/csh -v
# Written by Killuminati.


sudo apt update 
sudo apt upgrade 
sudo apt-get update
sudo apt install csh 
sudo apt-get install build-essential 
sudo apt-get install telnet 
sudo apt-get install git-all
sudo apt upgrade
sudo apt autoremove
sudo apt-get install dos2unix
sudo apt install git-sh
sudo apt install ssh
sudo apt install openssh-server
#sudo service ssh start

#Get ToC files
	#sudo git clone http://www.github.com/jeremydbean/ToC.git
	#sudo git config credential.helper store

git clone git@github.com:jeremydbean/toc.git
sudo chown toc toc
sudo chmod -R 777 toc
cd toc
dos2unix startup
dos2unix cleanup
dos2unix install.sh
sudo chmod a+x startup
sudo chmod a+x cleanup
