// Copyright 2022 liangzhuzhi2020@gmail.com, https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This file is called by tbc_mqtt_helper.c/.h.

#include <string.h>

#include "esp_err.h"

#include "tbc_mqtt_helper_internal.h"

const static char *TAG = "provision";

/*!< Initialize deviceprovision_t */
static deviceprovision_t *_deviceprovision_create(tbcmh_handle_t client, int request_id,
                                         const tbcmh_provision_params_t *params,
                                         void *context,
                                         tbcmh_provision_on_response_t on_response,
                                         tbcmh_provision_on_timeout_t on_timeout)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(on_response, NULL);

    deviceprovision_t *provision = TBC_MALLOC(sizeof(deviceprovision_t));
    if (!provision) {
        TBC_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(provision, 0x00, sizeof(deviceprovision_t));
    provision->client = client;
    provision->params = cJSON_Duplicate(params, true);
    provision->request_id = request_id;
    provision->timestamp = (uint64_t)time(NULL);
    provision->context = context;
    provision->on_response = on_response;
    provision->on_timeout = on_timeout;
    return provision;
}

/*!< Destroys the deviceprovision_t */
static tbc_err_t _deviceprovision_destroy(deviceprovision_t *provision)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(provision, ESP_FAIL);

    cJSON_Delete(provision->params);
    provision->params = NULL;
    TBC_FREE(provision);
    return ESP_OK;
}

// return ESP_OK on successful, ESP_FAIL on failure
static int __params_of_credentials_generated_by_server(tbcmh_provision_params_t *params, const tbc_provison_config_t *config)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(params, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config, ESP_FAIL);
    // TBC_CHECK_PTR_WITH_RETURN_VALUE(config->deviceName, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceKey, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceSecret, ESP_FAIL);

    if (config->deviceName) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_DEVICE_NAME, config->deviceName);
    }
    if (config->provisionDeviceKey) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_KEY, config->provisionDeviceKey);
    }
    if (config->provisionDeviceSecret) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_SECRET, config->provisionDeviceSecret);
    }
    return ESP_OK;
}

// return ESP_OK on successful, ESP_FAIL on failure
static int __params_of_devices_supplies_access_token(tbcmh_provision_params_t *params, const tbc_provison_config_t *config)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(params, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config, ESP_FAIL);
    // TBC_CHECK_PTR_WITH_RETURN_VALUE(config->deviceName, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceKey, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceSecret, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->token, ESP_FAIL);

    if (config->deviceName) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_DEVICE_NAME, config->deviceName);
    }
    if (config->provisionDeviceKey) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_KEY, config->provisionDeviceKey);
    }
    if (config->provisionDeviceSecret) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_SECRET, config->provisionDeviceSecret);
    }
    cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_CREDENTIALS_TYPE, TB_MQTT_VALUE_PROVISION_ACCESS_TOKEN); //Credentials type parameter.
    if (config->token) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_TOKEN, config->token);
    }
    return ESP_OK;
}

// return ESP_OK on successful, ESP_FAIL on failure
static int __params_of_devices_supplies_basic_mqtt_credentials(tbcmh_provision_params_t *params, const tbc_provison_config_t *config)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(params, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config, ESP_FAIL);
    // TBC_CHECK_PTR_WITH_RETURN_VALUE(config->deviceName, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceKey, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceSecret, ESP_FAIL);
    if (!config->clientId && !config->username) {
         TBC_LOGE("config->clientId and config->username are NULL! %s()", __FUNCTION__);
         return ESP_FAIL;
    }

    if (config->deviceName) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_DEVICE_NAME, config->deviceName);
    }
    if (config->provisionDeviceKey) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_KEY, config->provisionDeviceKey);
    }
    if (config->provisionDeviceSecret) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_SECRET, config->provisionDeviceSecret);
    }
    cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_CREDENTIALS_TYPE, TB_MQTT_VALUE_PROVISION_MQTT_BASIC); //Credentials type parameter.
    if (config->clientId) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_CLIENT_ID, config->clientId);
    }
    if (config->username) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_USERNAME, config->username);
    }
    if (config->password) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PASSWORD, config->password);
    }
    return ESP_OK;
}

