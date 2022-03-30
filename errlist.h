
// syslog
#ifdef _MSC_VER
#define	LOG_ERR								1
#define	LOG_INFO							2
#define	LOG_DEBUG							3
#endif

// width in chars
#define LOG_LEVEL_FIELD_WIDTH               2

#define LOG_MAIN_FUNC						1
#define LOG_UDP_LISTENER					2
#define LOG_IDENTITY_SVC					3
#define LOG_PACKET_HANDLER					4
#define LOG_PACKET_QUEUE					5
#define LOG_UDP_EMITTER						6

// Error codes
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
#define ERR_CODE_PULLOUT					-560
#define ERR_CODE_INVALID_STAT				-561
#define ERR_CODE_NO_PAYLOAD          		-562
#define ERR_CODE_NO_MESSAGE_TYPE			-563
#define ERR_CODE_QUEUE_EMPTY				-564
#define ERR_CODE_RM_FILE					-565
#define ERR_CODE_INVALID_BASE64				-566
#define ERR_CODE_MISSED_DEVICE				-567
#define ERR_CODE_MISSED_GATEWAY				-568
#define ERR_CODE_INVALID_FPORT				-569
#define ERR_CODE_INVALID_MIC				-570
#define ERR_CODE_SEGMENTATION_FAULT			-571
#define ERR_CODE_ABRT           			-572
#define ERR_CODE_BEST_GATEWAY_NOT_FOUND		-573
#define ERR_CODE_REPLY_MAC					-574
#define ERR_CODE_NO_MAC			           	-575
#define ERR_CODE_NO_DEVICE_STAT				-576
#define ERR_CODE_INIT_DEVICE_STAT			-577
#define ERR_CODE_INIT_IDENTITY				-578
#define ERR_CODE_INIT_QUEUE					-579
#define ERR_CODE_HANGUP_DETECTED			-580
#define ERR_CODE_NO_FCNT_DOWN				-581
#define ERR_CODE_CONTROL_NOT_AUTHORIZED		-582
#define ERR_CODE_GATEWAY_NOT_FOUND			-583
#define ERR_CODE_CONTROL_DEVICE_NOT_FOUND	-584
#define ERR_CODE_INVALID_CONTROL_PACKET		-585
#define ERR_CODE_DUPLICATED_PACKET			-586
#define ERR_CODE_INIT_GW_STAT   			-587
#define ERR_CODE_DEVICE_NAME_NOT_FOUND      -588
#define ERR_CODE_DEVICE_EUI_NOT_FOUND       -589
#define ERR_CODE_JOIN_EUI_NOT_MATCHED       -590
#define ERR_CODE_GATEWAY_NO_YET_PULL_DATA   -591
#define ERR_CODE_REGION_BAND_EMPTY          -592
#define ERR_CODE_INIT_REGION_BANDS          -593
#define ERR_CODE_INIT_REGION_NO_DEFAULT     -594
#define ERR_CODE_NO_REGION_BAND             -595
#define ERR_CODE_REGION_BAND_NO_DEFAULT     -596
#define ERR_CODE_IS_JOIN					-597
#define ERR_CODE_BAD_JOIN_REQUEST			-598
#define ERR_CODE_NETID_OR_NETTYPE_MISSED    -599
#define ERR_CODE_NETTYPE_OUT_OF_RANGE       -600
#define ERR_CODE_NETID_OUT_OF_RANGE         -601
#define ERR_CODE_TYPE_OUT_OF_RANGE          -602
#define ERR_CODE_NWK_OUT_OF_RANGE           -603
#define ERR_CODE_ADDR_OUT_OF_RANGE          -604
#define ERR_CODE_ADDR_SPACE_FULL            -605
#define ERR_CODE_INIT_LOGGER_HUFFMAN_PARSER -606

#define ERR_MESSAGE						"Error "
#define ERR_DEBUG						"Info "
#define ERR_WARNING						"Warning "
#define ERR_TIMEOUT						"Timeout"

