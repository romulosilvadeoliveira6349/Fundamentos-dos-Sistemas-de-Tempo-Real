/*
 * This example shows how to publish messages from outside of the Mosquitto network loop.
 */

#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


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



/** Callback called when the client receives a CONNACK message from the broker.
*/
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	/* Print out the connection result. mosquitto_connack_string() produces an
	 * appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
	 * clients is mosquitto_reason_string().
	 */
	printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
	if(reason_code != 0){
		/* If the connection fails for any reason, we don't want to keep on
		 * retrying in this example, so disconnect. Without this, the client
		 * will attempt to reconnect. */
		mosquitto_disconnect(mosq);
	}

	/* You may wish to set a flag here to indicate to your application that the
	 * client is now connected. */
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
	printf("Message with mid %d has been published.\n", mid);
}


int get_temperature(void)
{
	sleep(1); /* Prevent a storm of messages - this pretend sensor works at 1Hz */
	return random()%100;
}

/* This function pretends to read some data from a sensor and publish it.*/
void publish_sensor_data(struct mosquitto *mosq)
{
	char payload[20];
	int temp;
	int rc;

	/* Get our pretend data */
	temp = get_temperature();
	/* Print it to a string for easy human reading - payload format is highly
	 * application dependent. */
	snprintf(payload, sizeof(payload), "%d", temp);

	/* Publish the message
	 * mosq - our client instance
	 * *mid = NULL - we don't want to know what the message id for this message is
	 * topic = "example/temperature" - the topic on which this message will be published
	 * payloadlen = strlen(payload) - the length of our payload in bytes
	 * payload - the actual payload
	 * qos = 2 - publish with QoS 2 for this example
	 * retain = false - do not use the retained message feature for this message
	 */
	rc = mosquitto_publish(mosq, NULL, "solar22/nivelboiler", strlen(payload), payload, 2, false);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
	}
}


int main(int argc, char *argv[])
{
	int clientInstanceData = 2222;
	
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
	mosq = mosquitto_new("romulo-publish", true, &clientInstanceData);
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


	/* Set the publish callback.  This is called when a message initiated with mosquitto_publish has been sent to the broker successfully.
		- a valid mosquitto instance.
		- a callback function in the following form: void callback(struct mosquitto *mosq, void *obj, int mid)
	*/		
	mosquitto_publish_callback_set(mosq, on_publish);


	/* Connect to an MQTT broker.
		- a valid mosquitto instance
		- the hostname or ip address of the broker to connect to
		- the network port to connect to.  Usually 1883.
		- the number of seconds after which the broker should send a PING message to the client if no other messages have been exchanged in that time.
	*/
	rc = mosquitto_connect(mosq, "localhost", 1883, 120);
	printf("   mosquitto_connect retornou:  %s\n", msgRetorno(rc) );
	if(rc != MOSQ_ERR_SUCCESS) {
		printf("   Could not connect to Broker with return code %d\n", rc);
		/* Use to free memory associated with a mosquitto client instance.
			- a struct mosquitto pointer to free.
		*/
		mosquitto_destroy(mosq);
		return -1;
	}


	/* Run the network loop in a background thread, this call returns quickly.
	*/
	rc = mosquitto_loop_start(mosq);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}


	/* At this point the client is connected to the network socket, but may not
	 * have completed CONNECT/CONNACK.
	 * It is fairly safe to start queuing messages at this point, but if you
	 * want to be really sure you should wait until after a successful call to
	 * the connect callback.
	 * In this case we know it is 1 second before we start publishing.
	 */
	while(1){
		publish_sensor_data(mosq);
	}

	mosquitto_lib_cleanup();
	return 0;
}




