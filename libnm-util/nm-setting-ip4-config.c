/* -*- Mode: C; tab-width: 5; indent-tabs-mode: t; c-basic-offset: 5 -*- */

#include <dbus/dbus-glib.h>
#include "nm-setting-ip4-config.h"
#include "nm-param-spec-specialized.h"
#include "nm-utils.h"

G_DEFINE_TYPE (NMSettingIP4Config, nm_setting_ip4_config, NM_TYPE_SETTING)

enum {
	PROP_0,
	PROP_MANUAL,
	PROP_DNS,
	PROP_DNS_SEARCH,
	PROP_ADDRESSES,

	LAST_PROP
};

NMSetting *
nm_setting_ip4_config_new (void)
{
	return (NMSetting *) g_object_new (NM_TYPE_SETTING_IP4_CONFIG, NULL);
}

static gboolean
verify (NMSetting *setting, GSList *all_settings)
{
	NMSettingIP4Config *self = NM_SETTING_IP4_CONFIG (setting);

	if (self->manual) {
		if (!self->addresses) {
			g_warning ("address is not provided");
			return FALSE;
		}
	}

	return TRUE;
}


static void
nm_setting_ip4_config_init (NMSettingIP4Config *setting)
{
	((NMSetting *) setting)->name = g_strdup (NM_SETTING_IP4_CONFIG_SETTING_NAME);
}

static void
finalize (GObject *object)
{
	NMSettingIP4Config *self = NM_SETTING_IP4_CONFIG (object);

	if (self->dns)
		g_array_free (self->dns, TRUE);

	nm_utils_slist_free (self->dns_search, g_free);
	nm_utils_slist_free (self->addresses, g_free);

	G_OBJECT_CLASS (nm_setting_ip4_config_parent_class)->finalize (object);
}

static GSList *
ip4_addresses_from_gvalue (const GValue *value)
{
	GPtrArray *ptr_array;
	int i;
	GSList *list = NULL;

	ptr_array = (GPtrArray *) g_value_get_boxed (value);
	for (i = 0; i < ptr_array->len; i++) {
		GValueArray *value_array = (GValueArray *) g_ptr_array_index (ptr_array, i);

		if (value_array->n_values == 2 || value_array->n_values == 3) {
			NMSettingIP4Address *ip4_addr;

			ip4_addr = g_new0 (NMSettingIP4Address, 1);
			ip4_addr->address = g_value_get_uint (g_value_array_get_nth (value_array, 0));
			ip4_addr->netmask = g_value_get_uint (g_value_array_get_nth (value_array, 1));

			if (value_array->n_values == 3)
				ip4_addr->gateway = g_value_get_uint (g_value_array_get_nth (value_array, 2));

			list = g_slist_prepend (list, ip4_addr);
		} else
			nm_warning ("Ignoring invalid IP4 address");
	}

	return g_slist_reverse (list);
}

static void
ip4_addresses_to_gvalue (GSList *list, GValue *value)
{
	GPtrArray *ptr_array;
	GSList *iter;

	ptr_array = g_ptr_array_new ();

	for (iter = list; iter; iter = iter->next) {
		NMSettingIP4Address *ip4_addr = (NMSettingIP4Address *) iter->data;
		GArray *array;

		array = g_array_sized_new (FALSE, FALSE, sizeof (guint32), 3);
		g_array_append_val (array, ip4_addr->address);
		g_array_append_val (array, ip4_addr->netmask);

		if (ip4_addr->gateway)
			g_array_append_val (array, ip4_addr->gateway);

		g_ptr_array_add (ptr_array, array);
	}

	g_value_take_boxed (value, ptr_array);
}

static void
set_property (GObject *object, guint prop_id,
		    const GValue *value, GParamSpec *pspec)
{
	NMSettingIP4Config *setting = NM_SETTING_IP4_CONFIG (object);

	switch (prop_id) {
	case PROP_MANUAL:
		setting->manual = g_value_get_boolean (value);
		break;
	case PROP_DNS:
		if (setting->dns)
			g_array_free (setting->dns, TRUE);
		setting->dns = g_value_dup_boxed (value);
		break;
	case PROP_DNS_SEARCH:
		nm_utils_slist_free (setting->dns_search, g_free);
		setting->dns_search = g_value_dup_boxed (value);
		break;
	case PROP_ADDRESSES:
		nm_utils_slist_free (setting->addresses, g_free);
		setting->addresses = ip4_addresses_from_gvalue (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
get_property (GObject *object, guint prop_id,
		    GValue *value, GParamSpec *pspec)
{
	NMSettingIP4Config *setting = NM_SETTING_IP4_CONFIG (object);

	switch (prop_id) {
	case PROP_MANUAL:
		g_value_set_boolean (value, setting->manual);
		break;
	case PROP_DNS:
		g_value_set_boxed (value, setting->dns);
		break;
	case PROP_DNS_SEARCH:
		g_value_set_boxed (value, setting->dns_search);
		break;
	case PROP_ADDRESSES:
		ip4_addresses_to_gvalue (setting->addresses, value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
nm_setting_ip4_config_class_init (NMSettingIP4ConfigClass *setting_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (setting_class);
	NMSettingClass *parent_class = NM_SETTING_CLASS (setting_class);

	/* virtual methods */
	object_class->set_property = set_property;
	object_class->get_property = get_property;
	object_class->finalize     = finalize;
	parent_class->verify       = verify;

	/* Properties */
	g_object_class_install_property
		(object_class, PROP_MANUAL,
		 g_param_spec_boolean (NM_SETTING_IP4_CONFIG_MANUAL,
						   "Manual",
						   "Do not use DHCP",
						   FALSE,
						   G_PARAM_READWRITE | NM_SETTING_PARAM_SERIALIZE));

	g_object_class_install_property
		(object_class, PROP_DNS,
		 nm_param_spec_specialized (NM_SETTING_IP4_CONFIG_DNS,
							   "DNS",
							   "List of DNS servers",
							   DBUS_TYPE_G_UINT_ARRAY,
							   G_PARAM_READWRITE | NM_SETTING_PARAM_SERIALIZE));

	g_object_class_install_property
		(object_class, PROP_DNS_SEARCH,
		 nm_param_spec_specialized (NM_SETTING_IP4_CONFIG_DNS_SEARCH,
							   "DNS search",
							   "List of DNS search domains",
							   dbus_g_type_get_collection ("GSList", G_TYPE_STRING),
							   G_PARAM_READWRITE | NM_SETTING_PARAM_SERIALIZE));

	g_object_class_install_property
		(object_class, PROP_ADDRESSES,
		 nm_param_spec_specialized (NM_SETTING_IP4_CONFIG_ADDRESSES,
							   "Addresses",
							   "List of NMSettingIP4Addresses",
							   dbus_g_type_get_collection ("GPtrArray", G_TYPE_VALUE_ARRAY),
							   G_PARAM_READWRITE | NM_SETTING_PARAM_SERIALIZE));
}