// Error en-us description
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
#define ERR_PACKET_TOO_SHORT			"Semtech protocol packet is too short"
#define ERR_PARAM_NO_INTERFACE			"No interface specified"
#define ERR_MAC_TOO_SHORT				"MAC is too short"
#define ERR_MAC_INVALID					"Invalid MAC command"
#define ERR_MAC_UNKNOWN_EXTENSION		"Unknown MAC command extension"
#define ERR_PARAM_INVALID				"Invalid parameter"
#define ERR_INSUFFICIENT_PARAMS			"Insufficient parameters"
#define ERR_NO_MAC_NO_PAYLOAD           "No MAC command(s) and/or payload"
#define ERR_INVALID_REGEX               "Invalid regular expression"
#define ERR_NO_DATABASE               	"No database support enabled"
#define ERR_LOAD_PROTO					"Error loading proto files"
#define ERR_LOAD_DATABASE_CONFIG		"Database config file does not exists, invalid or empty"
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
#define ERR_PULLOUT						"Pullout"
#define ERR_INVALID_STAT				"Invalid stat"
#define ERR_NO_PAYLOAD          		"No payload"
#define ERR_NO_MESSAGE_TYPE				"No message type"
#define ERR_QUEUE_EMPTY					"Message queue is empty"
#define ERR_RM_FILE						"Can not delete file "
#define ERR_INVALID_BASE64				"Input is not valid base64-encoded data"
#define ERR_MISSED_DEVICE				"No device specified"
#define ERR_MISSED_GATEWAY				"No gateway specified"
#define ERR_INVALID_FPORT				"Invalid FPort value"
#define ERR_INVALID_MIC					"Invalid MIC"
#define ERR_SEGMENTATION_FAULT			"Segmentation fault"
#define ERR_ABRT                        "Abnormal program termination"
#define ERR_BEST_GATEWAY_NOT_FOUND		"Best gateway not found"
#define ERR_REPLY_MAC					"Can not send MAC reply"
#define ERR_NO_MAC			           	"No MAC command"
#define ERR_NO_DEVICE_STAT				"No device statistics"
#define ERR_INIT_DEVICE_STAT			"Device counter initialization error "
#define ERR_INIT_IDENTITY				"Device registry initialization error "
#define ERR_INIT_QUEUE					"Message queue initialization error "
#define ERR_HANGUP_DETECTED				"Hangup detected on controlling terminal or death of controlling process"
#define ERR_NO_FCNT_DOWN				"No FCnt down stored"
#define ERR_CONTROL_NOT_AUTHORIZED		"Identity is not authorized to control network service "
#define ERR_GATEWAY_NOT_FOUND			"Gateway not found "
#define ERR_CONTROL_DEVICE_NOT_FOUND	"Control device not found "
#define ERR_INVALID_CONTROL_PACKET		"Invalid control packet"
#define ERR_DUPLICATED_PACKET			"Packet is duplicated"
#define ERR_INIT_GW_STAT   			    "Gateway statistics log initialization error "
#define ERR_DEVICE_NAME_NOT_FOUND       "Device name not found"
#define ERR_DEVICE_EUI_NOT_FOUND        "Device EUI not found"
#define ERR_JOIN_EUI_NOT_MATCHED        "Join EUI not matched"
#define ERR_GATEWAY_NO_YET_PULL_DATA    "Gateway does not sent PULL_DATA request, can not send to gateway "
#define ERR_REGION_BAND_EMPTY           "RegionBands array element disappeared"
#define ERR_INIT_REGION_BANDS			"Regional settings does not loaded "
#define ERR_INIT_REGION_NO_DEFAULT		"No default regional settings loaded"
#define ERR_NO_REGION_BAND              "Regional settings not available"
#define ERR_REGION_BAND_NO_DEFAULT      "No default region assigned"
#define ERR_IS_JOIN					    "Join request"
#define ERR_BAD_JOIN_REQUEST			"Bad Join request"
#define ERR_NETID_OR_NETTYPE_MISSED     "NetId identifier or type missed"
#define ERR_NETTYPE_OUT_OF_RANGE        "NetType out of 0..7 range"
#define ERR_NETID_OUT_OF_RANGE          "NetId out of range"
#define ERR_TYPE_OUT_OF_RANGE           "Address type out of range"
#define ERR_NWK_OUT_OF_RANGE            "Address NwkId out of range"
#define ERR_ADDR_OUT_OF_RANGE           "Address out of range"
#define ERR_ADDR_SPACE_FULL             "Address space is full, no free address available"
#define ERR_INIT_LOGGER_HUFFMAN_PARSER  "Error initialize logger-huffman parser"

