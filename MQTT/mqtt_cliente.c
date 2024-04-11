/**	Este exemplo mostra como publicar e subscrever mensagens 
	através do broker MQTT da plataforma WEGnology.
	
	Utiliza a biblioteca cliente do Mosquitto.
	
	Baseado em
		https://mosquitto.org/api/files/mosquitto-h.html
		https://mosquitto.org/man/libmosquitto-3.html
	
		https://github.com/eclipse/mosquitto/blob/master/examples/publish/basic-1.c
		https://github.com/eclipse/mosquitto/blob/master/examples/subscribe/basic-1.c

		https://docs.app.wnology.io/mqtt/overview/
		https://docs.app.wnology.io/devices/state/
		https://docs.app.wnology.io/devices/commands/
	
		https://docs.app.wnology.io/
	

	gcc -o mqtt_cliente mqtt_cliente.c  -lmosquitto

 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <mosquitto.h>


// client id—Must be set to a valid Device ID that is already registered with the WEGnology Platform
static char *deviceId = "6247691a41d3ec2f47b6cd43";

// username—Must be set to a WEGnology Access Key
static char *username = "bb69ddcd-c579-4d47-944b-9170678964aa";

// password—Must be set to a WEGnology Access Secret, which is generated when creating an Access key
static char *password = "c23f729adc7f37c17c2df3031ae0058019c24aaff03d03800dc21a90480478e6";


// Flag global para indicar se estah ou nao conectado ao Broker
// Nao foi usada neste programa, util para multithread
static int conectado = 0;


// Topico para receber comandos
static char topico_command[1000];

// Topico para enviar estados
static char topico_state[1000];


// Variáveis de estado da planta (faz de conta)
static float nivelBoiler = 0.1;
static float tempBoiler = 34.2;
static float tempColetor = 56.3;
static float tempCanos = 78.4;

static int bombaColetor = 0;
static int bombaCirculacao = 0;
static int aquecedor = 0;
static int valvulaEntrada = 1;
static int valvulaEsgoto = 1;





/** Coloca na tela o significado de um retorno da libmosquitto
*/
char *msgRetorno( int rc)
{
	switch(rc) {
		case MOSQ_ERR_SUCCESS:			return "success";
		case MOSQ_ERR_INVAL:			return "invalid input parameters";
		case MOSQ_ERR_NOMEM:			return "out of memory";
		case MOSQ_ERR_NO_CONN:			return "client isn’t connected to a broker";
		case MOSQ_ERR_MALFORMED_UTF8:	return "the topic is not valid UTF-8";
		case MOSQ_ERR_OVERSIZE_PACKET:	return "resulting packet would be larger than supported by the broker";
		case MOSQ_ERR_CONN_LOST:		return "connection to the broker was lost";
		case MOSQ_ERR_PROTOCOL:			return "protocol error communicating with the broker";
		case MOSQ_ERR_NOT_SUPPORTED:	return "thread support is not available";
	}
	return "retorno desconhecido";
}



/** Chamada callback quando ocorre mensagem de log da biblioteca
	- mosq - the mosquitto instance making the callback.
	- obj - the user data provided in mosquitto_new
	- level - the log message level from the values: MOSQ_LOG_INFO MOSQ_LOG_NOTICE MOSQ_LOG_WARNING MOSQ_LOG_ERR MOSQ_LOG_DEBUG
	- str - the message string.
*/
void on_log_msg(struct mosquitto *mosq, void *obj, int level, const char *str)
{
	// Printing all log messages regardless of level
	switch(level){
		case MOSQ_LOG_INFO:
		case MOSQ_LOG_NOTICE:
		case MOSQ_LOG_WARNING:
		case MOSQ_LOG_ERR:
		case MOSQ_LOG_DEBUG: printf("##### clientInstanceData: %d   Nivel: %d   MSG: %s\n", * (int *) obj, level, str);
	}	
}


