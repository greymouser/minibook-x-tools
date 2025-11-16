/**
 * @file cmxd-protocol.h
 * @brief Shared protocol definitions for cmxd socket communication
 * 
 * This header defines the message formats and parsing utilities used for
 * communication between cmxd and client applications like tablet-mode-daemon.
 */

#ifndef CMXD_PROTOCOL_H
#define CMXD_PROTOCOL_H

#include <stddef.h>
#include <stdbool.h>

/*
 * =============================================================================
 * MESSAGE FORMAT DEFINITIONS
 * =============================================================================
 */

/**
 * Maximum size for a cmxd protocol message
 */
#define CMXD_PROTOCOL_MAX_MESSAGE_SIZE 512

/**
 * Maximum size for event type string
 */
#define CMXD_PROTOCOL_MAX_TYPE_SIZE 16

/**
 * Maximum size for event value string  
 */
#define CMXD_PROTOCOL_MAX_VALUE_SIZE 32

/**
 * Event types sent over the socket
 */
#define CMXD_PROTOCOL_EVENT_MODE "mode"
#define CMXD_PROTOCOL_EVENT_ORIENTATION "orientation"

/**
 * Mode values
 */
#define CMXD_PROTOCOL_MODE_LAPTOP "laptop"
#define CMXD_PROTOCOL_MODE_FLAT "flat" 
#define CMXD_PROTOCOL_MODE_TENT "tent"
#define CMXD_PROTOCOL_MODE_TABLET "tablet"
#define CMXD_PROTOCOL_MODE_CLOSING "closing"

/**
 * Orientation values - portrait, portrait-flipped, landscape, landscape-flipped
 */
#define CMXD_PROTOCOL_ORIENTATION_PORTRAIT "portrait"
#define CMXD_PROTOCOL_ORIENTATION_PORTRAIT_FLIPPED "portrait-flipped"
#define CMXD_PROTOCOL_ORIENTATION_LANDSCAPE "landscape"
#define CMXD_PROTOCOL_ORIENTATION_LANDSCAPE_FLIPPED "landscape-flipped"

/*
 * =============================================================================
 * MESSAGE STRUCTURE
 * =============================================================================
 */

/**
 * Parsed cmxd protocol message
 */
struct cmxd_protocol_message {
    double timestamp;                                    /**< Unix timestamp with nanosecond precision */
    char type[CMXD_PROTOCOL_MAX_TYPE_SIZE];             /**< Event type (mode/orientation) */
    char value[CMXD_PROTOCOL_MAX_VALUE_SIZE];           /**< Current value */
    char previous[CMXD_PROTOCOL_MAX_VALUE_SIZE];        /**< Previous value (optional) */
    bool has_previous;                                   /**< Whether previous value is present */
};

/*
 * =============================================================================
 * PROTOCOL FUNCTIONS
 * =============================================================================
 */

/**
 * Format a cmxd protocol message for transmission
 * 
 * @param buffer Output buffer for the formatted message
 * @param buffer_size Size of the output buffer
 * @param type Event type (use CMXD_PROTOCOL_EVENT_* constants)
 * @param value Current value
 * @param previous Previous value (can be NULL)
 * @return Length of formatted message, or -1 on error
 */
int cmxd_protocol_format_message(char *buffer, size_t buffer_size,
                                 const char *type, const char *value, 
                                 const char *previous);

/**
 * Parse a cmxd protocol message
 * 
 * @param message Raw message string
 * @param parsed Output structure for parsed message
 * @return 0 on success, -1 on parse error
 */
int cmxd_protocol_parse_message(const char *message, 
                                struct cmxd_protocol_message *parsed);

/**
 * Check if a mode value represents tablet mode
 * 
 * @param mode_value Mode string to check
 * @return 1 if tablet mode, 0 otherwise
 */
int cmxd_protocol_is_tablet_mode(const char *mode_value);

#endif /* CMXD_PROTOCOL_H */