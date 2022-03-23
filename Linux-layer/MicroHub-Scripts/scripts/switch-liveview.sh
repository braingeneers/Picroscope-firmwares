sudo sed -i "57 c\ \t\tproxy_pass http:\/\/$1.local/html/;" /etc/nginx/sites-enabled/default
sudo nginx -s reload
