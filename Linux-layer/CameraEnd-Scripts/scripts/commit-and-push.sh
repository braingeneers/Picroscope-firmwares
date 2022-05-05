cd /home/pi/Picroscope-firmwares &&
git stash &&
git pull &&
git stash apply &&
git commit -a -m "updated flows" &&
git push -u origin master
