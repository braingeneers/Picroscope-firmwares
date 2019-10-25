for i in $(seq $1 1 $2); do for j in {1..6}; do open http://camera$3$i$j.local/html/ ; done; done
