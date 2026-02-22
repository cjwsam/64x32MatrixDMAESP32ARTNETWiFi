#include "web_server.h"
#include "config_store.h"
#include "app_config.h"
#include "esp_http_server.h"
#include "esp_ota_ops.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_app_format.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char *TAG = "web_server";
static httpd_handle_t s_server = NULL;
static uint32_t s_fps = 0;
static uint32_t s_frames_ok = 0;
static uint32_t s_frames_partial = 0;

/* ── Embedded HTML config page ─────────────────────────────── */
static const char CONFIG_HTML[] =
"<!DOCTYPE html><html><head><meta charset=utf-8><meta name=viewport content='width=device-width,initial-scale=1'>"
"<title>ArtNet HUB75 Controller</title><style>"
"*{box-sizing:border-box;margin:0;padding:0}body{font-family:system-ui,-apple-system,sans-serif;"
"background:#0a0a0a;color:#e0e0e0;padding:20px;max-width:600px;margin:0 auto}"
"h1{color:#00d4ff;margin-bottom:20px;font-size:1.4em}h2{color:#888;font-size:.9em;margin:20px 0 10px;"
"text-transform:uppercase;letter-spacing:1px}"
".card{background:#1a1a1a;border:1px solid #333;border-radius:8px;padding:16px;margin-bottom:12px}"
"label{display:block;color:#aaa;font-size:.85em;margin-bottom:4px}"
"input,select{width:100%;padding:8px;background:#0a0a0a;border:1px solid #444;border-radius:4px;"
"color:#e0e0e0;font-size:.95em;margin-bottom:12px}input:focus,select:focus{border-color:#00d4ff;outline:none}"
"input[type=range]{padding:0}button{padding:10px 20px;border:none;border-radius:4px;cursor:pointer;"
"font-weight:600;font-size:.95em;margin-right:8px;margin-bottom:8px}"
".btn-primary{background:#00d4ff;color:#000}.btn-danger{background:#ff4444;color:#fff}"
".btn-primary:hover{background:#00b8e6}.btn-danger:hover{background:#cc3333}"
".status{display:grid;grid-template-columns:1fr 1fr;gap:8px}"
".stat{background:#111;padding:8px;border-radius:4px;text-align:center}"
".stat .val{font-size:1.3em;color:#00d4ff;font-weight:700}.stat .lbl{font-size:.75em;color:#666}"
".progress{width:100%;height:6px;background:#333;border-radius:3px;overflow:hidden;margin:8px 0}"
".progress-bar{height:100%;background:#00d4ff;transition:width .3s}"
"#ota-status{font-size:.85em;color:#888;margin-top:4px}"
"</style></head><body>"
"<h1>ArtNet HUB75 Controller</h1>"

"<h2>Status</h2>"
"<div class='card status' id=status>"
"<div class=stat><div class=val id=s-ip>--</div><div class=lbl>IP Address</div></div>"
"<div class=stat><div class=val id=s-rssi>--</div><div class=lbl>RSSI</div></div>"
"<div class=stat><div class=val id=s-fps>--</div><div class=lbl>FPS</div></div>"
"<div class=stat><div class=val id=s-heap>--</div><div class=lbl>Free Heap</div></div>"
"</div>"

"<h2>WiFi</h2>"
"<div class=card>"
"<label>SSID</label><input id=wifi_ssid maxlength=32>"
"<label>Password</label><input id=wifi_pass type=password maxlength=64>"
"</div>"

"<h2>Art-Net</h2>"
"<div class=card>"
"<label>Start Universe</label><input id=start_uni type=number min=0 max=32767>"
"<label>Node Name</label><input id=node_name maxlength=17>"
"</div>"

