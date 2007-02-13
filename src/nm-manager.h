#ifndef NM_MANAGER_H
#define NM_MANAGER_H 1

#include <glib/gtypes.h>
#include <glib-object.h>
#include <dbus/dbus-glib.h>
#include "nm-device.h"

#define NM_TYPE_MANAGER            (nm_manager_get_type ())
#define NM_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NM_TYPE_MANAGER, NMManager))
#define NM_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), NM_TYPE_MANAGER, NMManagerClass))
#define NM_IS_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NM_TYPE_MANAGER))
#define NM_IS_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), NM_TYPE_MANAGER))
#define NM_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), NM_TYPE_MANAGER, NMManagerClass))

#define NM_MANAGER_STATE "state"
#define NM_MANAGER_WIRELESS_ENABLED "wireless-enabled"

typedef struct {
	GObject parent;
} NMManager;

typedef struct {
	GObjectClass parent;

	/* Signals */
	void (*device_added) (NMManager *manager, NMDevice *device);
	void (*device_removed) (NMManager *manager, NMDevice *device);
	void (*state_change) (NMManager *manager, guint state);

} NMManagerClass;

GType nm_manager_get_type (void);

NMManager *nm_manager_new (void);

void nm_manager_add_device (NMManager *manager, NMDevice *device);
void nm_manager_remove_device (NMManager *manager, NMDevice *device);
GSList *nm_manager_get_devices (NMManager *manager);
NMDevice *nm_manager_get_device_by_iface (NMManager *manager, const char *iface);
NMDevice *nm_manager_get_device_by_udi (NMManager *manager, const char *udi);
NMState nm_manager_get_state (NMManager *manager);
gboolean nm_manager_wireless_enabled (NMManager *manager);
void nm_manager_sleep (NMManager *manager, gboolean sleep);

NMDevice *nm_manager_get_active_device (NMManager *manager);

#endif /* NM_MANAGER_H */