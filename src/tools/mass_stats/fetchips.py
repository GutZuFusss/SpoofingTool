import urllib2
import time

response = urllib2.urlopen("https://stats.teeworlds.tw/last_refreshes.php?time=60")
page_source = response.read()

f = open("ips.txt","w")
f.write(page_source)
print "Fetched IPs successfully!"
f.close()
num_lines = sum(1 for line in open('ips.txt'))
print "Total IPs fetched: "+str(num_lines)

time.sleep(5)