"<h2>Display</h2>"
"<div class=card>"
"<label>Panel Width</label><input id=panel_w type=number min=16 max=256>"
"<label>Panel Height</label><input id=panel_h type=number min=8 max=128>"
"<label>Scan Type</label><select id=scan_type>"
"<option value=4>1/4 Scan</option><option value=8>1/8 Scan</option>"
"<option value=16>1/16 Scan</option><option value=32>1/32 Scan</option></select>"
"<label>Brightness: <span id=br-val>128</span></label>"
"<input id=brightness type=range min=1 max=255>"
"</div>"

"<button class=btn-primary onclick=saveConfig()>Save Configuration</button>"

"<h2>Firmware Update</h2>"
"<div class=card>"
"<input type=file id=fw-file accept='.bin'>"
"<div class=progress><div class=progress-bar id=progress style='width:0%'></div></div>"
"<div id=ota-status></div>"
"<button class=btn-danger onclick=uploadFW()>Upload Firmware</button>"
"</div>"

"<button class=btn-danger onclick=\"if(confirm('Restart?'))fetch('/api/restart',{method:'POST'})\">Restart</button>"

"<script>"
"const $=id=>document.getElementById(id);"
"$('brightness').oninput=e=>$('br-val').textContent=e.target.value;"

"async function loadStatus(){"
"try{const r=await fetch('/api/status');const d=await r.json();"
"$('s-ip').textContent=d.ip||'--';"
"$('s-rssi').textContent=(d.rssi||0)+'dBm';"
"$('s-fps').textContent=d.fps||0;"
"$('s-heap').textContent=((d.heap||0)/1024).toFixed(0)+'K';"
"}catch(e){}}"

"async function loadConfig(){"
"try{const r=await fetch('/api/config');const d=await r.json();"
"$('wifi_ssid').value=d.wifi_ssid||'';"
"$('wifi_pass').value=d.wifi_pass||'';"
"$('start_uni').value=d.start_uni||0;"
"$('node_name').value=d.node_name||'';"
"$('panel_w').value=d.panel_w||64;"
"$('panel_h').value=d.panel_h||32;"
"$('scan_type').value=d.scan_type||16;"
"$('brightness').value=d.brightness||128;"
"$('br-val').textContent=d.brightness||128;"
"}catch(e){}}"

"async function saveConfig(){"
"const body=JSON.stringify({"
"wifi_ssid:$('wifi_ssid').value,wifi_pass:$('wifi_pass').value,"
"start_uni:+$('start_uni').value,node_name:$('node_name').value,"
"panel_w:+$('panel_w').value,panel_h:+$('panel_h').value,"
"scan_type:+$('scan_type').value,brightness:+$('brightness').value"
"});"
"try{const r=await fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body});"
"if(r.ok)alert('Saved! Restart to apply WiFi changes.');else alert('Save failed');"
"}catch(e){alert('Error: '+e);}}"

"async function uploadFW(){"
"const f=$('fw-file').files[0];if(!f){alert('Select a .bin file');return;}"
"$('ota-status').textContent='Uploading...';"
"const xhr=new XMLHttpRequest();"
"xhr.open('POST','/api/ota');"
"xhr.upload.onprogress=e=>{if(e.lengthComputable){"
"const p=Math.round(e.loaded/e.total*100);"
"$('progress').style.width=p+'%';"
"$('ota-status').textContent='Uploading: '+p+'%';}};"
"xhr.onload=()=>{$('ota-status').textContent=xhr.status===200?'Done! Restarting...':'Failed: '+xhr.responseText;};"
"xhr.onerror=()=>{$('ota-status').textContent='Upload error';};"
"xhr.send(f);}"

"loadConfig();loadStatus();setInterval(loadStatus,3000);"
"</script></body></html>";

