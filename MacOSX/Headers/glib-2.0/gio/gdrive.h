/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2006-2007 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Alexander Larsson <alexl@redhat.com>
 *         David Zeuthen <davidz@redhat.com>
 */

#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_DRIVE_H__
#define __G_DRIVE_H__

#include <gio/giotypes.h>

G_BEGIN_DECLS

#define G_TYPE_DRIVE           (g_drive_get_type ())
#define G_DRIVE(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_DRIVE, GDrive))
#define G_IS_DRIVE(obj)	       (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_DRIVE))
#define G_DRIVE_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), G_TYPE_DRIVE, GDriveIface))

/**
 * GDriveIface:
 * @g_iface: The parent interface.
 * @changed: Signal emitted when the drive is changed.
 * @disconnected: The removed signal that is emitted when the #GDrive have been disconnected. If the recipient is holding references to the object they should release them so the object can be finalized.
 * @eject_button: Signal emitted when the physical eject button (if any) of a drive have been pressed.
 * @get_name: Returns the name for the given #GDrive.
 * @get_icon: Returns a #GIcon for the given #GDrive.
 * @has_volumes: Returns %TRUE if the #GDrive has mountable volumes.
 * @get_volumes: Returns a list #GList of #GVolume for the #GDrive.
 * @is_media_removable: Returns %TRUE if the #GDrive supports removal and insertion of media.
 * @has_media: Returns %TRUE if the #GDrive has media inserted.
 * @is_media_check_automatic: Returns %TRUE if the #GDrive is capabable of automatically detecting media changes.
 * @can_poll_for_media: Returns %TRUE if the #GDrive is capable of manually polling for media change.
 * @can_eject: Returns %TRUE if the #GDrive can eject media.
 * @eject: Ejects a #GDrive.
 * @eject_finish: Finishes an eject operation.
 * @poll_for_media: Poll for media insertion/removal on a #GDrive.
 * @poll_for_media_finish: Finishes a media poll operation.
 * @get_identifier: Returns the identifier of the given kind, or %NULL if
 *    the #GDrive doesn't have one.
 * @enumerate_identifiers: Returns an array strings listing the kinds
 *    of identifiers which the #GDrive has.
 *
 * Interface for creating #GDrive implementations.
 */
typedef struct _GDriveIface    GDriveIface;

struct _GDriveIface
{
  GTypeInterface g_iface;

  /* signals */
  void     (* changed)                  (GDrive              *drive);
  void     (* disconnected)             (GDrive              *drive);
  void     (* eject_button)             (GDrive              *drive);

  /* Virtual Table */
  char *   (* get_name)                 (GDrive              *drive);
  GIcon *  (* get_icon)                 (GDrive              *drive);
  gboolean (* has_volumes)              (GDrive              *drive);
  GList *  (* get_volumes)              (GDrive              *drive);
  gboolean (* is_media_removable)       (GDrive              *drive);
  gboolean (* has_media)                (GDrive              *drive);
  gboolean (* is_media_check_automatic) (GDrive              *drive);
  gboolean (* can_eject)                (GDrive              *drive);
  gboolean (* can_poll_for_media)       (GDrive              *drive);
  void     (* eject)                    (GDrive              *drive,
                                         GMountUnmountFlags   flags,
                                         GCancellable        *cancellable,
                                         GAsyncReadyCallback  callback,
                                         gpointer             user_data);
  gboolean (* eject_finish)             (GDrive              *drive,
                                         GAsyncResult        *result,
                                         GError             **error);
  void     (* poll_for_media)           (GDrive              *drive,
                                         GCancellable        *cancellable,
                                         GAsyncReadyCallback  callback,
                                         gpointer             user_data);
  gboolean (* poll_for_media_finish)    (GDrive              *drive,
                                         GAsyncResult        *result,
                                         GError             **error);

  char *   (* get_identifier)           (GDrive              *drive,
                                         const char          *kind);
  char **  (* enumerate_identifiers)    (GDrive              *drive);
};

GType    g_drive_get_type                 (void) G_GNUC_CONST;

char *   g_drive_get_name                 (GDrive               *drive);
GIcon *  g_drive_get_icon                 (GDrive               *drive);
gboolean g_drive_has_volumes              (GDrive               *drive);
GList *  g_drive_get_volumes              (GDrive               *drive);
gboolean g_drive_is_media_removable       (GDrive               *drive);
gboolean g_drive_has_media                (GDrive               *drive);
gboolean g_drive_is_media_check_automatic (GDrive               *drive);
gboolean g_drive_can_poll_for_media       (GDrive               *drive);
gboolean g_drive_can_eject                (GDrive               *drive);
void     g_drive_eject                    (GDrive               *drive,
					   GMountUnmountFlags    flags,
                                           GCancellable         *cancellable,
                                           GAsyncReadyCallback   callback,
                                           gpointer              user_data);
gboolean g_drive_eject_finish             (GDrive               *drive,
                                           GAsyncResult         *result,
                                           GError              **error);
void     g_drive_poll_for_media           (GDrive               *drive,
                                           GCancellable         *cancellable,
                                           GAsyncReadyCallback   callback,
                                           gpointer              user_data);
gboolean g_drive_poll_for_media_finish    (GDrive               *drive,
                                           GAsyncResult         *result,
                                           GError              **error);
char *   g_drive_get_identifier           (GDrive              *drive,
					   const char          *kind);
char **  g_drive_enumerate_identifiers    (GDrive              *drive);

G_END_DECLS

#endif /* __G_DRIVE_H__ */
