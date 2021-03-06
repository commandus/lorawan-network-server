
/* syslog
#define	LOG_ERROR							1
#define	LOG_INFO							2
#define	LOG_DEBUG							3
*/

#define LOG_UDP_EMITTER						1
#define LOG_UDP_LISTENER					2
#define LOG_IDENTITY_SVC					3
#define LOG_PACKET_HANDLER					4

#define LORA_OK            					0
#define ERR_CODE_COMMAND_LINE		    	-500                
#define ERR_CODE_OPEN_DEVICE		    	-501
#define ERR_CODE_CLOSE_DEVICE		    	-502
#define ERR_CODE_BAD_STATUS		        	-503
#define ERR_CODE_INVALID_PAR_LOG_FILE   	-504
#define ERR_CODE_INVALID_SERVICE			-505
#define ERR_CODE_INVALID_GATEWAY_ID			-506
#define ERR_CODE_INVALID_DEVICE_EUI			-507
#define ERR_CODE_INVALID_BUFFER_SIZE		-508
#define ERR_CODE_GRPC_NETWORK_SERVER_FAIL	-509
#define ERR_CODE_INVALID_RFM_HEADER			-510
#define ERR_CODE_INVALID_ADDRESS			-511
#define ERR_CODE_INVALID_FAMILY				-512
#define ERR_CODE_SOCKET_CREATE		    	-513
#define ERR_CODE_SOCKET_BIND		    	-514
#define ERR_CODE_SOCKET_OPEN		    	-515
#define ERR_CODE_SOCKET_CLOSE		    	-516
#define ERR_CODE_SOCKET_READ		    	-517
#define ERR_CODE_SOCKET_WRITE		    	-518
#define ERR_CODE_SOCKET_NO_ONE				-519
#define ERR_CODE_SELECT						-520
#define ERR_CODE_INVALID_PACKET				-521
#define ERR_CODE_INVALID_JSON				-522
#define ERR_CODE_DEVICE_ADDRESS_NOTFOUND	-523
#define ERR_CODE_FAIL_IDENTITY_SERVICE		-524
#define ERR_CODE_LMDB_TXN_BEGIN				-525
#define ERR_CODE_LMDB_TXN_COMMIT			-526
#define ERR_CODE_LMDB_OPEN					-527
#define ERR_CODE_LMDB_CLOSE					-528
#define ERR_CODE_LMDB_PUT					-529
#define ERR_CODE_LMDB_PUT_PROBE				-530
#define ERR_CODE_LMDB_GET					-531
#define ERR_CODE_WRONG_PARAM				-532
#define ERR_CODE_INSUFFICIENT_MEMORY		-533
#define ERR_CODE_NO_CONFIG					-534
#define ERR_CODE_SEND_ACK					-535
#define ERR_CODE_NO_GATEWAY_STAT			-536
#define ERR_CODE_INVALID_PROTOCOL_VERSION	-537
#define ERR_CODE_PACKET_TOO_SHORT			-538
#define ERR_CODE_PARAM_NO_INTERFACE			-539
#define ERR_CODE_MAC_TOO_SHORT				-540
#define ERR_CODE_MAC_INVALID				-541
#define ERR_CODE_MAC_UNKNOWN_EXTENSION		-542
#define ERR_CODE_PARAM_INVALID				-543
#define ERR_CODE_INSUFFICIENT_PARAMS		-544
#define ERR_CODE_NO_MAC_NO_PAYLOAD          -545
#define ERR_CODE_INVALID_REGEX              -546
#define ERR_CODE_NO_DATABASE               	-547
#define ERR_CODE_LOAD_PROTO					-548
#define ERR_CODE_LOAD_DATABASE_CONFIG		-549
#define ERR_CODE_DB_SELECT					-550
#define ERR_CODE_DB_DATABASE_NOT_FOUND		-551
#define ERR_CODE_DB_DATABASE_OPEN			-552
#define ERR_CODE_DB_DATABASE_CLOSE			-553
#define ERR_CODE_DB_CREATE					-554
#define ERR_CODE_DB_INSERT					-555
#define ERR_CODE_DB_START_TRANSACTION		-556
#define ERR_CODE_DB_COMMIT_TRANSACTION		-557
#define ERR_CODE_DB_EXEC					-558
#define ERR_CODE_PING						-559
#define ERR_CODE_INVALID_STAT				-560
#define ERR_CODE_NO_PAYLOAD          		-561
#define ERR_CODE_NO_MESSAGE_TYPE			-562
#define ERR_CODE_QUEUE_EMPTY				-563
#define ERR_CODE_RM_FILE					-564
#define ERR_CODE_INVALID_BASE64				-565

#define ERR_MESSAGE						"Error "
#define ERR_DEBUG						"Info "
#define ERR_WARNING						"Warning "
#define ERR_TIMEOUT						"Timeout"

