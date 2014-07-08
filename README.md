WebRados
---------
This software provides REST access to CEPH storage directly. The goal of WebRados is to provide simple and fast access to RADOS. 
This is single tenant system, which allows website owners with large amount of static files effectively serve data stored in CEPH.                             
WebRados works directly with RADOS and provide access to CEPH's bare object-storage layer.

Unlike RadosGW it does not provide S3/Swift compatible API.

Supported modes are  fetch,  delete, stat

Before installing WebRados make sure you have CEPH installed and running (https://ceph.com/docs/master/rados/deployment/ceph-deploy-install).

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
		
WebRados config. Sample webrados.conf file can be in cloned directory
Adjust webrados.conf to meet your needs and copy to /etc

Run
		
		./webrados.fcgi -c /etc/webrados.conf -f /etc/ceph/ceph.conf
	
Usage
		
		Put     : curl -XPUT -T filename http://uploadhost.com/poolname/filename
		Get     : curl -XGET http://uploadhost.com/poolname/filename
		Deletee : curl -XDELETE http://uploadhost.com/poolname/filename
		Status  : curl -XOPTIONS http://uploadhost.com/poolname/filename

Dependencies 
		
		libpthread, libfcgi-dev,libfcgi,librados-dev,librados,libconfig-dev,libconfig