/** Chamada callback quando ocorre uma conexao com o Broker:
	the client receives a CONNACK message from the broker
	- mosq - the mosquitto instance making the callback
	- obj - the user data provided in mosquitto_new
	- reason_code - the return code of the connection response
*/
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	int rc;
	
	// mosquitto_connack_string() produces an appropriate string for MQTT v3.x clients
	// the equivalent for MQTT v5.0 clients is mosquitto_reason_string()

	printf("   on_connect:   clientInstanceData: %d   return code: %d\n", * (int *) obj, reason_code);
	printf("   on_connect: %s\n", mosquitto_connack_string(reason_code));

	if(reason_code != 0){
		/* If the connection fails for any reason, we don't want to keep on
		 * retrying in this example, so disconnect. Without this, the client
		 * will attempt to reconnect. */
		printf("   on_connect: vai desconectar pois o connect falhou, reason_code %d\n", reason_code);
		mosquitto_disconnect(mosq);
		conectado = 0;
		mosquitto_lib_cleanup();
		exit(1);
	}
	else {
		// Liga flag global para indicar que estah conectado ao Broker
		conectado = 1;
	}	

	/* Making subscriptions in the on_connect() callback means that if the
	 * connection drops and is automatically resumed by the client, then the
	 * subscriptions will be recreated when the client reconnects. */

	/* Subscribe to a topic
		- a valid mosquitto instance.
		- a pointer to an int.  If not NULL, the function will set this to the message id of this particular message
		- the subscription pattern
		- the requested Quality of Service for this subscription
	*/

	rc = mosquitto_subscribe(mosq, NULL, topico_command, 1);
	printf("   mosquitto_subscribe retornou:  %s\n", msgRetorno(rc) );
	if(rc != MOSQ_ERR_SUCCESS){
		printf("   Error subscribing: %s\n", mosquitto_strerror(rc));
		/* We might as well disconnect if we were unable to subscribe */
		printf("   on_connect: vai desconectar pois o subscribe falhou, rc %d\n", rc);
		mosquitto_disconnect(mosq);
		mosquitto_lib_cleanup();
		exit(1);
	}
}


/**	Callback called when the broker sends a SUBACK in response to a SUBSCRIBE.
	- mosq - the mosquitto instance making the callback
	- obj - the user data provided in mosquitto_new
	- mid - the message id of the subscribe message
	- qos_count - the number of granted subscriptions (size of granted_qos)
	- granted_qos - an array of integers indicating the granted QoS for each of the subscriptions
*/
void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	int i;
	bool have_subscription = false;

	printf("   on_subscribe:   qos_count: %d,   granted_qos[0]: %d\n", qos_count, granted_qos[0]);


	/* In this example we only subscribe to a single topic at once, but a
	 * SUBSCRIBE can contain many topics at once, so this is one way to check
	 * them all. */
	for(i=0; i<qos_count; i++){
		printf("   on_subscribe: %d  granted qos = %d\n", i, granted_qos[i]);
		if(granted_qos[i] <= 2){
			have_subscription = true;
		}
	}
	if(have_subscription == false){
		/* The broker rejected all of our subscriptions, we know we only sent
		 * the one SUBSCRIBE, so there is no point remaining connected. */
		printf("   on_subscribe:   Error: All subscriptions rejected.\n");
		mosquitto_disconnect(mosq);
		mosquitto_lib_cleanup();
		exit(1);
	}
}



/** Funcao auxiliar para receber os valores dos comandos
*/
void novoValor( char recebido, int *estadoAtual)
{
	if( recebido == 't' )
		*estadoAtual = 1;
	else if( recebido == 'f' )
		*estadoAtual = 0;
	else
		printf("   on_message:   Valor desconhecido recebido no comando\n");
}


/** Funcao callback quando chega uma conexao do Broker
	- mosq - the mosquitto instance making the callback
	- obj - the user data provided in mosquitto_new
	- msg - the message data. This variable and associated memory will be freed by the library after the callback completes.
*/
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	printf("   on_message:   Topico: %s   QoS: %d   Mensagem: %s\n", msg->topic, msg->qos, (char *) msg->payload);

	if( strcmp(msg->topic,topico_command) != 0 ) {
		printf("   on_message: Topico não confere\n");
	}	

	// Vai direto no lugar da mensagem onde esta o valor 'true' ou 'false' para cada atuador
	char *col = strstr( msg->payload, "combombacoletor") + strlen("combombacoletor") + 4;
	char *cir = strstr( msg->payload, "combombacirculacao") + strlen("combombacirculacao") + 4;
	char *aqc = strstr( msg->payload, "comaquecedor") + strlen("comaquecedor") + 4;
	char *ent = strstr( msg->payload, "comvalvulaentrada") + strlen("comvalvulaentrada") + 4;
	char *esg = strstr( msg->payload, "comvalvulaesgoto") + strlen("comvalvulaesgoto") + 4;

	novoValor( *col, &bombaColetor);
	novoValor( *cir, &bombaCirculacao);
	novoValor( *aqc, &aquecedor);
	novoValor( *ent, &valvulaEntrada);
	novoValor( *esg, &valvulaEsgoto);

	printf(">>>>>> %s\n\n", esg);

}


/** Callback called when the client knows to the best of its abilities that a
	PUBLISH has been successfully sent. For QoS 0 this means the message has
	been completely written to the operating system. For QoS 1 this means we
	have received a PUBACK from the broker. For QoS 2 this means we have
	received a PUBCOMP from the broker.
 	- the mosquitto instance making the callback
	- the user data provided in mosquitto_new
	- the message id of the sent message
*/
void on_publish(struct mosquitto *mosq, void *obj, int mid)
{
	printf("   on_publish: Message with mid %d has been published.\n", mid);
}