#define ERR_COMMAND_LINE        		"Wrong parameter(s)"
#define ERR_OPEN_DEVICE         		"open error "
#define ERR_CLOSE_DEVICE        		"close error "
#define ERR_BAD_STATUS          		"Bad status"
#define ERR_INVALID_PAR_LOG_FILE		"Invalid log file "
#define ERR_GET_ADDRESS					"get address info error "
#define ERR_BIND						"Can not bind "
#define ERR_INVALID_SERVICE				"Invalid service address "
#define ERR_INVALID_GATEWAY_ID			"Invalid gateway identifier"
#define ERR_INVALID_DEVICE_EUI			"Invalid device EUI"
#define ERR_INVALID_BUFFER_SIZE			"Invalid buffer size "
#define ERR_GRPC_NETWORK_SERVER_FAIL	"gRPC network server error "
#define ERR_INVALID_RFM_HEADER			"Invalid RFM header"
#define ERR_INVALID_ADDRESS				"Invalid address"
#define ERR_INVALID_FAMILY				"Invalid address family"
#define ERR_SOCKET_CREATE		    	"Can not create socket"
#define ERR_SOCKET_BIND		    		"Can not bind socket "
#define ERR_SOCKET_OPEN		    		"Can not open socket"
#define ERR_SOCKET_CLOSE		    	"Can not close socket"
#define ERR_SOCKET_READ		    		"Can not read socket"
#define ERR_SOCKET_WRITE		    	"Can not write socket"
#define ERR_SOCKET_NO_ONE				"No sockets"
#define ERR_SELECT						"Select error"
#define ERR_INVALID_PACKET				"Invalid packet"
#define ERR_INVALID_JSON				"Invalid JSON"
#define ERR_DEVICE_ADDRESS_NOTFOUND		"Device not found, address is not registered"
#define ERR_FAIL_IDENTITY_SERVICE		"Identity service failure"
#define ERR_LMDB_TXN_BEGIN				"Can not begin LMDB transaction "
#define ERR_LMDB_TXN_COMMIT				"Can not commit LMDB transaction "
#define ERR_LMDB_OPEN					"Can not open database file "
#define ERR_LMDB_CLOSE					"Can not close database file "
#define ERR_LMDB_PUT					"Can not put LMDB "
#define ERR_LMDB_PUT_PROBE				"Can not put LMDB probe "
#define ERR_LMDB_GET					"Can not get LMDB "
#define ERR_WRONG_PARAM					"Wrong parameter"
#define ERR_INSUFFICIENT_MEMORY			"Insufficient memory"
#define ERR_NO_CONFIG					"No config is provided"
#define ERR_SEND_ACK					"Send ACK error "
#define ERR_NO_GATEWAY_STAT				"No gateway statistics provided"
#define ERR_INVALID_PROTOCOL_VERSION	"Invalid protocol version"
#define ERR_PACKET_TOO_SHORT			"Protocol packet is too short"
#define ERR_PARAM_NO_INTERFACE			"No interface specified"
#define ERR_MAC_TOO_SHORT				"MAC is too short"
#define ERR_MAC_INVALID					"Invalid MAC command"
#define ERR_MAC_UNKNOWN_EXTENSION		"Unknown MAC command extension"
#define ERR_PARAM_INVALID				"Invalid paramater"
#define ERR_INSUFFICIENT_PARAMS			"Insufficient parameters"
#define ERR_NO_MAC_NO_PAYLOAD           "No MAC command(s) and/or payload"
#define ERR_INVALID_REGEX               "Invalid regular expression"
#define ERR_NO_DATABASE               	"No database support enabled"
#define ERR_LOAD_PROTO					"Error loading proto files"
#define ERR_LOAD_DATABASE_CONFIG		"Error loading database config file"
#define ERR_DB_SELECT					"Error SQL select from table "
#define ERR_DB_DATABASE_NOT_FOUND		"Database not found "
#define ERR_DB_DATABASE_OPEN			"Error open database "
#define ERR_DB_DATABASE_CLOSE			"Error close database "
#define ERR_DB_CREATE					"Error create SQL table "
#define ERR_DB_INSERT					"Error insert record into SQL table "
#define ERR_DB_START_TRANSACTION		"Error start SQL transaction"
#define ERR_DB_COMMIT_TRANSACTION		"Error commit SQL transaction"
#define ERR_DB_EXEC						"Error exec SQL statement"
#define ERR_PING						"Ping"
#define ERR_INVALID_STAT				"Invalid stat"
#define ERR_NO_PAYLOAD          		"No payload"
#define ERR_NO_MESSAGE_TYPE				"No message type"
#define ERR_QUEUE_EMPTY					"Message queue is empty"
#define ERR_RM_FILE						"Can not delete file "
#define ERR_INVALID_BASE64				"Input is not valid base64-encoded data"

#define MSG_PROG_NAME					"LoRaWAN network listener"
#define MSG_PROTO_DB_PROG_NAME			"proto-db helper utility"
#define MSG_INTERRUPTED 				"Interrupted "
#define MSG_GRACEFULLY_STOPPED			"Stopped gracefully"
#define MSG_PG_CONNECTED        		"Connected"
#define MSG_PG_CONNECTING       		"Connecting..."
#define MSG_DAEMON_STARTED      		"Start daemon "
#define MSG_DAEMON_STARTED_1    		". Check syslog."
#define MSG_WS_TIMEOUT					"Web service time out"
#define MSG_TIMEOUT						"Timeout"
#define MSG_STOPPED						"ChirpStack logger stopped gracefully"
#define MSG_CHIRPSTACK_SERVER_VERSION	"ChirpStack network server version "
#define MSG_GATEWAY						"Gateway "
#define MSG_DEVICE						"Device "
#define MSG_DEVICE_EUI					"Device EUI "
#define MSG_DEVICE_ACTIVATION			"Device activation "
#define MSG_READ_BYTES					"Read bytes "
#define MSG_RECEIVED					"Message received "
#define MSG_SENT_ACK_TO					"Sent ACK to "
#define MSG_GATEWAY_STAT				"Gateway statistics "
#define MSG_RXPK						"rxpk "
#define MSG_MAC_COMMANDS                "MAC commands"
#define MSG_SEND_TO                     "Send to "
#define MSG_DATABASE_LIST				"Databases: "
#define MSG_CONN_ESTABLISHED			 "established"
#define MSG_CONN_FAILED					"failed"
#define MSG_CONNECTION					"connection "

#define ERROR							"Error "

const char *strerror_client(int errcode);
