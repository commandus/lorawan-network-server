#ifndef ERRLIST_H_
#define ERRLIST_H_	1

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
#define LOG_EMBEDDED_GATEWAY    			7
#define LOG_TEMPERATURE_LOGGER_PASSPORT     8

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
#define ERR_CODE_WS_START_FAILED			-607
#define ERR_CODE_NO_DEFAULT_WS_DATABASE		-608
#define ERR_CODE_INIT_LOGGER_HUFFMAN_DB     -609
#define ERR_CODE_NO_PACKET_PARSER           -610
#define ERR_CODE_LOAD_WS_PASSWD_NOT_FOUND   -611

// embedded gateway config
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_BOARD_FAILED       -612
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_TIME_STAMP         -613
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_SX1261_RADIO       -614
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_TX_GAIN_LUT        -615
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_INVALID_RADIO      -616

#define ERR_CODE_LORA_GATEWAY_CONFIGURE_DEMODULATION       -617
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_MULTI_SF_CHANNEL   -618
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_STD_CHANNEL        -619
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_FSK_CHANNEL        -620
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_DEBUG              -621

#define ERR_CODE_LORA_GATEWAY_CONFIGURE_GPS_FAILED         -622
#define ERR_CODE_LORA_GATEWAY_START_FAILED                 -623
#define ERR_CODE_LORA_GATEWAY_GET_EUI                      -624
#define ERR_CODE_LORA_GATEWAY_GPS_GET_TIME                 -625
#define ERR_CODE_LORA_GATEWAY_GPS_SYNC_TIME                -626
#define ERR_CODE_LORA_GATEWAY_GPS_DISABLED                 -627

#define ERR_CODE_LORA_GATEWAY_GPS_GET_COORDS               -628
#define ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_START_FAILED   -629
#define ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_TIMEOUT        -630
#define ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_FAILED         -631
#define ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_ABORTED        -632
#define ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_UNEXPECTED_STATUS -633
#define ERR_CODE_LORA_GATEWAY_GET_TX_STATUS                -634
#define ERR_CODE_LORA_GATEWAY_SKIP_SPECTRAL_SCAN            -635
#define ERR_CODE_LORA_GATEWAY_STATUS_FAILED                 -636
#define ERR_CODE_LORA_GATEWAY_EMIT_ALLREADY                 -637
#define ERR_CODE_LORA_GATEWAY_SCHEDULED_ALLREADY            -638
#define ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_ABORT_FAILED    -639
#define ERR_CODE_LORA_GATEWAY_SEND_FAILED                   -640
#define ERR_CODE_LORA_GATEWAY_SENT                          -641
#define ERR_CODE_LORA_GATEWAY_JIT_DEQUEUE_FAILED            -642
#define ERR_CODE_LORA_GATEWAY_JIT_PEEK_FAILED               -643
#define ERR_CODE_LORA_GATEWAY_JIT_ENQUEUE_FAILED            -644
#define ERR_CODE_LORA_GATEWAY_FETCH                         -645
#define ERR_CODE_LORA_GATEWAY_UNKNOWN_STATUS                -646
#define ERR_CODE_LORA_GATEWAY_UNKNOWN_DATARATE              -647
#define ERR_CODE_LORA_GATEWAY_UNKNOWN_BANDWIDTH             -648
#define ERR_CODE_LORA_GATEWAY_UNKNOWN_CODERATE              -649
#define ERR_CODE_LORA_GATEWAY_UNKNOWN_MODULATION            -650
#define ERR_CODE_LORA_GATEWAY_RECEIVED                      -651
#define ERR_CODE_LORA_GATEWAY_AUTOQUIT_THRESHOLD            -652
#define ERR_CODE_LORA_GATEWAY_BEACON_FAILED                 -653
#define ERR_CODE_LORA_GATEWAY_UNKNOWN_TX_MODE               -654
#define ERR_CODE_LORA_GATEWAY_SEND_AT_GPS_TIME              -655
#define ERR_CODE_LORA_GATEWAY_SEND_AT_GPS_TIME_DISABLED     -656
#define ERR_CODE_LORA_GATEWAY_SEND_AT_GPS_TIME_INVALID      -657
#define ERR_CODE_LORA_GATEWAY_TX_CHAIN_DISABLED             -658
#define ERR_CODE_LORA_GATEWAY_TX_UNSUPPORTED_FREQUENCY      -659
#define ERR_CODE_LORA_GATEWAY_TX_UNSUPPORTED_POWER          -660
#define ERR_CODE_LORA_GATEWAY_USB_NOT_FOUND                 -661
#define ERR_CODE_LORA_GATEWAY_SHUTDOWN_TIMEOUT              -662
#define ERR_CODE_LORA_GATEWAY_STOP_FAILED                   -663

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
#define ERR_WS_START_FAILED				"Start web service failed "
#define ERR_NO_DEFAULT_WS_DATABASE		"No default web service database name "
#define ERR_INIT_LOGGER_HUFFMAN_DB      "No huffman-logger database assigned"
#define ERR_NO_PACKET_PARSER            "No packet parser"
#define ERR_LOAD_WS_PASSWD_NOT_FOUND    "User password file not found"

