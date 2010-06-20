#ifndef __G_NATIVE_VOLUME_MONITOR_H__
#define __G_NATIVE_VOLUME_MONITOR_H__

#include <gio/gvolumemonitor.h>

G_BEGIN_DECLS

#define G_TYPE_NATIVE_VOLUME_MONITOR        (g_native_volume_monitor_get_type ())
#define G_NATIVE_VOLUME_MONITOR(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_NATIVE_VOLUME_MONITOR, GNativeVolumeMonitor))
#define G_NATIVE_VOLUME_MONITOR_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_NATIVE_VOLUME_MONITOR, GNativeVolumeMonitorClass))
#define G_IS_NATIVE_VOLUME_MONITOR(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_NATIVE_VOLUME_MONITOR))
#define G_IS_NATIVE_VOLUME_MONITOR_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_NATIVE_VOLUME_MONITOR))

#define G_NATIVE_VOLUME_MONITOR_EXTENSION_POINT_NAME "gio-native-volume-monitor"

typedef struct _GNativeVolumeMonitor      GNativeVolumeMonitor;
typedef struct _GNativeVolumeMonitorClass GNativeVolumeMonitorClass;

struct _GNativeVolumeMonitor
{
  GVolumeMonitor parent_instance;
};

struct _GNativeVolumeMonitorClass
{
  GVolumeMonitorClass parent_class;

  GMount * (* get_mount_for_mount_path) (const char   *mount_path,
                                         GCancellable *cancellable);
};

GType g_native_volume_monitor_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __G_NATIVE_VOLUME_MONITOR_H__ */
