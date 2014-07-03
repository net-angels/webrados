Webrados
---------
This software provides REST access to CEPH storage directly. 
It does not provide any extra possibilities, like RadosGateway,
just add(append mode), fetch, delete and stat.
Before installing, make sure you have CEPH installed and running (https://ceph.com/docs/master/rados/deployment/ceph-deploy-install).

Installation

		git clone https://github.com/net-angels/webrados.git
		cd webrados;
		apt-get install libconfig-dev libconfig<num>
		apt-get install librados-dev librados
		apt-get install libfcgi-dev libfcgi
		./makeinstall.sh
		vi /etc/nginx/sites-enabled/webrados.conf

NginX Sample config

		location / {
		client_max_body_size 20480m;
		fastcgi_pass unix:/tmp/webrados.sock;
		include fastcgi_params;
		}
		

WebRados config. This file does not exist. 
Create new file with following content 

		threads-count = 100;
		foreground = false;
		group = "www-data";
		user = "www-data";
		socket = "/tmp/webrados.sock";
		pidfile = "/var/run/webrados.pid";
		logfile = "/var/log/ceph/webrados.log";
		logging = true;
		ceph-user = "admin";
		rados-mount = ["bucket_allowed1","bucket_allowed"];

Run
		
		./webrados.fcgi -c /etc/webrados.conf -f /etc/ceph/ceph.conf
	
Usage
		
		Upload: curl -XPUT -T filename http://uploadhost.com/poolname/filename
		Download: curl -XGET http://uploadhost.com/poolname/filename
		Delete: curl -XDELETE http://uploadhost.com/poolname/filename
		Stat: curl -XOPTIONS http://uploadhost.com/poolname/filename

Dependencies 
		
		libpthread, libfcgi-dev,libfcgi,librados-dev,librados,libconfig-dev,libconfig