// hash - Public key X509 hash for device in ThingsBoard.
// return ESP_OK on successful, ESP_FAIL on failure
static int __params_of_devices_supplies_x509_certificate(tbcmh_provision_params_t *params, const tbc_provison_config_t *config)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(params, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config, ESP_FAIL);
    // TBC_CHECK_PTR_WITH_RETURN_VALUE(config->deviceName, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceKey, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->provisionDeviceSecret, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(config->hash, ESP_FAIL);

    if (config->deviceName) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_DEVICE_NAME, config->deviceName);
    }
    if (config->provisionDeviceKey) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_KEY, config->provisionDeviceKey);
    }
    if (config->provisionDeviceSecret) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_PROVISION_DEVICE_SECRET, config->provisionDeviceSecret);
    }
    cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_CREDENTIALS_TYPE, TB_MQTT_VALUE_PROVISION_X509_CERTIFICATE); //Credentials type parameter.
    if (config->hash) {
         cJSON_AddStringToObject(params, TB_MQTT_KEY_PROVISION_HASH, config->hash);  ////Public key X509 hash for device in ThingsBoard.
    }
    return ESP_OK;
}

static int _deviceprovision_request_with_params(tbcmh_handle_t client,
                                const tbcmh_provision_params_t *params,
                                void *context,
                                tbcmh_provision_on_response_t on_response,
                                tbcmh_provision_on_timeout_t on_timeout)
{
     TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);
     TBC_CHECK_PTR_WITH_RETURN_VALUE(params, ESP_FAIL);

     // Take semaphore
     if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
          TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
          return ESP_FAIL;
     }

     // Send msg to server
     int request_id = _tbcmh_get_request_id(client);
     if (request_id <= 0) {
          TBC_LOGE("failure to getting request id!");
          return -1;
     }
     //cJSON *object = cJSON_CreateObject(); // create json object
     //cJSON_AddStringToObject(object, TB_MQTT_TEXT_PROVISION_METHOD, method);
     //if (params)
     //     cJSON_AddItemReferenceToObject(object, TB_MQTT_TEXT_PROVISION_PARAMS, params);
     //else 
     //     cJSON_AddNullToObject(object, TB_MQTT_TEXT_PROVISION_PARAMS);
     //char *params_str = cJSON_PrintUnformatted(object); //cJSON_Print(object);
     char *params_str = cJSON_PrintUnformatted(params); //cJSON_Print(object);
     int msg_id = tbcm_provision_request(client->tbmqttclient, params_str, request_id,
                              1/*qos*/, 0/*retain*/);
     cJSON_free(params_str); // free memory
     //cJSON_Delete(object); // delete json object
     if (msg_id<0) {
          TBC_LOGE("Init tbcm_provision_request failure! %s()", __FUNCTION__);
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Create provision
     deviceprovision_t *provision = _deviceprovision_create(client, request_id, params, context, on_response, on_timeout);
     if (!provision) {
          TBC_LOGE("Init provision failure! %s()", __FUNCTION__);
          xSemaphoreGive(client->_lock);
          return ESP_FAIL;
     }

     // Insert provision to list
     deviceprovision_t *it, *last = NULL;
     if (LIST_FIRST(&client->deviceprovision_list) == NULL) {
          // Insert head
          LIST_INSERT_HEAD(&client->deviceprovision_list, provision, entry);
     } else {
          // Insert last
          LIST_FOREACH(it, &client->deviceprovision_list, entry) {
               last = it;
          }
          if (it == NULL) {
               assert(last);
               LIST_INSERT_AFTER(last, provision, entry);
          }
     }

     // Give semaphore
     xSemaphoreGive(client->_lock);
     return request_id;
}

