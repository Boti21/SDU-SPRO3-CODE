/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"


#include "lwip/err.h"
#include "lwip/sys.h"

// Includes from simple http server example

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include "esp_netif.h"
#include <esp_http_server.h>
#include "esp_netif.h"

// Includes for hardware interface
#include "driver/gpio.h"
#define LED GPIO_NUM_4

#include "FL_webfrontend.h"

/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      "fork_connect"
#define EXAMPLE_ESP_WIFI_PASS      "forky_the_forklift"
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       4

static const char *TAG = "ForkConnect";

static char changing_text[20];

/* An HTTP GET handler */
static esp_err_t forkconnect_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "ForkConnect");

    gpio_set_level(LED, 0);
    char* resp_str = (char*) req->user_ctx;
    char* final_resp_str = NULL;

    xSemaphoreTake(web_mutex, portMAX_DELAY);
    asprintf(&final_resp_str, resp_str, changing_text);
    xSemaphoreGive(web_mutex);
    
    esp_err_t error = httpd_resp_send(req, final_resp_str, strlen(final_resp_str)); // Send the response

    free(final_resp_str);

    if (error != ESP_OK)
    {
        ESP_LOGI(TAG, "Error %d while sending Response", error);
    }
    else 
    {
        ESP_LOGI(TAG, "Response sent Successfully");
    }
    return error;
}

static const httpd_uri_t forkconnect = {
    .uri       = "/forkconnect",
    .method    = HTTP_GET,
    .handler   = forkconnect_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = FORKCONNECThtml
};
//<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Material+Symbols+Outlined:opsz,wght,FILL,GRAD@24,400,1,0" />

static esp_err_t forkpage2_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "ForkPage2");

    gpio_set_level(LED, 1);

    char* resp_str = (char*) req->user_ctx;
    char* final_resp_str = NULL;
    asprintf(&final_resp_str, resp_str, changing_text);

    esp_err_t error = httpd_resp_send(req, final_resp_str, strlen(final_resp_str)); // Send the response

    free(final_resp_str);

    if (error != ESP_OK)
    {
        ESP_LOGI(TAG, "Error %d while sending Response", error);
    }
    else 
    {
        ESP_LOGI(TAG, "Response sent Successfully");
    }
    return error;
}


static const httpd_uri_t forkpage2 = {
    .uri       = "/forkpage2",
    .method    = HTTP_GET,
    .handler   = forkpage2_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = 
"<!DOCTYPE html>\
<html>\
<head>\
<style>\
.button {\
  border: none;\
  color: white;\
  padding: 15px 32px;\
  text-align: center;\
  text-decoration: none;\
  display: inline-block;\
  font-size: 16px;\
  margin: 4px 2px;\
  cursor: pointer;\
}\
\
.button1 {background-color: #000000;} /* Black */\
\
</style>\
</head>\
<body>\
\
<h1>Fork Connect - Your way to fork.</h1>\
<p>Control your forklift from here:</p>\
\
<h2>Start %s </h2>\
\
<button class=\"button button1\" onclick= \"window.location.href='/forkconnect'\" >Stop</button>\
</div>\
\
<nav>\
<label class=\"logo\">Design</label>\
</nav>\
</body>\
<script>\
setInterval(function() {\
    var xhttp = new XMLHttpRequest();\
    xhttp.onreadystatechange = function() {\
        if (this.readyState == 4 && this.status == 200) {\
            document.getElementById(\"forkpage2\").innerHTML = this.responseText;\
        }\
    };\
    xhttp.open(\"GET\", \"/forkconnect\", true);\
    xhttp.send();\
}, 5000);\ 
</html>\
"
};

/*
<div>\
/<button type=\"button button1\">PALLET</button>\
<button type=\"button button1\">FIELD</button>\
<button onclick = \"doSomething()\" style=\"font-size:25px;\
*/

static esp_err_t forkconnect_input_handler(httpd_req_t *req)
{
    // Read the URI line and get the host
    char *buf;
    size_t buf_len;
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Host: %s", buf);
        }
        free(buf);
    }

    // Read the URI line and get the parameters
     buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query: %s", buf);
            char param[32];
            if (httpd_query_key_value(buf, "str", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "The string value = %s", param);
                strcpy(changing_text, param);
            }
            if (httpd_query_key_value(buf, "int", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "The int value = %s", param);
            }
        }
        free(buf);
    }

    // The response
    
    httpd_resp_set_status(req, "302 Found"); // Set the status to 302
    httpd_resp_set_hdr(req, "Location", "/forkconnect"); // Set the Location header to new URI to redirect
    httpd_resp_send(req, NULL, 0); // Send the response
    
    return ESP_OK;
}

static const httpd_uri_t forkconnect_input = {
    .uri       = "/get",
    .method    = HTTP_GET,
    .handler   = forkconnect_input_handler,
    .user_ctx  = NULL
};

static esp_err_t root_handler(httpd_req_t *req)
{
    esp_err_t error;
    ESP_LOGI(TAG, "Root handler.");

    // gpio_set_level(LED, 1);

    const char *response = (const char *) req->user_ctx;
    error = httpd_resp_send(req, response, strlen(response));
    if (error != ESP_OK)
    {
        ESP_LOGI(TAG, "Error %d while sending Response", error);
    }
    else 
    {
        ESP_LOGI(TAG, "Response sent Successfully");
    }

    return error;
}

static const httpd_uri_t root = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_handler,
    .user_ctx  = "Redirect to home page... enter http://192.168.4.1/forkconnect"
};


// 404 Error handler
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &forkconnect);
        httpd_register_uri_handler(server, &forkpage2);
        httpd_register_uri_handler(server, &forkconnect_input);
        
        
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}


static esp_err_t stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) == ESP_OK) {
            *server = NULL;
        } else {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}



static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
            .authmode = WIFI_AUTH_WPA3_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
#else /* CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT */
            .authmode = WIFI_AUTH_WPA2_PSK,
#endif
            .pmf_cfg = {
                    .required = true,
            },
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

void init_fork_connect(void)
{

    strcpy(changing_text, "undifined");

    static httpd_handle_t server = NULL;

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

    ESP_ERROR_CHECK(esp_netif_init());
    
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &connect_handler, &server));
    // ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
    
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    gpio_set_level(LED, 0);


}
