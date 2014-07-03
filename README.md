Webrados
---------
This software provides REST access to CEPH storage directly. 
It does not provide any extra possibilities, like RadosGateway,
just add(append mode), fetch, delete and stat.
Before installing, make sure you have CEPH installed and running (hint: http://reborns.info/ustanovka-ceph-storage-cluster-za-15-minut/).

INSTALL

		git clone https://github.com/net-angels/webrados.git
		cd webrados;
		apt-get install libconfig-dev libconfig<num>
		apt-get install librados-dev librados
		apt-get install libfcgi-dev libfcgi
		./makeinstall.sh
		vi /etc/nginx/sites-enabled/webrados.conf
		#add/modify these lines
		location / {
		client_max_body_size 20480m;
		fastcgi_pass unix:/tmp/webrados.sock;
		include fastcgi_params;
		}
		vi /etc/uploader.conf
		#add lines below
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

RUN
		./radosweb.fcgi -c /etc/uploader.conf -f /etc/ceph/ceph.conf
	
USAGE
		
		Upload: curl -XPUT -T filename http://uploadhost.com/poolname/filename
		Download: curl -XGET http://uploadhost.com/poolname/filename
		Delete: curl -XDELETE http://uploadhost.com/poolname/filename
		Stat: curl -XOPTIONS http://uploadhost.com/poolname/filename

DEPENDENCIES
		libpthread, libfcgi-dev,libfcgi,librados-dev,librados,libconfig-dev,libconfig
