aws --profile prp --endpoint https://s3.nautilus.optiputer.net s3 cp /home/pi/Pictures/$1 s3://braingeneers/imaging/$1/images --acl public-read --recursive 

