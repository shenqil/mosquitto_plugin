#include <stdio.h>
#include <string.h>

#include "mosquitto_broker.h"
#include "mosquitto_plugin.h"
#include "mosquitto.h"
#include "mqtt_protocol.h"

#define UNUSED(A) (void)(A)

static mosquitto_plugin_id_t *mosq_pid = NULL;



static int callback_message(int event, void *event_data, void *userdata)
{
	struct mosquitto_evt_message *ed = event_data;
	char *new_payload;
	uint32_t new_payloadlen;

	UNUSED(event);
	UNUSED(userdata);

	/* This simply adds "hello " to the front of every payload. You can of
	 * course do much more complicated message processing if needed. */

	/* Calculate the length of our new payload */
	new_payloadlen = ed->payloadlen + (uint32_t)strlen("hello ")+1;

	/* Allocate some memory - use
	 * mosquitto_calloc/mosquitto_malloc/mosquitto_strdup when allocating, to
	 * allow the broker to track memory usage */
	new_payload = mosquitto_calloc(1, new_payloadlen);
	if(new_payload == NULL){
		return MOSQ_ERR_NOMEM;
	}

	/* Print "hello " to the payload */
	snprintf(new_payload, new_payloadlen, "hello ");
	memcpy(new_payload+(uint32_t)strlen("hello "), ed->payload, ed->payloadlen);

	/* Assign the new payload and payloadlen to the event data structure. You
	 * must *not* free the original payload, it will be handled by the
	 * broker. */
	ed->payload = new_payload;
	ed->payloadlen = new_payloadlen;

	return MOSQ_ERR_SUCCESS;
}

/**
 * @brief 解析配置项
 * 
 * @param opts 
 * @param opt_count 
 * @return * void 
 */
static void paser_opts(struct mosquitto_opt* opts, int opt_count) {
	struct mosquitto_opt* o = opts;
	for (size_t i = 0; i < opt_count; i++)
	{
		mosquitto_log_printf(MOSQ_LOG_INFO, "opts[%d] key = %s, v = %s",i,o->key,o->value);
		o++;
	}
}

/**
 * @brief 重载
 * 
 * @param event 
 * @param event_data 
 * @param userdata 
 * @return * int 
 */
static int callback_reload(int event, void* event_data, void* userdata) {
	UNUSED(event);
	UNUSED(userdata);

	mosquitto_log_printf(MOSQ_LOG_INFO, "[callback_reload]");

	struct mosquitto_evt_reload* ed = event_data;
	paser_opts(ed->options, ed->option_count);
	return MOSQ_ERR_SUCCESS;
}

/**
 * @brief 基础认证
 * 
 * @param event 
 * @param event_data 
 * @param userdata 
 * @return * int 
 */
static int callback_basic_auth(int event, void* event_data, void* userdata) {
	UNUSED(event);
	UNUSED(userdata);

	struct mosquitto_evt_basic_auth* ed = event_data;
	char* client_id = mosquitto_client_id(ed->client);
	char* client_address = mosquitto_client_address(ed->client);

	mosquitto_log_printf(MOSQ_LOG_INFO, "[callback_basic_auth] username=%s, password=%s, client_id=%s, client_address=%s", ed->username, ed->password, client_id, client_address);


	return MOSQ_ERR_SUCCESS;
}

/**
 * @brief acl认证
 * 
 * @param event 
 * @param event_data 
 * @param userdata 
 * @return * int 
 */
static int callback_acl_check(int event, void* event_data, void* userdata) {
	UNUSED(event);
	UNUSED(userdata);

	struct mosquitto_evt_acl_check* ed = event_data;
	char* username = mosquitto_client_username(ed->client);
	char* client_id = mosquitto_client_id(ed->client);
	char* topic = ed->topic;
	int access = ed->access;
	int qos = ed->qos;
	bool retain = ed->retain;

	mosquitto_log_printf(MOSQ_LOG_INFO, "[callback_acl_check] username=%s, client_id=%s, topic=%s, access=%d, qos=%d, retain=%d", username, client_id, topic, access, qos, retain);
	
	return MOSQ_ERR_SUCCESS;
}


int mosquitto_plugin_version(int supported_version_count, const int *supported_versions)
{
	int i;

	for(i=0; i<supported_version_count; i++){
		if(supported_versions[i] == 5){
			return 5;
		}
	}
	return -1;
}

int mosquitto_plugin_init(mosquitto_plugin_id_t *identifier, void **user_data, struct mosquitto_opt *opts, int opt_count)
{
	UNUSED(user_data);
	UNUSED(opts);
	UNUSED(opt_count);
	int err_code = MOSQ_ERR_SUCCESS;

	mosq_pid = identifier;
	mosquitto_log_printf(MOSQ_LOG_INFO, "[mosquitto_plugin_init]");
	paser_opts(opts, opt_count); 


	err_code = mosquitto_callback_register(mosq_pid, MOSQ_EVT_RELOAD, callback_reload, NULL, NULL);
	if (err_code != MOSQ_ERR_SUCCESS) {
		mosquitto_log_printf(MOSQ_LOG_ERR, "[mosquitto_plugin_init][register] MOSQ_EVT_RELOAD err = %d", err_code);
		return err_code;
	}

	err_code = mosquitto_callback_register(mosq_pid, MOSQ_EVT_BASIC_AUTH, callback_basic_auth, NULL, NULL);
	if (err_code != MOSQ_ERR_SUCCESS) {
		mosquitto_log_printf(MOSQ_LOG_ERR, "[mosquitto_plugin_init][register] MOSQ_EVT_BASIC_AUTH err = %d", err_code);
		return err_code;
	}

	err_code = mosquitto_callback_register(mosq_pid, MOSQ_EVT_ACL_CHECK, callback_acl_check, NULL, NULL);
	if (err_code != MOSQ_ERR_SUCCESS) {
		mosquitto_log_printf(MOSQ_LOG_ERR, "[mosquitto_plugin_init][register] MOSQ_EVT_ACL_CHECK err = %d", err_code);
		return err_code;
	}


	return err_code;

}

int mosquitto_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count)
{
	UNUSED(user_data);
	UNUSED(opts);
	UNUSED(opt_count);
	int err_code = MOSQ_ERR_SUCCESS;
	mosquitto_log_printf(MOSQ_LOG_INFO, "[mosquitto_plugin_cleanup]");

	err_code = mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_RELOAD, callback_reload, NULL);
	if (err_code != MOSQ_ERR_SUCCESS) {
		mosquitto_log_printf(MOSQ_LOG_ERR, "[mosquitto_plugin_cleanup][unregister] MOSQ_EVT_RELOAD err = %d", err_code);
	}
	err_code = mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_BASIC_AUTH, callback_basic_auth, NULL);
	if (err_code != MOSQ_ERR_SUCCESS) {
		mosquitto_log_printf(MOSQ_LOG_ERR, "[mosquitto_plugin_cleanup][unregister] MOSQ_EVT_BASIC_AUTH err = %d", err_code);
	}
	err_code = mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_ACL_CHECK, callback_acl_check, NULL);
	if (err_code != MOSQ_ERR_SUCCESS) {
		mosquitto_log_printf(MOSQ_LOG_ERR, "[mosquitto_plugin_cleanup][unregister] MOSQ_EVT_ACL_CHECK err = %d", err_code);
	}

	return MOSQ_ERR_SUCCESS;
}
