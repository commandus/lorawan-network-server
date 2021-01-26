
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

#define ERR_COMMAND_LINE        		"Wrong parameter(s)"
#define ERR_OPEN_SOCKET         		"open socket error "
#define ERR_CLOSE_SOCKET        		"close socket error "
#define ERR_BAD_STATUS          		"Bad status"
#define ERR_INVALID_PAR_LOG_FILE		"Invalid log file "
#define ERR_GET_ADDRESS					"get address info error "
#define ERR_BIND						"bind error "
#define ERR_INVALID_SERVICE				"Invalid service address "
#define ERR_INVALID_GATEWAY_ID			"Invalid gateway identifier"
#define ERR_INVALID_DEVICE_EUI			"Invalid device EUI"
#define ERR_INVALID_BUFFER_SIZE			"Invalid buffer size "
#define ERR_GRPC_NETWORK_SERVER_FAIL	"gRPC network server error "
#define ERR_INVALID_RFM_HEADER			"Invalid RFM header"

#define MSG_INTERRUPTED 				"Interrupted "
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
#define ERROR							"Error "

const char *strerror_client(int errcode);
