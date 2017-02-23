/*
 *
 * IEEE 802.1X-2010 KaY interface for hostapd
 *
 */

#include "utils/includes.h"
#include "utils/common.h"
#include "utils/ovs_utils.h"

#include <ifaddrs.h>
#include <net/if.h>

#include "pae/ieee802_1x_key.h"
#include "pae/ieee802_1x_kay.h"

#include "hostapd.h"
#include "hapd_kay.h"

#define CAK_LEN 16
#define CKN_LEN 16

static int hapd_macsec_init(void *priv, struct macsec_init_params *params)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->macsec_init(hapd->drv_priv, params);
}

static int hapd_macsec_deinit(void *priv)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->macsec_deinit(hapd->drv_priv);
}

static int hapd_macsec_get_capability(void *priv, enum macsec_cap *cap)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->macsec_get_capability(hapd->drv_priv, cap); 
}

static int hapd_enable_protect_frames(void *priv, Boolean enabled)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->enable_protect_frames(hapd->drv_priv, enabled);
}

static int hapd_enable_encrypt(void *priv, Boolean enabled)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->enable_encrypt(hapd->drv_priv, enabled);
}

static int hapd_set_replay_protect(void *priv, Boolean enabled, u32 window)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->set_replay_protect(hapd->drv_priv, enabled, 
			window);
}

static int hapd_set_current_cipher_suite(void *priv, u64 cs)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->set_current_cipher_suite(hapd->drv_priv, cs);
}

static int hapd_enable_controlled_port(void *priv, Boolean enabled)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->enable_controlled_port(hapd->drv_priv, enabled);
}

static int hapd_get_receive_lowest_pn(void *priv, struct receive_sa *sa)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->get_receive_lowest_pn(hapd->drv_priv, sa);
}

static int hapd_get_transmit_next_pn(void *priv, struct transmit_sa *sa)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->get_transmit_next_pn(hapd->drv_priv, sa);
}

static int hapd_set_transmit_next_pn(void *priv, struct transmit_sa *sa)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->set_transmit_next_pn(hapd->drv_priv, sa);
}

static unsigned int conf_offset_val(enum confidentiality_offset co)
{
	switch (co) {
	case CONFIDENTIALITY_OFFSET_30:
		return 30;
		break;
	case CONFIDENTIALITY_OFFSET_50:
		return 50;
		break;
	default:
		return 0;
	}
}

static int hapd_create_receive_sc(void *priv, struct receive_sc *sc,
		struct ieee802_1x_mka_sci *sci,
		enum validate_frames vf,
		enum confidentiality_offset co)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->create_receive_sc(hapd->drv_priv, sc,
		       			sci->addr,
					be_to_host16(sci->port),
					conf_offset_val(co), vf);	
}

static int hapd_delete_receive_sc(void *priv, struct receive_sc *sc)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->delete_receive_sc(hapd->drv_priv, sc);
}

static int hapd_create_receive_sa(void *priv, struct receive_sa *sa)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->create_receive_sa(hapd->drv_priv, sa);
}

static int hapd_delete_receive_sa(void *priv, struct receive_sa *sa)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->delete_receive_sa(hapd->drv_priv, sa);
}

static int hapd_enable_receive_sa(void *priv, struct receive_sa *sa)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->enable_receive_sa(hapd->drv_priv, sa);
}


static int hapd_disable_receive_sa(void *priv, struct receive_sa *sa)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->disable_receive_sa(hapd->drv_priv, sa);
}

static int hapd_create_transmit_sc(void *priv, struct transmit_sc *sc,
		enum confidentiality_offset co)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->create_transmit_sc(hapd->drv_priv, sc,
					conf_offset_val(co));	
}

static int hapd_delete_transmit_sc(void *priv, struct transmit_sc *sc)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->delete_transmit_sc(hapd->drv_priv, sc);
}

static int hapd_create_transmit_sa(void *priv, struct transmit_sa *sa)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->create_transmit_sa(hapd->drv_priv, sa);
}

static int hapd_delete_transmit_sa(void *priv, struct transmit_sa *sa)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->delete_transmit_sa(hapd->drv_priv, sa);
}

static int hapd_enable_transmit_sa(void *priv, struct transmit_sa *sa)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->enable_transmit_sa(hapd->drv_priv, sa);
}


static int hapd_disable_transmit_sa(void *priv, struct transmit_sa *sa)
{
	struct hostapd_data *hapd = priv;

	return hapd->driver->disable_transmit_sa(hapd->drv_priv, sa);
}