/* ── Status endpoint ───────────────────────────────────────── */
static esp_err_t handle_get_status(httpd_req_t *req)
{
    char buf[256];
    int8_t rssi = 0;
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        rssi = ap_info.rssi;
    }

    esp_netif_ip_info_t ip_info = {};
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif) esp_netif_get_ip_info(netif, &ip_info);

    char ip_str[16];
    snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&ip_info.ip));

    int len = snprintf(buf, sizeof(buf),
        "{\"ip\":\"%s\",\"rssi\":%d,\"fps\":%lu,\"frames_ok\":%lu,"
        "\"frames_partial\":%lu,\"heap\":%lu,\"uptime\":%lld}",
        ip_str, rssi, (unsigned long)s_fps, (unsigned long)s_frames_ok,
        (unsigned long)s_frames_partial,
        (unsigned long)esp_get_free_heap_size(),
        esp_timer_get_time() / 1000000LL);

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, buf, len);
}

/* ── Config GET ────────────────────────────────────────────── */
static esp_err_t handle_get_config(httpd_req_t *req)
{
    const app_config_t *cfg = config_store_get();
    char buf[512];
    int len = snprintf(buf, sizeof(buf),
        "{\"wifi_ssid\":\"%s\",\"wifi_pass\":\"%s\","
        "\"start_uni\":%u,\"node_name\":\"%s\","
        "\"panel_w\":%u,\"panel_h\":%u,"
        "\"scan_type\":%u,\"brightness\":%u,\"gamma_en\":%u}",
        cfg->wifi_ssid, cfg->wifi_pass,
        cfg->start_universe, cfg->node_name,
        cfg->panel_width, cfg->panel_height,
        cfg->scan_type, cfg->brightness, cfg->gamma_enabled);

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, buf, len);
}

/* ── Simple JSON value extraction (no dependency on cJSON) ── */
static bool json_get_str(const char *json, const char *key, char *dst, size_t max)
{
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":\"", key);
    const char *p = strstr(json, search);
    if (!p) return false;
    p += strlen(search);
    const char *end = strchr(p, '"');
    if (!end) return false;
    size_t len = end - p;
    if (len >= max) len = max - 1;
    memcpy(dst, p, len);
    dst[len] = '\0';
    return true;
}

static bool json_get_int(const char *json, const char *key, int *val)
{
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":", key);
    const char *p = strstr(json, search);
    if (!p) return false;
    p += strlen(search);
    while (*p == ' ') p++;
    *val = atoi(p);
    return true;
}

/* ── Config POST ───────────────────────────────────────────── */
static esp_err_t handle_post_config(httpd_req_t *req)
{
    char buf[512];
    int received = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (received <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Empty body");
        return ESP_FAIL;
    }
    buf[received] = '\0';

    app_config_t cfg;
    memcpy(&cfg, config_store_get(), sizeof(cfg));

    json_get_str(buf, "wifi_ssid", cfg.wifi_ssid, sizeof(cfg.wifi_ssid));
    json_get_str(buf, "wifi_pass", cfg.wifi_pass, sizeof(cfg.wifi_pass));
    json_get_str(buf, "node_name", cfg.node_name, sizeof(cfg.node_name));

    int v;
    if (json_get_int(buf, "start_uni",  &v)) cfg.start_universe = (uint16_t)v;
    if (json_get_int(buf, "brightness", &v)) cfg.brightness = (uint8_t)v;
    if (json_get_int(buf, "panel_w",    &v)) cfg.panel_width = (uint16_t)v;
    if (json_get_int(buf, "panel_h",    &v)) cfg.panel_height = (uint16_t)v;
    if (json_get_int(buf, "scan_type",  &v)) cfg.scan_type = (uint8_t)v;

    esp_err_t err = config_store_save(&cfg);
    if (err == ESP_OK) {
        httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Save failed");
    }
    return err;
}

