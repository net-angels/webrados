WebRados
---------
WebRados provides simple and fast access to CEPH, it works directly with librados  and access CEPH's via bare object-storage layer.
This is single tenant system, which allows website owners with large amount of static files effectively serve data stored in CEPH.                             

Unlike RadosGW it does not provide S3/Swift compatible API.
Access API is primitive as possible and supports fetch,  delete, stat

Before installing WebRados make sure you have CEPH installed and running (https://ceph.com/docs/master/rados/deployment/ceph-deploy-install).

Installation

		git clone https://github.com/net-angels/webrados.git
		cd webrados;
		apt-get install libconfig-dev libconfig<num>
		apt-get install librados-dev librados
		apt-get install libfcgi-dev libfcgi
		./makeinstall.sh

Script will build webrados.fcgi and put it to webrados/bin/ directory
After compile is completed you can copy webrados.fcgi to any servers and run. 
Binary file requires libconfig and librados to be installed on destination servers.

NginX Sample config. 
Some of parameters like cache and access restrictions are not mandatory, but we think that having these is always a good idea. 

	fastcgi_cache_path /opt/nginx_cache/cache  keys_zone=one:720m  loader_threshold=500 loader_files=2000 max_size=2048000m;
	fastcgi_cache_key "$scheme$request_method$host$request_uri";
	server {
        	server_name _;
	        listen   80;
        	root    /var/www;

	        location = /favicon.ico {
        	        access_log off;
	                log_not_found off;
        	}

	        location / {
        	        fastcgi_keep_conn on;
                	client_max_body_size 20480m;
	                fastcgi_pass unix:/tmp/webrados.sock;
        	        include fastcgi_params;
                	        limit_except GET {
                                	allow 192.168.50.0/24;
	                                allow 127.0.0.1;
        	                        deny all;
                	            }
	        }
        	fastcgi_cache_min_uses 1;
	        fastcgi_cache one;
	        fastcgi_cache_valid  200 302 304 720m;
	        fastcgi_cache_valid  404 10m;
	        fastcgi_cache_valid  301 720m;
	        fastcgi_cache_valid  any 720m;

	}


WebRados config. Sample webrados.conf file. 

	threads-count = 100;
	foreground = false;
	group = "www-data";
	user = "www-data";
	socket = "/tmp/webrados.sock";
	pidfile = "/var/run/webrados.pid";
	logfile = "/var/log/ceph/webrados.log";
	logging = true;
	ceph-user = "admin";
	rados-mount = ["StarWars","pics","pics2","test"];

Be sure to create rados-mount pools before running webrados.fcgi
	
	rados mkpool StarWars 
	rados mkpool foo
	rados mkpool blah

Adjust webrados.conf to meet your needs and copy to /etc

Run
		
		./webrados.fcgi -c /etc/webrados.conf -f /etc/ceph/ceph.conf
	
Usage
		
		Upload       : curl -XPUT -T filename http://uploadhost.com/poolname/filename
		Download     : curl -XGET http://uploadhost.com/poolname/filename
		Delete       : curl -XDELETE http://uploadhost.com/poolname/filename
		File Status  : curl -XOPTIONS http://uploadhost.com/poolname/filename
		Server Stats : curl -XGET http://uploadhost.com/?status

Dependencies 
		
		libpthread, libfcgi-dev,libfcgi,librados-dev,librados,libconfig-dev,libconfig