static const struct ieee802_1x_kay_ctx kay_ops = {
	.macsec_init			= hapd_macsec_init,
	.macsec_deinit			= hapd_macsec_deinit,
	.macsec_get_capability		= hapd_macsec_get_capability,
	.enable_protect_frames		= hapd_enable_protect_frames,
	.enable_encrypt			= hapd_enable_encrypt,
	.set_replay_protect		= hapd_set_replay_protect,
	.set_current_cipher_suite	= hapd_set_current_cipher_suite,
	.enable_controlled_port		= hapd_enable_controlled_port,
	.get_receive_lowest_pn		= hapd_get_receive_lowest_pn,
	.get_transmit_next_pn		= hapd_get_transmit_next_pn,
	.set_transmit_next_pn		= hapd_set_transmit_next_pn,
	
	.create_receive_sc		= hapd_create_receive_sc,
	.delete_receive_sc		= hapd_delete_receive_sc,
	.create_receive_sa		= hapd_create_receive_sa,
	.delete_receive_sa		= hapd_delete_receive_sa,
	.enable_receive_sa		= hapd_enable_receive_sa,
	.disable_receive_sa		= hapd_disable_receive_sa,
	
	.create_transmit_sc		= hapd_create_transmit_sc,
	.delete_transmit_sc		= hapd_delete_transmit_sc,
	.create_transmit_sa		= hapd_create_transmit_sa,
	.delete_transmit_sa		= hapd_delete_transmit_sa,
	.enable_transmit_sa		= hapd_enable_transmit_sa,
	.disable_transmit_sa		= hapd_disable_transmit_sa,
};

void * ieee802_1x_create_hapd_mka(struct hostapd_data *hapd, u8 *cak, u8 *ckn)
{

	struct ieee802_1x_kay_ctx *kay_ctx;
	struct ieee802_1x_kay *kay = NULL;
	void *res  = NULL;

	struct mka_key *mka_cak;
	struct mka_key_name *mka_ckn;

	kay_ctx = os_zalloc(sizeof(*kay_ctx));

	os_memcpy(kay_ctx, &kay_ops, sizeof(*kay_ctx));
	kay_ctx->ctx = hapd;

	kay = ieee802_1x_kay_init(kay_ctx, SHOULD_ENCRYPT, 1,
		       hapd->conf->iface, hapd->own_addr);
	

	if(kay == NULL){
		wpa_printf(MSG_ERROR, "Could not allocate authenticator KaY");
		os_free(kay_ctx);
		return NULL;
	}	

	hapd->kay = kay;

	mka_cak = os_zalloc(sizeof(*mka_cak));
	mka_ckn = os_zalloc(sizeof(*mka_ckn));
	
	mka_cak->len = CAK_LEN;
	os_memcpy(mka_cak->key, cak, mka_cak->len);

	mka_ckn->len = CKN_LEN;
	os_memcpy(mka_ckn->name, ckn, mka_ckn->len);

	res = ieee802_1x_kay_create_mka(kay, mka_ckn, mka_cak, 0, EAP_EXCHANGE, TRUE);

	if(res == NULL) {
		wpa_printf(MSG_ERROR, "Could not create MKA");
		os_free(kay_ctx);
		os_free(mka_cak);
		os_free(mka_ckn);
	} else {

		/*
		 * Find the latest netdevice created (i.e. the one with the highest if_index) 
		 * and attach it to OvS
		 */

		struct ifaddrs *a, *t;
		int idx = 0;
		int curr_idx = 0;

		
		/* Return a linked list of all the interfaces present on this machine */
		getifaddrs(&a);

		/* Find the max if_index and the relative if_name */
		for (t=a; t!=NULL; t=t->ifa_next) {
			curr_idx = if_nametoindex(t->ifa_name);
			if(curr_idx > idx)
				idx = curr_idx;
		}

		/* Assing if_name and if_index to hostapd_data */
		hapd->macsec_ifname = os_zalloc(IF_NAMESIZE);
		if_indextoname(idx, hapd->macsec_ifname);
		hapd->macsec_ifindex = idx;

		/* Create OvS port */
		wpa_printf(MSG_DEBUG, "Adding %s to OvS...", hapd->macsec_ifname);
		ovs_add_port(hapd->ovs_br_name, hapd->macsec_ifname);

		freeifaddrs(a);
	}

	return res;	
}

void ieee802_1x_dealloc_hapd_mka(struct hostapd_data *hapd)
{
	ieee802_1x_kay_deinit(hapd->kay);
	hapd->kay = NULL;

	ovs_del_port(hapd->ovs_br_name, hapd->macsec_ifname);

	os_free(hapd->macsec_ifname);
	
}
