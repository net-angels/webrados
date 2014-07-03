WebRados
---------
This software provides REST access to CEPH storage directly. 
The goal of WebRados is to provide as simple and fast as possible acces to RADOS.
This is single tenant system which aims to help website owners with large ammount of statis file to effectively srore and retrieve data in reliable CEPH cluster. 
If you need something multi tenant and with more features plese refer to Ceph/RadosGateway.

Supported modes are  fetch,  delete, stat
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
		
		Put     : curl -XPUT -T filename http://uploadhost.com/poolname/filename
		Get     : curl -XGET http://uploadhost.com/poolname/filename
		Deletee : curl -XDELETE http://uploadhost.com/poolname/filename
		Status  : curl -XOPTIONS http://uploadhost.com/poolname/filename

Dependencies 
		
		libpthread, libfcgi-dev,libfcgi,librados-dev,librados,libconfig-dev,libconfig