#define ERR_LORA_GATEWAY_CONFIGURE_BOARD_FAILED       "Failed to configure board"
#define ERR_LORA_GATEWAY_CONFIGURE_TIME_STAMP         "Failed to configure fine timestamp"
#define ERR_LORA_GATEWAY_CONFIGURE_SX1261_RADIO       "Failed to configure the SX1261 radio"
#define ERR_LORA_GATEWAY_CONFIGURE_TX_GAIN_LUT        "Failed to configure concentrator TX Gain LUT"
#define ERR_LORA_GATEWAY_CONFIGURE_INVALID_RADIO      "Invalid configuration for radio"

#define ERR_LORA_GATEWAY_CONFIGURE_DEMODULATION       "Invalid configuration for demodulation parameters"
#define ERR_LORA_GATEWAY_CONFIGURE_MULTI_SF_CHANNEL   "Invalid configuration for Lora multi-SF channel"
#define ERR_LORA_GATEWAY_CONFIGURE_STD_CHANNEL        "Invalid configuration for Lora standard channel"
#define ERR_LORA_GATEWAY_CONFIGURE_FSK_CHANNEL        "Invalid configuration for Lora FSK channel"
#define ERR_LORA_GATEWAY_CONFIGURE_DEBUG              "Failed to configure debug"

#define ERR_LORA_GATEWAY_CONFIGURE_GPS_FAILED          "Failed to configure GPS"
#define ERR_LORA_GATEWAY_START_FAILED                  "Failed to start the concentrator"
#define ERR_LORA_GATEWAY_GET_EUI                       "Failed to get concentrator EUI"
#define ERR_LORA_GATEWAY_GPS_GET_TIME                  "Could not get GPS time"
#define ERR_LORA_GATEWAY_GPS_SYNC_TIME                 "GPS out of sync"

#define ERR_LORA_GATEWAY_GPS_DISABLED                  "GPS disabled"
#define ERR_LORA_GATEWAY_GPS_GET_COORDS                "Could not get GPS coordinates"
#define ERR_LORA_GATEWAY_SPECTRAL_SCAN_START_FAILED    "Spectral scan start failed"
#define ERR_LORA_GATEWAY_SPECTRAL_SCAN_TIMEOUT         "Timeout on Spectral Scan"
#define ERR_LORA_GATEWAY_SPECTRAL_SCAN_FAILED          "Spectral scan status failed"

#define ERR_LORA_GATEWAY_SPECTRAL_SCAN_ABORTED         "Spectral scan has been aborted"
#define ERR_LORA_GATEWAY_SPECTRAL_SCAN_UNEXPECTED_STATUS "Unexpected spectral scan status"
#define ERR_LORA_GATEWAY_GET_TX_STATUS                  "Failed to get TX status on spectral scan"
#define ERR_LORA_GATEWAY_SKIP_SPECTRAL_SCAN             "Skip spectral scan"
#define ERR_LORA_GATEWAY_STATUS_FAILED                  "Getting gateway status failed"
#define ERR_LORA_GATEWAY_EMIT_ALLREADY                  "Concentrator is currently emitting"
#define ERR_LORA_GATEWAY_SCHEDULED_ALLREADY             "Downlink was already scheduled on, overwriting it"

#define ERR_LORA_GATEWAY_SPECTRAL_SCAN_ABORT_FAILED     "Spectral scan abort failed"
#define ERR_LORA_GATEWAY_SEND_FAILED                    "Gateway send failed"
#define ERR_LORA_GATEWAY_SENT                           "Gateway sent successfully"
#define ERR_LORA_GATEWAY_JIT_DEQUEUE_FAILED             "JIT dequeue failed"
#define ERR_LORA_GATEWAY_JIT_PEEK_FAILED                "JIT peek failed"
#define ERR_LORA_GATEWAY_JIT_ENQUEUE_FAILED             "JIT enqueueTxPacket failed"

#define ERR_LORA_GATEWAY_FETCH                          "Failed Lora packet fetch, exiting"
#define ERR_LORA_GATEWAY_UNKNOWN_STATUS                 "Received Lora packet with unknown status"
#define ERR_LORA_GATEWAY_UNKNOWN_DATARATE               "Received Lora packet with unknown data rate"
#define ERR_LORA_GATEWAY_UNKNOWN_BANDWIDTH              "Received Lora packet with unknown bandwidth"
#define ERR_LORA_GATEWAY_UNKNOWN_CODERATE               "Received Lora packet with unknown code rate"

#define ERR_LORA_GATEWAY_UNKNOWN_MODULATION             "Received Lora packet with unknown modulation"
#define ERR_LORA_GATEWAY_RECEIVED                       "Received Lora packet "
#define ERR_LORA_GATEWAY_AUTOQUIT_THRESHOLD             "Last PULL_DATA were not ACKed, exiting application"
#define ERR_LORA_GATEWAY_BEACON_FAILED                  "Beacon queuing failed"
#define ERR_LORA_GATEWAY_DUPLICATE_ACK                  "Duplicate ACK received"

