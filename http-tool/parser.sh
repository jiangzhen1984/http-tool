

echo "=======================parse get http========="
sudo tcpdump -r /tmp/get.log  -s 384 -i any -nnq -tttt > /tmp/get_tmp.log
cut -c "11-21"  /tmp/get_tmp.log  |awk '{print $1}' |  sort | uniq -c  | awk '{print $2 " " $1}'
echo "=======================parse post http========="
sudo tcpdump -r /tmp/post.log  -s 384 -i any -nnq -tttt > /tmp/post_tmp.log
cut -c "11-21"  /tmp/post_tmp.log  |awk '{print $1}' |  sort | uniq -c  | awk '{print $2 " " $1}'