/* ── OTA upload ────────────────────────────────────────────── */
static esp_err_t handle_post_ota(httpd_req_t *req)
{
    const esp_partition_t *update = esp_ota_get_next_update_partition(NULL);
    if (!update) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No OTA partition");
        return ESP_FAIL;
    }

    esp_ota_handle_t ota;
    esp_err_t err = esp_ota_begin(update, OTA_WITH_SEQUENTIAL_WRITES, &ota);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "OTA begin failed: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OTA begin failed");
        return ESP_FAIL;
    }

    char buf[1024];
    int remaining = req->content_len;

    while (remaining > 0) {
        int received = httpd_req_recv(req, buf, (remaining < (int)sizeof(buf)) ? remaining : (int)sizeof(buf));
        if (received <= 0) {
            esp_ota_abort(ota);
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Receive error");
            return ESP_FAIL;
        }
        err = esp_ota_write(ota, buf, received);
        if (err != ESP_OK) {
            esp_ota_abort(ota);
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Write error");
            return ESP_FAIL;
        }
        remaining -= received;
    }

    err = esp_ota_end(ota);
    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OTA validation failed");
        return ESP_FAIL;
    }

    esp_ota_set_boot_partition(update);
    httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
    ESP_LOGI(TAG, "OTA complete, restarting in 1s...");

    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
    return ESP_OK;
}

/* ── Restart ───────────────────────────────────────────────── */
static esp_err_t handle_post_restart(httpd_req_t *req)
{
    httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
    vTaskDelay(pdMS_TO_TICKS(500));
    esp_restart();
    return ESP_OK;
}

/* ── Root page ─────────────────────────────────────────────── */
static esp_err_t handle_get_root(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, CONFIG_HTML, strlen(CONFIG_HTML));
}

/* ── Captive portal redirect ───────────────────────────────── */
static esp_err_t captive_redirect(httpd_req_t *req, httpd_err_code_t err_code)
{
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "http://192.168.4.1/");
    return httpd_resp_send(req, NULL, 0);
}

/* ── URI definitions ───────────────────────────────────────── */
static const httpd_uri_t uri_get_root     = { .uri = "/",            .method = HTTP_GET,  .handler = handle_get_root };
static const httpd_uri_t uri_get_status   = { .uri = "/api/status",  .method = HTTP_GET,  .handler = handle_get_status };
static const httpd_uri_t uri_get_config   = { .uri = "/api/config",  .method = HTTP_GET,  .handler = handle_get_config };
static const httpd_uri_t uri_post_config  = { .uri = "/api/config",  .method = HTTP_POST, .handler = handle_post_config };
static const httpd_uri_t uri_post_ota     = { .uri = "/api/ota",     .method = HTTP_POST, .handler = handle_post_ota };
static const httpd_uri_t uri_post_restart = { .uri = "/api/restart", .method = HTTP_POST, .handler = handle_post_restart };

esp_err_t web_server_start(bool provisioning_mode)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 8;
    config.stack_size = WEB_SERVER_STACK;
    config.max_open_sockets = 2;
    config.lru_purge_enable = true;

    esp_err_t err = httpd_start(&s_server, &config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server: %s", esp_err_to_name(err));
        return err;
    }

    httpd_register_uri_handler(s_server, &uri_get_root);
    httpd_register_uri_handler(s_server, &uri_get_status);
    httpd_register_uri_handler(s_server, &uri_get_config);
    httpd_register_uri_handler(s_server, &uri_post_config);
    httpd_register_uri_handler(s_server, &uri_post_ota);
    httpd_register_uri_handler(s_server, &uri_post_restart);

    if (provisioning_mode) {
        httpd_register_err_handler(s_server, HTTPD_404_NOT_FOUND, captive_redirect);
    }

    ESP_LOGI(TAG, "Web server started%s", provisioning_mode ? " (provisioning mode)" : "");
    return ESP_OK;
}

esp_err_t web_server_stop(void)
{
    if (s_server) {
        httpd_stop(s_server);
        s_server = NULL;
    }
    return ESP_OK;
}

void web_server_update_stats(uint32_t fps, uint32_t frames_ok, uint32_t frames_partial)
{
    s_fps = fps;
    s_frames_ok = frames_ok;
    s_frames_partial = frames_partial;
}
