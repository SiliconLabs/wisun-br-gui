/***************************************************************************//**
 * @file ws_br_agent_settings.h
 * @brief Soc Border Router settings
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef WS_BR_AGENT_SETTINGS_H
#define WS_BR_AGENT_SETTINGS_H

#include "ws_br_agent_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Defines whether keys are included in the settings
#define WS_BR_AGENT_SETTINGS_HAVE_KEYS 0U

/// FAN1.1 PHY configuration
typedef struct __attribute__((packed, aligned(4))){
  /// Regulatory domain (#sl_wisun_regulatory_domain_t)
  uint8_t reg_domain;
  /// Channel plan ID
  uint8_t chan_plan_id;
  /// PHY mode ID
  uint8_t phy_mode_id;
} ws_br_agent_phy_config_fan11_t;

typedef struct __attribute__((packed, aligned(4))) {
  /// Regulatory domain (#sl_wisun_regulatory_domain_t)
  uint8_t reg_domain;
  /// Operating class (#sl_wisun_operating_class_t)
  uint8_t op_class;
  /// Operating mode (#sl_wisun_operating_mode_t)
  uint8_t op_mode;
  /// 1 if FEC is enabled, 0 if not
  uint8_t fec;
} ws_br_agent_phy_config_fan10_t;

/// Explicit PHY configuration
typedef struct __attribute__((packed, aligned(4))) {
  /// Ch0 center frequency in kHz
  uint32_t ch0_frequency_khz;
  /// Number of channels
  uint16_t number_of_channels;
  /// Channel spacing (#sl_wisun_channel_spacing_t)
  uint8_t channel_spacing;
  /// PHY mode ID
  uint8_t phy_mode_id;
} ws_br_agent_phy_config_explicit_t;


/// Explicit RAIL configuration
typedef struct __attribute__((packed, aligned(4))) {
  /// Protocol ID
  uint16_t protocol_id;
  /// Channel ID
  uint16_t channel_id;
  /// PHY mode ID
  uint8_t phy_mode_id;
  /// Reserved, set to zero
  uint8_t reserved[3];
} ws_br_agent_phy_config_ids_t;


/// Custom FSK PHY configuration
typedef struct __attribute__((packed, aligned(4))) {
  /// Ch0 center frequency in kHz
  uint32_t ch0_frequency_khz;
  /// Channel spacing in kHz
  uint16_t channel_spacing_khz;
  /// Number of channels
  uint16_t number_of_channels;
  /// PHY mode ID
  uint8_t phy_mode_id;
  /// FSK CRC type (#sl_wisun_crc_type_t)
  uint8_t crc_type;
  /// FSK preamble length in bits
  uint8_t preamble_length;
  /// Reserved, set to zero
  uint8_t reserved[1];
} ws_br_agent_phy_config_custom_fsk_t;


/// Custom OFDM PHY configuration
typedef struct __attribute__((packed, aligned(4))){
  /// Ch0 center frequency in kHz
  uint32_t ch0_frequency_khz;
  /// Channel spacing in kHz
  uint16_t channel_spacing_khz;
  /// Number of channels
  uint16_t number_of_channels;
  /// PHY mode ID
  uint8_t phy_mode_id;
  /// OFDM CRC type (#sl_wisun_crc_type_t)
  uint8_t crc_type;
  /// STF length in number of symbols
  uint8_t stf_length;
  /// Reserved, set to zero
  uint8_t reserved[1];
} ws_br_agent_phy_config_custom_ofdm_t;

/// Custom OQPSK PHY configuration
typedef struct __attribute__((packed, aligned(4))) {
  /// Ch0 center frequency in kHz
  uint32_t ch0_frequency_khz;
  /// Channel spacing in kHz
  uint16_t channel_spacing_khz;
  /// Number of channels
  uint16_t number_of_channels;
  /// PHY mode ID
  uint8_t phy_mode_id;
  /// OFDM CRC type (#sl_wisun_crc_type_t)
  uint8_t crc_type;
  /// OQPSK preamble length in bits
  uint8_t preamble_length;
  /// Reserved, set to zero
  uint8_t reserved[1];
} ws_br_agent_phy_config_custom_oqpsk_t;

typedef struct __attribute__((packed, aligned(1))){
  /// Configuration type (#sl_wisun_phy_config_type_t)
  uint32_t type;

  /// Configuration
  union {
    /// Configuration for #SL_WISUN_PHY_CONFIG_FAN10 type
    ws_br_agent_phy_config_fan10_t fan10;
    /// Configuration for #SL_WISUN_PHY_CONFIG_FAN11 type
    ws_br_agent_phy_config_fan11_t fan11;
    /// Configuration for #SL_WISUN_PHY_CONFIG_EXPLICIT type
    ws_br_agent_phy_config_explicit_t explicit_plan;
    /// Configuration for #SL_WISUN_PHY_CONFIG_IDS type
    ws_br_agent_phy_config_ids_t ids;
    /// Configuration for #SL_WISUN_PHY_CONFIG_CUSTOM_FSK type
    ws_br_agent_phy_config_custom_fsk_t custom_fsk;
    /// Configuration for #SL_WISUN_PHY_CONFIG_CUSTOM_OFDM type
    ws_br_agent_phy_config_custom_ofdm_t custom_ofdm;
    /// Configuration for #SL_WISUN_PHY_CONFIG_CUSTOM_OQPSK type
    ws_br_agent_phy_config_custom_oqpsk_t custom_oqpsk;
  } config;
} ws_br_agent_phy_config_t;

/// Enumeration for PHY configuration type
typedef enum ws_br_agent_phy_config_type {
  /// FAN1.0 PHY configuration
  WS_BR_AGENT_PHY_CONFIG_FAN10 = 0,
  /// FAN1.1 PHY configuration
  WS_BR_AGENT_PHY_CONFIG_FAN11,
  /// Explicit PHY configuration
  WS_BR_AGENT_PHY_CONFIG_EXPLICIT,
  /// Explicit RAIL configuration
  WS_BR_AGENT_PHY_CONFIG_IDS,
  /// Custom FSK PHY configuration
  WS_BR_AGENT_PHY_CONFIG_CUSTOM_FSK,
  /// Custom OFDM PHY configuration
  WS_BR_AGENT_PHY_CONFIG_CUSTOM_OFDM,
  /// Custom OQPSK PHY configuration
  WS_BR_AGENT_PHY_CONFIG_CUSTOM_OQPSK,
} ws_br_agent_phy_config_type_t;


typedef enum ws_br_agent_reg_domain{
    // Value of domains is specified by the Wi-SUN specification
    WS_BR_AGENT_REG_DOMAIN_WW =   0x00, // World Wide
    WS_BR_AGENT_REG_DOMAIN_NA =   0x01, // North America
    WS_BR_AGENT_REG_DOMAIN_JP =   0x02, // Japan
    WS_BR_AGENT_REG_DOMAIN_EU =   0x03, // European Union
    WS_BR_AGENT_REG_DOMAIN_CN =   0x04, // China
    WS_BR_AGENT_REG_DOMAIN_IN =   0x05, // India
    WS_BR_AGENT_REG_DOMAIN_MX =   0x06, // Mexico
    WS_BR_AGENT_REG_DOMAIN_BZ =   0x07, // Brazil
    WS_BR_AGENT_REG_DOMAIN_AZ =   0x08, // Australia
    WS_BR_AGENT_REG_DOMAIN_NZ =   0x08, // New zealand
    WS_BR_AGENT_REG_DOMAIN_KR =   0x09, // Korea
    WS_BR_AGENT_REG_DOMAIN_PH =   0x0A, // Philippines
    WS_BR_AGENT_REG_DOMAIN_MY =   0x0B, // Malaysia
    WS_BR_AGENT_REG_DOMAIN_HK =   0x0C, // Hong Kong
    WS_BR_AGENT_REG_DOMAIN_SG =   0x0D, // Singapore
    WS_BR_AGENT_REG_DOMAIN_TH =   0x0E, // Thailand
    WS_BR_AGENT_REG_DOMAIN_VN =   0x0F, // Vietnam
    WS_BR_AGENT_REG_DOMAIN_UNDEF,
} ws_br_agent_reg_domain_t;

/// Network size enum
typedef enum ws_br_agent_network_size {
  /// Small network size
  WS_BR_AGENT_NETWORK_SIZE_SMALL,
  /// Medium network size
  WS_BR_AGENT_NETWORK_SIZE_MEDIUM,
  /// Large network size
  WS_BR_AGENT_NETWORK_SIZE_LARGE,
  /// Extra large network size
  WS_BR_AGENT_NETWORK_SIZE_XLARGE,
  /// Certification network size
  WS_BR_AGENT_NETWORK_SIZE_CERTIFICATION,
} ws_br_agent_network_size_t;

/// Enumeration for keychain
typedef enum ws_br_agent_keychain {
  /// Automatic keychain selection. If SL_WISUN_KEYCHAIN_NVM does not have
  /// any credentials, SL_WISUN_KEYCHAIN_BUILTIN is used instead.
  WS_BR_AGENT_KEYCHAIN_AUTOMATIC,
  /// Built-in keychain
  WS_BR_AGENT_KEYCHAIN_BUILTIN,
  /// NVM keychain
  WS_BR_AGENT_KEYCHAIN_NVM
} ws_br_agent_keychain_t;

/// Structure for Wi-SUN Border Router settings
typedef struct __attribute__((packed, aligned(1))) ws_br_agent_settings {
  /// Network name (null-terminated string)
  char network_name[WS_BR_AGENT_NETWORK_NAME_SIZE + 1];
  /// Network size
  uint8_t network_size;
  /// TX Power
  int16_t tx_power_ddbm;
  /// UC Dwell interval in ms
  uint8_t uc_dwell_interval_ms;
  /// BC interval in ms
  uint32_t bc_interval_ms;
  /// BC Dwell interval in ms
  uint8_t bc_dwell_interval_ms;
  /// State
  uint8_t state;
  /// Allowed channels
  char allowed_channels[WS_BR_AGENT_SETTINGS_STR_DATA_MAX_LENGTH + 1];
  /// IPv6 prefix
  char ipv6_prefix[WS_BR_AGENT_IPV6_PREFIX_SIZE + 1];
  /// Regulation
  uint8_t regulation;
  /// FEC
  uint8_t fec;
  /// RX PHY mode IDs
  uint8_t rx_phy_mode_ids[WS_BR_AGENT_MAX_PHY_MODE_ID_COUNT];
  /// RX PHY mode IDs count
  uint8_t rx_phy_mode_ids_count;
  /// LFN profile
  uint8_t lfn_profile;
  /// Maximum neighbor count
  uint8_t max_neighbor_count;
  /// Maximum child count
  uint8_t max_child_count;
  /// Maximum security neighbor count
  uint16_t max_security_neighbor_count;
  /// Key chain
  uint8_t keychain;
  /// Key chain index
  uint8_t keychain_index;
  /// Socket RX buffer size
  uint16_t socket_rx_buffer_size;
  /// PHY configuration type
  ws_br_agent_phy_config_t phy;
  /// Default PHY
  bool is_default_phy;
  /// PAN ID
  uint16_t pan_id;
} ws_br_agent_settings_t;

/**
 * @brief Load configuration from a file.
 * @param[in] conf_file Path to the configuration file.
 * @param[out] settings Pointer to settings structure to be filled.
 * @return WS_BR_AGENT_RET_OK on success, error code otherwise.
 */
ws_br_agent_ret_t ws_br_agent_settings_load_config(const char * conf_file, 
                                                   ws_br_agent_settings_t *settings);

#ifdef __cplusplus
}
#endif

#endif // WS_BR_AGENT_SETTINGS_H