/** Simula a leitura de sensores e o envio dos seus estados como mensagem MQTT
*/
void publish_sensor_data(struct mosquitto *mosq)
{
	int rc;


	// Altera alguns valores a cada envio
	nivelBoiler += 0.1;
	tempBoiler += 1.0;
	tempColetor += 1.0; 
	tempCanos += 1.0; 


	// Converte para string os valores a serem enviados
	char strNBoiler[100];
	char strTBoiler[100];
	char strTColetor[100];
	char strTCanos[100];

	sprintf( strNBoiler, "%.1f", nivelBoiler);
	sprintf( strTBoiler, "%.1f", tempBoiler);
	sprintf( strTColetor, "%.1f", tempColetor); 
	sprintf( strTCanos, "%.1f", tempCanos); 

	char strBombaColetor[100];
	char strBombaCirculacao[100];
	char strAquecedor[100];
	char strValvulaEntrada[100];
	char strValvulaEsgoto[100];

	sprintf( strBombaColetor, "%d", bombaColetor);
	sprintf( strBombaCirculacao, "%d", bombaCirculacao);
	sprintf( strAquecedor, "%d", aquecedor);
	sprintf( strValvulaEntrada, "%d", valvulaEntrada);
	sprintf( strValvulaEsgoto, "%d", valvulaEsgoto);


	// Monta o payload da mensagem MQTT publicada

	char payload[1000];
	
	strcpy( payload, "{\n"
					 "  \"data\": {\n"

					 "    \"nivelboiler\": " );
	strcat( payload, strNBoiler);				 
					 
	strcat( payload, ",\n    \"tempboiler\": " );
	strcat( payload, strTBoiler);				 
					 
	strcat( payload, ",\n    \"tempcoletor\": " );
	strcat( payload, strTColetor);				 
	
	strcat( payload, ",\n    \"tempcanos\": " );
	strcat( payload, strTCanos);
	

	strcat( payload, ",\n    \"bombacoletor\": " );
	strcat( payload, strBombaColetor);

	strcat( payload, ",\n    \"bombacirculacao\": " );
	strcat( payload, strBombaCirculacao);

	strcat( payload, ",\n    \"aquecedor\": " );
	strcat( payload, strAquecedor);

	strcat( payload, ",\n    \"valvulaentrada\": " );
	strcat( payload, strValvulaEntrada);

	strcat( payload, ",\n    \"valvulaesgoto\": " );
	strcat( payload, strValvulaEsgoto);

	strcat( payload, "\n  }\n}\n" );


	/* Publish the message
	 * mosq - our client instance
	 * *mid = NULL - we don't want to know what the message id for this message is
	 * topic = "example/temperature" - the topic on which this message will be published
	 * payloadlen = strlen(payload) - the length of our payload in bytes
	 * payload - the actual payload
	 * qos = 2 - publish with QoS 2 for this example
	 * retain = false - do not use the retained message feature for this message
	 */

	rc = mosquitto_publish(mosq, NULL, topico_state, strlen(payload), payload, 1, false);
	if(rc != MOSQ_ERR_SUCCESS){
		printf("   publish_sensor_data: Error publishing: %s\n", mosquitto_strerror(rc));
	}
}


