aws --profile prp-braingeneers --endpoint https://s3-west.nrp-nautilus.io s3 cp /home/pi/Pictures/$1 s3://braingeneers/imaging/$1/images --acl public-read --recursive 
