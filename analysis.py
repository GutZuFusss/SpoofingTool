import mysql.connector    
import os
import atexit

FIFO = 'IPs.log'

cnx = mysql.connector.connect(user='analyser', password='1234567890',
                              host='127.0.0.1',
                              database='analysis')

try:
	cursor = cnx.cursor()
	cursor.execute("""    
	CREATE TABLE IF NOT EXISTS requests
	(
	  id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	  ip VARCHAR (128) UNIQUE NOT NULL,
	  lastest_port INT NOT NULL,
	  num_requests INT NOT NULL
	);
   """)

	if os.path.exists("IPs.log"):
		os.remove("IPs.log")

	os.mkfifo(FIFO)
	while True:
		fifo = open(FIFO, "rw")
		for line in fifo:
			ip = line.split(":")
			raw_ip = ip[0]
			raw_port = ip[1]
			query = ("INSERT INTO requests (ip, lastest_port, num_requests) VALUES (%s, %s, %s) ON DUPLICATE KEY UPDATE lastest_port = %s, num_requests = num_requests + 1")
			cursor.execute(query, (raw_ip, int(raw_port), 1, int(raw_port)))
			cnx.commit()
		fifo.close()
finally:
	cnx.close()
