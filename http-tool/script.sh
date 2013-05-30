#

IP=`ip addr list eth0 |grep "inet " |cut -d' ' -f6|cut -d/ -f1`
PORT=80
#sudo tcpdump  -s 0 -q  'src '$IP'  and tcp dst port '$PORT' and (tcp[((tcp[12:1] & 0xf0) >> 2):4] = 0x47455420)' -w /tmp/get.log 
sudo tcpdump  -s 0 -q   'src '$IP'  and tcp dst port '$PORT' and (tcp[((tcp[12:1] & 0xf0) >> 2):4] = 0x504f5354)'  -w /tmp/post.log 


