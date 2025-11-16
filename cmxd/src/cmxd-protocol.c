/**
 * @file cmxd-protocol.c
 * @brief Implementation of cmxd socket communication protocol
 */

#include "cmxd-protocol.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>

/*
 * =============================================================================
 * PROTOCOL MESSAGE FORMATTING
 * =============================================================================
 */

/* Public API - used by external clients (e.g., tablet-mode-daemon) */
__attribute__((visibility("default")))
int cmxd_protocol_format_message(char *buffer, size_t buffer_size,
                                 const char *type, const char *value, 
                                 const char *previous)
{
    struct timespec ts;
    int ret;
    
    if (!buffer || !type || !value) {
        return -1;
    }
    
    /* Get timestamp */
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
        ts.tv_sec = 0;
        ts.tv_nsec = 0;
    }
    
    /* Format message as JSON for easy parsing by clients */
    ret = snprintf(buffer, buffer_size,
                   "{"
                   "\"timestamp\":%ld.%09ld,"
                   "\"type\":\"%s\","
                   "\"value\":\"%s\""
                   "%s%s%s"
                   "}\n",
                   ts.tv_sec, ts.tv_nsec,
                   type, value,
                   previous ? ",\"previous\":\"" : "",
                   previous ? previous : "",
                   previous ? "\"" : "");
    
    if (ret >= (int)buffer_size) {
        return -1;  /* Message truncated */
    }
    
    return ret;
}

/*
 * =============================================================================
 * PROTOCOL MESSAGE PARSING
 * =============================================================================
 */

/* Simple JSON parser for our specific message format */
static const char *find_json_string_value(const char *json, const char *key, 
                                          char *value_buf, size_t value_size)
{
    char search_key[64];
    const char *key_pos, *value_start, *value_end;
    size_t value_len;
    
    /* Build search pattern: "key":" */
    snprintf(search_key, sizeof(search_key), "\"%s\":\"", key);
    
    key_pos = strstr(json, search_key);
    if (!key_pos) {
        return NULL;
    }
    
    value_start = key_pos + strlen(search_key);
    value_end = strchr(value_start, '"');
    if (!value_end) {
        return NULL;
    }
    
    value_len = value_end - value_start;
    if (value_len >= value_size) {
        return NULL;  /* Value too long */
    }
    
    strncpy(value_buf, value_start, value_len);
    value_buf[value_len] = '\0';
    
    return value_buf;
}

static double find_json_timestamp(const char *json)
{
    const char *timestamp_pos, *value_start, *value_end;
    char timestamp_str[32];
    size_t value_len;
    
    timestamp_pos = strstr(json, "\"timestamp\":");
    if (!timestamp_pos) {
        return 0.0;
    }
    
    value_start = timestamp_pos + strlen("\"timestamp\":");
    value_end = strchr(value_start, ',');
    if (!value_end) {
        value_end = strchr(value_start, '}');
        if (!value_end) {
            return 0.0;
        }
    }
    
    value_len = value_end - value_start;
    if (value_len >= sizeof(timestamp_str)) {
        return 0.0;
    }
    
    strncpy(timestamp_str, value_start, value_len);
    timestamp_str[value_len] = '\0';
    
    return strtod(timestamp_str, NULL);
}

/* Public API - used by external clients (e.g., tablet-mode-daemon) */
__attribute__((visibility("default")))
int cmxd_protocol_parse_message(const char *message, 
                                struct cmxd_protocol_message *parsed)
{
    if (!message || !parsed) {
        return -1;
    }
    
    /* Initialize output structure */
    memset(parsed, 0, sizeof(*parsed));
    
    /* Parse timestamp */
    parsed->timestamp = find_json_timestamp(message);
    
    /* Parse type */
    if (!find_json_string_value(message, "type", parsed->type, sizeof(parsed->type))) {
        return -1;
    }
    
    /* Parse value */
    if (!find_json_string_value(message, "value", parsed->value, sizeof(parsed->value))) {
        return -1;
    }
    
    /* Parse optional previous value */
    if (find_json_string_value(message, "previous", parsed->previous, sizeof(parsed->previous))) {
        parsed->has_previous = true;
    } else {
        parsed->has_previous = false;
    }
    
    return 0;
}

/*
 * =============================================================================
 * PROTOCOL UTILITY FUNCTIONS
 * =============================================================================
 */

/* Public API - used by external clients (e.g., tablet-mode-daemon) */
__attribute__((visibility("default")))
int cmxd_protocol_is_tablet_mode(const char *mode_value)
{
    if (!mode_value) {
        return 0;
    }
    
    /* Only "tablet" mode is considered actual tablet mode */
    /* tent, flat, and laptop modes keep keyboard/touchpad enabled */
    return (strcmp(mode_value, CMXD_PROTOCOL_MODE_TABLET) == 0);
}
