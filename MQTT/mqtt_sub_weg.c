/*
 * This example shows how to subscribe messages from outside of the Mosquitto network loop.
 */


//https://github.com/eclipse/mosquitto/blob/master/client/sub_client.c


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <mosquitto.h>



// client id—Must be set to a valid Device ID that is already registered with the WEGnology Platform.
static char *deviceId = "6247691a41d3ec2f47b6cd43";

// username—Must be set to a WEGnology Access Key.
static char *username = "bb69ddcd-c579-4d47-944b-9170678964aa";

// password—Must be set to a WEGnology Access Secret, which is generated when creating an Access key.
static char *password = "c23f729adc7f37c17c2df3031ae0058019c24aaff03d03800dc21a90480478e6";





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
	return "Retorno desconhecido";
}




/** Chamada callback quando ocorre mensagem de log
	- the mosquitto instance making the callback.
	- the user data provided in mosquitto_new
	- the log message level from the values: MOSQ_LOG_INFO MOSQ_LOG_NOTICE MOSQ_LOG_WARNING MOSQ_LOG_ERR MOSQ_LOG_DEBUG
	- the message string.
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
	- the mosquitto instance making the callback
	- the user data provided in mosquitto_new
	- the return code of the connection response
*/
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	int rc;

	printf("   on_connect:   clientInstanceData: %d   return code: %d\n", * (int *) obj, reason_code);
	printf("   on_connect: %s\n", mosquitto_connack_string(reason_code));

	if(reason_code != 0){
		/* If the connection fails for any reason, we don't want to keep on
		 * retrying in this example, so disconnect. Without this, the client
		 * will attempt to reconnect. */

		printf("   on_connect: vai desconectar pois o connect falhou, reason_code %d\n", reason_code);
		mosquitto_disconnect(mosq);
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

	char topico[1000];
	strcpy( topico, "losant/");
	strcat( topico, deviceId);
	strcat( topico, "/command");

	rc = mosquitto_subscribe(mosq, NULL, topico, 1);
	printf("   mosquitto_subscribe retornou:  %s\n", msgRetorno(rc) );
	if(rc != MOSQ_ERR_SUCCESS){
		printf("   Error subscribing: %s\n", mosquitto_strerror(rc));
		/* We might as well disconnect if we were unable to subscribe */
		printf("   on_connect: vai desconectar pois o subscribe falhou, rc %d\n", rc);
		mosquitto_disconnect(mosq);
	}
}



/**	Callback called when the broker sends a SUBACK in response to a SUBSCRIBE.
	- the mosquitto instance making the callback
	- the user data provided in mosquitto_new
	- the message id of the subscribe message
	- the number of granted subscriptions (size of granted_qos)
	- an array of integers indicating the granted QoS for each of the subscriptions
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
	}
}


/** Funcao callback quando chega uma conexao do Broker
	- the mosquitto instance making the callback
	- the user data provided in mosquitto_new
	- the message data.  This variable and associated memory will be freed by the library after the callback completes.
*/
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	printf("   on_message:   Topico: %s   QoS: %d   Mensagem: %s\n", msg->topic, msg->qos, (char *) msg->payload);


(char *) msg->payload)

{"name":"comando_solar2022","payload":"{\n  \"name\": \"comandos\",\n  \"payload\": {\n    \"bombacoletor\": 0,\n    \"bombacirculacao\": 0,\n    \"aquecedor\": 0,\n    \"valvulaentrada\": 0,\n    \"valvulaesgoto\": 0,\n  }\n}\n\n","time":{"$date":"2022-04-05T00:41:32.114Z"}}




}



/** Main para testar uma subscricao
*/
int main(int argc, char *argv[])
{
	int clientInstanceData = 1111;
	
	struct mosquitto *mosq;		// Instancia de cliente Mosquitto
	int rc;						// Valor de retorno de algumas funcoes	


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
		exit(-1);
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
		printf("   Could not connect to Broker with return code %d\n", rc);
		/* Use to free memory associated with a mosquitto client instance.
			- a struct mosquitto pointer to free.
		*/
		mosquitto_destroy(mosq);
		return -1;
	}




	// RECEPÇÃO DE MENSAGENS: ALTERNATIVA COM LOOP INFINITO

	/* Run the network loop in a blocking call. The only thing we do in this
	 * example is to print incoming messages, so a blocking call here is fine.
	 *
	 * This call will continue forever, carrying automatic reconnections if
	 * necessary, until the user calls mosquitto_disconnect().
	 */
	//	rc = mosquitto_loop_forever(mosq, -1, 1);
	//	printf("mosquitto_loop_forever retornou:  %s\n", msgRetorno(rc) );	

	// Outra thread:
	/* Disconnect from the broker.
		- a valid mosquitto instance.
	*	
	mosquitto_disconnect(mosq);*/




	// RECEPÇÃO DE MENSAGENS: ALTERNATIVA COM THREAD EM PARALELO

	// Espera um tempo para a subscrição funcionar
	printf(">>>>> Esperando 10 segundos ...\n");
	sleep(10);



	/* This is part of the threaded client interface.  Call this once to start a new thread to process network traffic.
		- mosq	a valid mosquitto instance.
	*/
	rc = mosquitto_loop_start(mosq);
	printf("   mosquitto_loop_start retornou:  %s\n", msgRetorno(rc) );


	/* Thread main estah liberada para fazer outras coisas
	*/
	printf(">>>>> Tecle <Enter> para terminar...\n");
	getchar();
	
	
	/* This is part of the threaded client interface.  Call this once to stop the network thread previously created with mosquitto_loop_start.
		This call will block until the network thread finishes. You must have previously called mosquitto_disconnect or have set the force parameter to true.
		- a valid mosquitto instance.
		- set to true to force thread cancellation. If false, mosquitto_disconnect must have already been called.
	*/	
	rc = mosquitto_loop_stop(mosq, true);
	printf("   mosquitto_loop_stop retornou:  %s\n", msgRetorno(rc) );




	/* Call to free resources associated with the library.
	*/
	mosquitto_lib_cleanup();

	
	// Fim
	return 0;
}