// return request_id or ESP_FAIL
int tbcmh_deviceprovision_request(tbcmh_handle_t client,
                                    const tbc_provison_config_t *config,
                                    void *context,
                                    tbcmh_provision_on_response_t on_response,
                                    tbcmh_provision_on_timeout_t on_timeout)
{
    //TBC_CHECK_PTR_WITH_RETURN_VALUE(config, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(client, ESP_FAIL);

    tbcmh_provision_params_t *params = cJSON_CreateObject();
    if (!params) {
         TBC_LOGE("create params is error(NULL)!");
         return ESP_FAIL;
    }
    int ret = ESP_FAIL;
    if (config->provisionType == TBC_PROVISION_TYPE_SERVER_GENERATES_CREDENTIALS) { // Credentials generated by the ThingsBoard server
         ret = __params_of_credentials_generated_by_server(params, config);
    } else if (config->provisionType == TBC_PROVISION_TYPE_DEVICE_SUPPLIES_ACCESS_TOKEN) { // Devices supplies Access Token
         ret = __params_of_devices_supplies_access_token(params, config);
    } else if (config->provisionType == TBC_PROVISION_TYPE_DEVICE_SUPPLIES_BASIC_MQTT_CREDENTIALS) { // Devices supplies Basic MQTT Credentials
         ret = __params_of_devices_supplies_basic_mqtt_credentials(params, config);
    } else if (config->provisionType == TBC_PROVISION_TYPE_DEVICE_SUPPLIES_X509_CREDENTIALS) { // Devices supplies X.509 Certificate)
         ret = __params_of_devices_supplies_x509_certificate(params, config);
    } else {
         TBC_LOGE("config->provisionType(%d) is error!", config->provisionType);
         ret = ESP_FAIL;
    }

    if (ret != ESP_OK) {
         // TBC_LOGE("ret is error!", ret);
         cJSON_Delete(params); // delete json object     
         return ESP_FAIL;
    }

     // request_id
     ret = _deviceprovision_request_with_params(client, params, context, on_response, on_timeout);
     cJSON_Delete(params); // delete json object     
     return ret;
}

void _tbcmh_deviceprovision_on_create(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    // Take semaphore
    // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }

    // list create
    memset(&client->deviceprovision_list, 0x00, sizeof(client->deviceprovision_list)); //client->deviceprovision_list = LIST_HEAD_INITIALIZER(client->deviceprovision_list);

    // Give semaphore
    // xSemaphoreGive(client->_lock);
}

void _tbcmh_deviceprovision_on_destroy(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    // Take semaphore
    // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }

    memset(&client->deviceprovision_list, 0x00, sizeof(client->deviceprovision_list));

    // Give semaphore
    // xSemaphoreGive(client->_lock);
}

void _tbcmh_deviceprovision_on_connected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client)
    int msg_id = tbcm_subscribe(client->tbmqttclient, TB_MQTT_TOPIC_PROVISION_RESPONSE, 0);
    TBC_LOGI("sent subscribe successful, msg_id=%d, topic=%s",
                msg_id, TB_MQTT_TOPIC_PROVISION_RESPONSE);
}

void _tbcmh_deviceprovision_on_disconnected(tbcmh_handle_t client)
{
    // This function is in semaphore/client->_lock!!!
    TBC_CHECK_PTR(client);

    // Take semaphore
    // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
    //      TBC_LOGE("Unable to take semaphore!");
    //      return;
    // }

    // remove all item in deviceprovision_list
    _tbcmh_deviceprovision_on_check_timeout(client, (uint64_t)time(NULL)+ TB_MQTT_TIMEOUT + 2);
    memset(&client->deviceprovision_list, 0x00, sizeof(client->deviceprovision_list));

    // Give semaphore
    // xSemaphoreGive(client->_lock);
}

static char *__parse_string_item(const cJSON *object, const char* key)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(object, NULL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(key, NULL);

    cJSON *item = cJSON_GetObjectItem(object, key);
    if (!item) {
        TBC_LOGW("%s is not existed!", key);
        return NULL;
    }
    char *string_value = cJSON_GetStringValue(item);
    if (!string_value) {
        TBC_LOGW("%s is NULL!", key);
        return NULL;
    }
    return string_value;
}