/** Main para testar comunicacao via MQTT com o Broker da Wegnology
*/
int main(int argc, char *argv[])
{
	int clientInstanceData = 1111;	// Usado no caso de comunicacao com varios Brokers
	struct mosquitto *mosq;			// Instancia de cliente Mosquitto
	int rc;							// Valor de retorno de algumas funcoes	


	// Cria o topico unico que sera publicado, varios sensores na mensagem
	strcpy( topico_state, "losant/");
	strcat( topico_state, deviceId);
	strcat( topico_state, "/state");

	// Cria o topico unico que sera assinado, varios comandos na mensagem
	strcpy( topico_command, "losant/");
	strcat( topico_command, deviceId);
	strcat( topico_command, "/command");


	/*	Must be called before any other mosquitto functions. This function is not thread safe.
	*/
	mosquitto_lib_init();


	/* Create a new mosquitto client instance
		- string to use as the client id, NULL asks the broker to generate a client id
		- set to true to instruct the broker to clean all messages and subscriptions on disconnect, false to instruct it to keep them
		- a user pointer that will be passed as an argument to any callbacks that are specified
	*/
	mosq = mosquitto_new( deviceId, true, &clientInstanceData);
	if(mosq == NULL) {
		printf("   mosquitto_new retornou erro %d\n", errno);
		mosquitto_lib_cleanup();
		exit(1);
	} else {
		printf("   mosquitto_new retornou sucesso\n");
	}


	/* Set the logging callback.  This should be used if you want event logging information from the client library.
		- a valid mosquitto instance.
		- a callback function in the following form: void callback(struct mosquitto *mosq, void *obj, int level, const char *str)
	*/
	mosquitto_log_callback_set(mosq, on_log_msg);


	/* Set the connect callback. This is called when the broker sends a CONNACK message in response to a connection.
		- a valid mosquitto instance
		- a callback function in the following form: void callback(struct mosquitto *mosq, void *obj, int rc)
	*/
	mosquitto_connect_callback_set(mosq, on_connect);


	/* Set the publish callback.  This is called when a message initiated with mosquitto_publish has been sent to the broker successfully.
		- a valid mosquitto instance.
		- a callback function in the following form: void callback(struct mosquitto *mosq, void *obj, int mid)
	*/		
	mosquitto_publish_callback_set(mosq, on_publish);


	/* Set the subscribe callback.  This is called when the broker responds to a subscription request.
		- a valid mosquitto instance.
		- a callback function in the following form: void callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
	*/
	mosquitto_subscribe_callback_set(mosq, on_subscribe);


	/* Set the message callback. This is called when a message is received from the broker.
		- a valid mosquitto instance
		- a callback function in the following form: void callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
	*/
	mosquitto_message_callback_set(mosq, on_message);


	/* Configure username and password for a mosquitto instance.  By default, no username or password will be sent.
		This is must be called before calling mosquitto_connect.
		- a valid mosquitto instance.
		- the username to send as a string, or NULL to disable authentication.
		- the password to send as a string.
	*/
	rc = mosquitto_username_pw_set(mosq, username, password);
	printf("   mosquitto_username_pw_set retornou:  %s\n", msgRetorno(rc) );


	/* Connect to an MQTT broker.
		- a valid mosquitto instance
		- the hostname or ip address of the broker to connect to
		- the network port to connect to.  Usually 1883.
		- the number of seconds after which the broker should send a PING message to the client if no other messages have been exchanged in that time.
	*/
	rc = mosquitto_connect(mosq, "broker.app.wnology.io", 1883, 120);
	printf("   mosquitto_connect retornou:  %s\n", msgRetorno(rc) );
	if(rc != MOSQ_ERR_SUCCESS) {
		printf("   main: Could not connect to Broker with return code %d\n", rc);
		/* Use to free memory associated with a mosquitto client instance.
			- a struct mosquitto pointer to free.
		*/
		mosquitto_destroy(mosq);
		exit(1);
	}


	/*	Espera um tempo para a conexao funcionar, por via das duvidas
	*/
	printf(">>>>> Esperando 5 segundos ...\n");
	sleep(5);



	/* ALTERNATIVA:
		Run the network loop in a blocking call. The only thing we do in this
		example is to print incoming messages, so a blocking call here is fine.
		This call will continue forever, carrying automatic reconnections if
		necessary, until the user calls mosquitto_disconnect().

	rc = mosquitto_loop_forever(mosq, -1, 1);
	printf("mosquitto_loop_forever retornou:  %s\n", msgRetorno(rc) );	

		Outra thread:
		Disconnect from the broker.
		- a valid mosquitto instance.

	mosquitto_disconnect(mosq);
	*/



	/* Run the network loop in a background thread, this call returns quickly.
	   This is part of the threaded client interface.  Call this once to start a new thread to process network traffic.
		- mosq	a valid mosquitto instance.
	*/
	rc = mosquitto_loop_start(mosq);
	printf("   mosquitto_loop_start retornou:  %s\n", msgRetorno(rc) );
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		printf("   main: Error: %s\n", mosquitto_strerror(rc));
		exit(1);
	}


	/* At this point the client is connected to the network socket, but may not
	 * have completed CONNECT/CONNACK.
	 * It is fairly safe to start queuing messages at this point, but if you
	 * want to be really sure you should wait until after a successful call to
	 * the connect callback.
	 * In this case we know it is 1 second before we start publishing.
	 */
	for( int i=0; i<50; ++i) {
		sleep(2);
		publish_sensor_data(mosq);
	}


	/* This is part of the threaded client interface.  Call this once to stop the network thread previously created with mosquitto_loop_start.
		This call will block until the network thread finishes.
		You must have previously called mosquitto_disconnect or have set the force parameter to true.
		- a valid mosquitto instance.
		- set to true to force thread cancellation. If false, mosquitto_disconnect must have already been called.
	*/
	conectado = 0;
	rc = mosquitto_loop_stop(mosq, true);
	printf("   mosquitto_loop_stop retornou:  %s\n", msgRetorno(rc) );


	/* Call to free resources associated with the library.
	*/
	mosquitto_lib_cleanup();
	
	
	// Fim
	return 0;
}







