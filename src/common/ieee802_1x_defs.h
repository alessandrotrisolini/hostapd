/*
 * IEEE Std 802.1X-2010 definitions
 * Copyright (c) 2013-2014, Qualcomm Atheros, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef IEEE802_1X_DEFS_H
#define IEEE802_1X_DEFS_H

#include "utils/list.h"

#define CS_ID_LEN		8
#define CS_ID_GCM_AES_128	0x0080020001000001ULL
#define CS_NAME_GCM_AES_128	"GCM-AES-128"

enum macsec_policy {
	/**
	 * Should secure sessions.
	 * This accepts key server's advice to determine whether to secure the
	 * session or not.
	 */
	SHOULD_SECURE,

	/**
	 * Disabled MACsec - do not secure sessions.
	 */
	DO_NOT_SECURE,

	/**
	 * Should secure sessions, and try to use encryption.
	 * Like @SHOULD_SECURE, this follows the key server's decision.
	 */
	SHOULD_ENCRYPT,
};


/* IEEE Std 802.1X-2010 - Table 11-6 - MACsec Capability */
enum macsec_cap {
	/**
	 * MACsec is not implemented
	 */
	MACSEC_CAP_NOT_IMPLEMENTED,

	/**
	 * 'Integrity without confidentiality'
	 */
	MACSEC_CAP_INTEGRITY,

	/**
	 * 'Integrity without confidentiality' and
	 * 'Integrity and confidentiality' with a confidentiality offset of 0
	 */
	MACSEC_CAP_INTEG_AND_CONF,

	/**
	 * 'Integrity without confidentiality' and
	 * 'Integrity and confidentiality' with a confidentiality offset of 0,
	 * 30, 50
	 */
	MACSEC_CAP_INTEG_AND_CONF_0_30_50,
};

enum validate_frames {
	Disabled,
	Checked,
	Strict,
};

/* IEEE Std 802.1X-2010 - Table 11-6 - Confidentiality Offset */
enum confidentiality_offset {
	CONFIDENTIALITY_NONE      = 0,
	CONFIDENTIALITY_OFFSET_0  = 1,
	CONFIDENTIALITY_OFFSET_30 = 2,
	CONFIDENTIALITY_OFFSET_50 = 3,
};

/* IEEE Std 802.1X-2010 - Table 9-2 */
#define DEFAULT_PRIO_INFRA_PORT        0x10
#define DEFAULT_PRIO_PRIMRAY_AP        0x30
#define DEFAULT_PRIO_SECONDARY_AP      0x50
#define DEFAULT_PRIO_GROUP_CA_MEMBER   0x70
#define DEFAULT_PRIO_NOT_KEY_SERVER    0xFF

struct ieee802_1x_mka_sci {
	u8 addr[ETH_ALEN];
	be16 port;
};

/* TransmitSC in IEEE Std 802.1AE-2006, Figure 10-6 */
struct transmit_sc {
	struct ieee802_1x_mka_sci sci; /* const SCI sci */
	Boolean transmitting; /* bool transmitting (read only) */

	struct os_time created_time; /* Time createdTime */

	u8 encoding_sa; /* AN encodingSA (read only) */
	u8 enciphering_sa; /* AN encipheringSA (read only) */

	struct dl_list list;
	struct dl_list sa_list;
};

/* TransmitSA in IEEE Std 802.1AE-2006, Figure 10-6 */
struct transmit_sa {
	Boolean in_use; /* bool inUse (read only) */
	u32 next_pn; /* PN nextPN (read only) */
	struct os_time created_time; /* Time createdTime */

	Boolean enable_transmit; /* bool EnableTransmit */

	u8 an;
	Boolean confidentiality;
	struct data_key *pkey;

	struct transmit_sc *sc;
	struct dl_list list; /* list entry in struct transmit_sc::sa_list */
};

static inline u64 mka_sci_u64(struct ieee802_1x_mka_sci *sci)
{
	struct ieee802_1x_mka_sci tmp;

	memcpy(tmp.addr, sci->addr, ETH_ALEN);
	tmp.port = sci->port;

	return *((u64 *)&tmp);
}

/* ReceiveSC in IEEE Std 802.1AE-2006, Figure 10-6 */
struct receive_sc {
	struct ieee802_1x_mka_sci sci; /* const SCI sci */
	Boolean receiving; /* bool receiving (read only) */

	struct os_time created_time; /* Time createdTime */

	struct dl_list list;
	struct dl_list sa_list;
};

/* ReceiveSA in IEEE Std 802.1AE-2006, Figure 10-6 */
struct receive_sa {
	Boolean enable_receive; /* bool enableReceive */
	Boolean in_use; /* bool inUse (read only) */

	u32 next_pn; /* PN nextPN (read only) */
	u32 lowest_pn; /* PN lowestPN (read only) */
	u8 an;
	struct os_time created_time;

	struct data_key *pkey;
	struct receive_sc *sc; /* list entry in struct receive_sc::sa_list */

	struct dl_list list;
};

#endif /* IEEE802_1X_DEFS_H */
