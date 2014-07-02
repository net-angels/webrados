Webrados
---------
This software provides REST access to CEPH storage directly. 
It does not provide any extra possibilities, like RadosGateway,
just add(append mode), fetch, delete and stat.

	USAGE
		Upload: curl -XPUT -T filename http://uploadhost.com/poolname/filename
		Download: curl -XGET http://uploadhost.com/poolname/filename
		Delete: curl -XDELETE http://uploadhost.com/poolname/filename
		Stat: curl -XOPTIONS http://uploadhost.com/poolname/filename

	DEPENDENCIES
		libpthread
		libfcgi
		librados
		libconfig