// Message en-us locale strings
#define MSG_PROG_NAME					"LoRaWAN network listener"
#define MSG_PROTO_DB_PROG_NAME			"proto-db helper utility"
#define MSG_LORA_PRINT_PROG_NAME		"lora-print helper utility"
#define MSG_INTERRUPTED 				"Interrupted "
#define MSG_GRACEFULLY_STOPPED			"Stopped gracefully"
#define MSG_PG_CONNECTED        		"Connected"
#define MSG_PG_CONNECTING       		"Connecting..."
#define MSG_DAEMON_STARTED      		"Start daemon "
#define MSG_DAEMON_STARTED_1    		". Check syslog."
#define MSG_WS_TIMEOUT					"Web service time out"
#define MSG_TIMEOUT						"Timeout"
#define MSG_GATEWAY						"Gateway "
#define MSG_DEVICE						"Device "
#define MSG_DEVICES						"Devices:"
#define MSG_REGIONAL_SETTINGS   		"Regional parameters "
#define MSG_DEVICE_EUI					"Device EUI "
#define MSG_JOIN_EUI					"Join EUI "
#define MSG_DEV_NONCE                   "Dev nonce "
#define MSG_DEVICE_ACTIVATION			"Device activation "
#define MSG_READ_BYTES					"Read bytes "
#define MSG_RECEIVED					"Message received "
#define MSG_MAC_COMMAND_RECEIVED		"MAC command(s) received from "
#define MSG_SENT_ACK_TO					"Sent ACK to "
#define MSG_SENT_REPLY_TO				"Sent reply to "
#define MSG_GATEWAY_STAT				"Gateway statistics "
#define MSG_RXPK						"rxpk "
#define MSG_MAC_COMMANDS                "MAC commands"
#define MSG_SEND_TO                     "Send to "
#define MSG_DATABASE_LIST				"Databases: "
#define MSG_CONN_ESTABLISHED			"established"
#define MSG_CONN_FAILED					"failed"
#define MSG_CONNECTION					"connection "
#define MSG_BEST_GATEWAY				"best gateway "
#define MSG_SEND_MAC_REPLY				"Send reply on MAC command(s)"
#define MSG_GATEWAY_SNR					" SNR "
#define MSG_SIG_FLUSH_FILES				"Signal to flush files received "
#define MSG_PUSH_PACKET_QUEUE			"Push packet queue "
#define MSG_RECEIVED_CONTROL_FRAME		"Control network service message received "
#define MSG_DB_INSERT                   "Insert payload to the database successful"
#define MSG_ENQUEUE_DB                  "Enqueue payload to the database"
#define MSG_ENQUEUE_JOIN_REQUEST        "Enqueue join request response "
#define MSG_SEND_JOIN_REQUEST_REPLY     "Sent join request response "
#define MSG_GATEWAY_LIST                "Gateways: "
#define MSG_INIT_UDP_LISTENER           "Initialize UDP listener.."
#define MSG_LISTEN_IP_ADDRESSES         "Listen IP addresses: "
#define MSG_TO_REQUEST                   "to request "
#define MSG_JOIN_REQUEST			    "Join request"
#define MSG_DEVICE_COUNT                "device(s)"
#define MSG_BYTES                       "bytes"
#define MSG_EXPECTED                    "Expected"
#define MSG_PAYLOAD                     "payload"
#define MSG_SIZE                        "size"
#define MSG_IS_TOO_SMALL                " is too small"
#define MSG_PREPARE                     "Prepare INSERT to database "

const char *logLevelString(int logLevel);
const char *logLevelColor(int logLevel);

const char *strerror_lorawan_ns(int errcode);
