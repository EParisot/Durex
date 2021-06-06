# Durex

C Backdoor. 
The Durex program creates a new Durex executable located in /bin and creates init_d / systemctl services (start on boot / process management).
The created program is packed in order to hide it's content until it's executed (and self unencrypted).
The created program will provide a shell command available to several clients (see defines in includes/payload.h).

A client is provided in order to use a secured handshake:
- a client asks for connexion
- the server sends a random 16 chars string to client
- the client encrypt this string (key used is password) and sends it back to server
- the server check the encrypted handshake received from client is correct and grants connexion.

Durex creates several files:
/bin/Durex
/etc/init.d/Durex
/etc/systemd/system/Durex.service

### compile:
```
make
```

### pack (optionnal):
```
./pack_payload.sh
```

### dump payloads:
Clean the includes/durex.h (from the comment to the end of file),
then:
```
./build_payloads.sh
```

### run server
```
sudo ./Durex 
```

### run client
```
./client/client [-h] [-s SERVER_IP] [-p SERVER_PORT] [-k KEY]

SERVER_IP default 		: 127.0.0.1
SERVER_PORT default 	: 4242
KEY (16 chars) default 	: secret__password
```

### remove
```
make fclean
```