static int __parse_provision_response(const tbcmh_provision_results_t *results,
                                          tbc_transport_credentials_config_t *credentials)
{
    TBC_CHECK_PTR_WITH_RETURN_VALUE(results, ESP_FAIL);
    TBC_CHECK_PTR_WITH_RETURN_VALUE(credentials, ESP_FAIL);

    bool isSuccess = false;
    cJSON *status = cJSON_GetObjectItem(results, TB_MQTT_KEY_PROVISION_STATUS);
    if (status) {
        if (strcmp(cJSON_GetStringValue(status), TB_MQTT_VALUE_PROVISION_SUCCESS) ==0) {
            isSuccess = true;
        }
    }
    cJSON *provisionDeviceStatus = cJSON_GetObjectItem(results, TB_MQTT_KEY_PROVISION_DEVICE_STATUS);
    if (provisionDeviceStatus) {
        if (strcmp(cJSON_GetStringValue(provisionDeviceStatus), TB_MQTT_VALUE_PROVISION_SUCCESS) ==0) {
            isSuccess = true;
        }
    }
    if (!isSuccess) {
        char *results_str = cJSON_PrintUnformatted(results); //cJSON_Print(object);
        TBC_LOGW("provision response is failure! %s", results_str);
        cJSON_free(results_str); // free memory
        return ESP_FAIL;
    }

    char *credentialsTypeStr = __parse_string_item(results, TB_MQTT_KEY_PROVISION_CREDENTIALS_TYPE);
    if (!credentialsTypeStr) {
        TBC_LOGW("credentialsType is error!");
        return ESP_FAIL;
    }

    if (strcmp(credentialsTypeStr, TB_MQTT_VALUE_PROVISION_ACCESS_TOKEN) == 0) {
        //{
        //  "credentialsType":"ACCESS_TOKEN",
        //  "credentialsValue":"sLzc0gDAZPkGMzFVTyUY"
        //  "status":"SUCCESS"
        //}
        credentials->type = TBC_TRANSPORT_CREDENTIALS_TYPE_ACCESS_TOKEN;
        credentials->token = __parse_string_item(results, TB_MQTT_KEY_PROVISION_CREDENTIALS_VALUE);
        if (!credentials->token){
            TBC_LOGW("credentialsValue is error!");
            return ESP_FAIL;
        }

    } else if (strcmp(credentialsTypeStr, TB_MQTT_VALUE_PROVISION_MQTT_BASIC) == 0) {
        //{
        //  "credentialsType":"MQTT_BASIC",
        //  "credentialsValue": {
        //    "clientId":"DEVICE_CLIENT_ID_HERE",
        //    "userName":"DEVICE_USERNAME_HERE",
        //    "password":"DEVICE_PASSWORD_HERE"
        //    },
        //  "status":"SUCCESS"
        //}
        credentials->type = TBC_TRANSPORT_CREDENTIALS_TYPE_BASIC_MQTT;
        cJSON *credentialsValue = cJSON_GetObjectItem(results, TB_MQTT_KEY_PROVISION_CREDENTIALS_VALUE);
        if (!credentialsValue){
            TBC_LOGW("credentialsValue is NOT existed!");
            return ESP_FAIL;
        }
        credentials->client_id = __parse_string_item(credentialsValue, TB_MQTT_KEY_PROVISION_CLIENT_ID);
        credentials->username  = __parse_string_item(credentialsValue, TB_MQTT_KEY_PROVISION_USERNAME);
        if (!credentials->username) {
            credentials->username  = __parse_string_item(credentialsValue, TB_MQTT_KEY_PROVISION_USERNAME2);
        }
        credentials->password  = __parse_string_item(credentialsValue, TB_MQTT_KEY_PROVISION_PASSWORD);

    } else if (strcmp(credentialsTypeStr, TB_MQTT_VALUE_PROVISION_X509_CERTIFICATE) == 0) {
        //{
        //  "deviceId":"3b829220-232f-11eb-9d5c-e9ed3235dff8",
        //  "credentialsType":"X509_CERTIFICATE",
        //  "credentialsId":"f307a1f717a12b32c27203cf77728d305d29f64694a8311be921070dd1259b3a",
        //  "credentialsValue":"MIIB........AQAB",
        //  "provisionDeviceStatus":"SUCCESS"
        //}
        credentials->type = TBC_TRANSPORT_CREDENTIALS_TYPE_X509;
        credentials->token = __parse_string_item(results, TB_MQTT_KEY_PROVISION_CREDENTIALS_VALUE);
        if (!credentials->token){
            TBC_LOGW("credentialsValue is error!");
            return ESP_FAIL;
        }

    } else {
        TBC_LOGW("credentialsType(%s) is error!", credentialsTypeStr);
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

//on response.
void _tbcmh_deviceprovision_on_data(tbcmh_handle_t client, int request_id, const cJSON *object)
{
     TBC_CHECK_PTR(client);
     TBC_CHECK_PTR(object);

     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
     //      return;
     // }

     // Search provision
     deviceprovision_t *provision = NULL, *next;
     LIST_FOREACH_SAFE(provision, &client->deviceprovision_list, entry, next) {
          if (provision && (provision->request_id==request_id)) {
              LIST_REMOVE(provision, entry);
              break;
          }
     }

     // Give semaphore
     // xSemaphoreGive(client->_lock);

     if (!provision) {
          TBC_LOGW("Unable to find provision:%d! %s()", request_id, __FUNCTION__);
          return;
     }

     // Do response - parse results of provision response
     tbc_transport_credentials_config_t credentials = {0};
     int result = __parse_provision_response(object, &credentials);
     if (result == ESP_OK) {
         provision->on_response(provision->client, provision->context,
                                provision->request_id, &credentials);
     } else {
         provision->on_timeout(provision->client, provision->context,
                                provision->request_id); // TODO: a new faiure callback?
     }

     // Free cache
     _deviceprovision_destroy(provision);
}

void _tbcmh_deviceprovision_on_check_timeout(tbcmh_handle_t client, uint64_t timestamp)
{
     TBC_CHECK_PTR(client);

     // Take semaphore
     // if (xSemaphoreTake(client->_lock, (TickType_t)0xFFFFF) != pdTRUE) {
     //      TBC_LOGE("Unable to take semaphore! %s()", __FUNCTION__);
     //      return;
     // }

     // Search & move timeout item to timeout_list
     deviceprovision_list_t timeout_list = LIST_HEAD_INITIALIZER(timeout_list);
     deviceprovision_t *request = NULL, *next;
     LIST_FOREACH_SAFE(request, &client->deviceprovision_list, entry, next) {
          if (request && request->timestamp + TB_MQTT_TIMEOUT <= timestamp) {
               LIST_REMOVE(request, entry);
               // append to timeout list
               deviceprovision_t *it, *last = NULL;
               if (LIST_FIRST(&timeout_list) == NULL) {
                    LIST_INSERT_HEAD(&timeout_list, request, entry);
               } else {
                    LIST_FOREACH(it, &timeout_list, entry) {
                         last = it;
                    }
                    if (it == NULL) {
                         assert(last);
                         LIST_INSERT_AFTER(last, request, entry);
                    }
               }
          }
     }

     // Give semaphore
     // xSemaphoreGive(client->_lock);

     // Deal timeout
     bool clientIsValid = true;
     LIST_FOREACH_SAFE(request, &timeout_list, entry, next) {
          int result = 0;
          if (clientIsValid && request->on_timeout) {
              result = request->on_timeout(request->client, request->context,
                    request->request_id);
          }
          if (result == 2) { // result is equal to 2 if calling tbcmh_disconnect()/tbcmh_destroy() inside on_timeout()
              clientIsValid = false;
          }

          LIST_REMOVE(request, entry);
          _deviceprovision_destroy(request);
     }
}