#define ERR_LORA_GATEWAY_UNKNOWN_TX_MODE                "Unknown Tx mode"
#define ERR_LORA_GATEWAY_SEND_AT_GPS_TIME               "No valid GPS time reference yet, impossible to send packet on specific GPS time, TX aborted"
#define ERR_LORA_GATEWAY_SEND_AT_GPS_TIME_DISABLED      "GPS disabled, impossible to send packet on specific GPS time, TX aborted"
#define ERR_LORA_GATEWAY_SEND_AT_GPS_TIME_INVALID       "Сould not convert GPS time to timestamp, TX aborted"
#define ERR_LORA_GATEWAY_TX_CHAIN_DISABLED              "TX is not enabled on RF chain, TX aborted"

#define ERR_LORA_GATEWAY_TX_UNSUPPORTED_FREQUENCY       "Unsupported frequency, TX aborted"
#define ERR_LORA_GATEWAY_TX_UNSUPPORTED_POWER           "RF power is not supported, closest lower power used"

#define ERR_LORA_GATEWAY_USB_NOT_FOUND                  "Gateway USB path not found"
#define ERR_LORA_GATEWAY_SHUTDOWN_TIMEOUT               "Gateway shutdown timeout"
#define ERR_LORA_GATEWAY_STOP_FAILED                    "Gateway stop failed"

// Message en-us locale strings
#define MSG_PROG_NAME					"LoRaWAN network listener"
#define MSG_PROTO_DB_PROG_NAME			"proto-dbSqlite3 helper utility"
#define MSG_LORA_PRINT_PROG_NAME		"lora-print helper utility"
#define MSG_INTERRUPTED 				"Interrupted "
#define MSG_GRACEFULLY_STOPPED			"Stopped gracefully"
#define MSG_PG_CONNECTED        		"Connected"
#define MSG_PG_CONNECTING       		"Connecting..."
#define MSG_DAEMON_STARTED      		"Start daemon "
#define MSG_DAEMON_STOPPED      		"Daemon has been stopped"

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
#define MSG_DEFAULT_DATABASE			"(default)"
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
#define MSG_WS_START					"Start web service "
#define MSG_WS_START_SUCCESS			"web service started successfully "
#define MSG_IDENTITY_START				"Start identity service.."
#define MSG_IDENTITY_INIT               "Initialize identity service NetId: "
#define MSG_GW_STAT_FILE_START          "Start gateway statistics service file.."
#define MSG_GW_STAT_POST_START          "Start gateway statistics service post.."
#define MSG_GW_STAT_INIT                "Initialize gateway statistics service.."
#define MSG_DEV_STAT_START              "Start device statistics service.."
#define MSG_DEV_STAT_INIT               "Initialize device statistics service.."
#define MSG_REGIONAL_SET_START          "Start regional settings .."
#define MSG_REGIONAL_SET_INIT           "Initialize regional settings "
#define MSG_DEV_HISTORY_START           "Start device history service.."
#define MSG_DEV_HISTORY_INIT            "Initialize device history service.."
#define MSG_RECEIVER_QUEUE_START        "Start received message queue service .."
#define MSG_RECEIVER_QUEUE_INIT         "Initialize receiver message queue service .."
#define MSG_START_PACKET_PROCESSOR      "Start LoRaWAN packet processor.."
#define MSG_LISTENER_DAEMON_RUN         "Listener daemon run .."
#define MSG_LISTENER_RUN                "Listener run .."
#define MSG_INIT_LOGGER_HUFFMAN         "Initialize payload parser logger-huffman, database "
#define MSG_START_OUTPUT_DB_SVC         "Start output database service .."
#define MSG_CHECKING_DB_AVAILABILITY    "Checking database availability.."
#define MSG_LISTEN_SOCKETS              "Listen sockets "
#define MSG_LISTEN_EMBEDDED_USB         "Listen USB gateway "
#define MSG_LISTEN_EMBEDDED_PCI         "Listen PCI gateway "
#define MSG_LISTEN_SOCKET_COUNT         " socket(s)"
#define MSG_LISTEN_LARGEST_SOCKET       "Largest socket "
#define MSG_SPECTRAL_SCAN_FREQUENCY     "Spectral scan, frequency "
#define MSG_LORA_GATEWAY_SEND_AT_GPS_TIME   "A packet will be sent on timestamp (calculated from GPS time)"
#define MSG_TO_BE_SEND_TO               " to be send to "
#define MSG_LOAD_PLUGINS                "Load plugins "
#define MSG_PLUGIN_PATH_NOT_FOUND       "No plugin directory specified or path not found "
#define MSG_NO_PLUGINS_LOADED           "No any plugin found"
#define MSG_LOADED_PLUGINS_COUNT        "Plugins loaded: "

const char *logLevelString(int logLevel);
const char *logLevelColor(int logLevel);

const char *strerror_lorawan_ns(int errcode);

#endif
