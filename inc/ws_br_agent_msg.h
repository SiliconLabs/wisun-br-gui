#ifndef WS_BR_AGENT_MSG_H
#define WS_BR_AGENT_MSG_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "ws_br_agent_defs.h"
#include "ws_br_agent_settings.h"

#ifdef __cplusplus
extern "C" {
#endif


/// Agent Service Request/Response codes
/// Topology msg code
#define WS_BR_AGENT_MSG_CODE_TOPOLOGY           (0x00000001U)
/// Get config params msg code
#define WS_BR_AGENT_MSG_CODE_GET_CONFIG_PARAMS  (0x00000002U)
/// Set config params msg code
#define WS_BR_AGENT_MSG_CODE_SET_CONFIG_PARAMS  (0x00000003U)
/// Start Border Router msg code
#define WS_BR_AGENT_MSG_CODE_START_BR           (0x00000004U)
/// Stop Border Router msg code
#define WS_BR_AGENT_MSG_CODE_STOP_BR            (0x00000005U)

/// Minimum buffer size for a message (header only, no payload)
#define WS_BR_AGENT_MSG_MIN_BUF_SIZE \
  (sizeof(ws_br_agent_msg_code_t) + sizeof(ws_br_agent_msg_len_t))

/// Buffer size for SET_CONFIG_PARAMS message
#define WS_BR_AGENT_MSG_SET_PARAM_MSG_BUF_SIZE \
  (WS_BR_AGENT_MSG_MIN_BUF_SIZE + sizeof(ws_br_agent_msg_settings_payload_t))

/// @brief Type for message codes
typedef uint32_t ws_br_agent_msg_code_t;
/// @brief Type for message payload length
typedef uint32_t ws_br_agent_msg_len_t;
/// @brief Type for SET_CONFIG_PARAMS message payload
typedef ws_br_agent_settings_t ws_br_agent_msg_settings_payload_t;

/// Packet structure:
/// [msg code 4 byte] [payload len 4 byte] [payload data n byte]
typedef struct ws_br_agent_msg {
  /// @brief Message code
  ws_br_agent_msg_code_t msg_code;
  /// @brief Length of the payload in bytes
  ws_br_agent_msg_len_t payload_len;
  /// @brief Pointer to the payload data (NULL if no payload)
  uint8_t *payload;
} ws_br_agent_msg_t;

/**
 * @brief Build a message buffer from a message structure.
 * @details The buffer is dynamically allocated and should be freed by the caller.
 * @param[in] msg Pointer to the message structure.
 * @param[out] buf_size Pointer to a variable to store the size of the built buffer.
 * @return Pointer to the built buffer, or NULL on error. The caller is responsible for freeing the buffer.
 */
uint8_t *ws_br_agent_msg_build_buf(const ws_br_agent_msg_t * const msg, size_t * const buf_size);

/**
 * @brief Parse a message buffer into a message structure.
 * @details The message structure is dynamically allocated and should be freed by the caller using ws_br_agent_msg_free().
 * @param[in] buf Pointer to the buffer containing the message.
 * @param[in] buf_size Size of the buffer in bytes.
 * @return Pointer to the parsed message structure, or NULL on error. The caller is responsible for freeing the message structure.
 */
ws_br_agent_msg_t *ws_br_agent_msg_parse_buf(const uint8_t * const buf, const size_t buf_size);

void ws_br_agent_msg_free(ws_br_agent_msg_t *msg);

#ifdef __cplusplus
}
#endif

#endif // WS_BR_AGENT_MSG_H