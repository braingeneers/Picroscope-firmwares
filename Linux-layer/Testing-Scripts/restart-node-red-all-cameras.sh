for i in {1..4} ; do for j in {1..6}; do ssh pi@camera$1$i$j.local node-red-restart ; done; done
