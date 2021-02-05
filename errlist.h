
/* syslog
#define	LOG_ERROR							1
#define	LOG_INFO							2
#define	LOG_DEBUG							3
*/

#define LOG_UDP_EMITTER						1
#define LOG_UDP_LISTENER					2

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

#define ERR_MESSAGE						"Error "
#define ERR_DEBUG						"Info "
#define ERR_WARNING						"Warning "
#define ERR_TIMEOUT						"Timeout"

#define ERR_COMMAND_LINE        		"Wrong parameter(s)"
#define ERR_OPEN_SOCKET         		"open socket error "
#define ERR_CLOSE_SOCKET        		"close socket error "
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
#define ERR_INVALID_ADDRESS				"Invalid Internet address"
#define ERR_INVALID_FAMILY				"Invalid Internet address family"
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

#define MSG_PROG_NAME					"LoRaWAN network listener"
#define MSG_INTERRUPTED 				MSG_PROG_NAME " interrupted "
#define MSG_GRACEFULLY_STOPPED			MSG_PROG_NAME " closed gracefully"
#define MSG_PG_CONNECTED        		"Connected"
#define MSG_PG_CONNECTING       		"Connecting..."
#define MSG_DAEMON_STARTED      		"Start daemon "
#define MSG_DAEMON_STARTED_1    		". Check syslog."
#define MSG_WS_TIMEOUT					"Web service time out"
#define MSG_STOPPED						"ChirpStack logger stopped gracefully"
#define MSG_CHIRPSTACK_SERVER_VERSION	"ChirpStack network server version "
#define MSG_GATEWAY						"Gateway "
#define MSG_DEVICE						"Device "
#define MSG_DEVICE_EUI					"Device EUI "
#define MSG_DEVICE_ACTIVATION			"Device activation "
#define MSG_READ_BYTES					"Read bytes "
#define ERROR							"Error "

const char *strerror_client(int errcode);
