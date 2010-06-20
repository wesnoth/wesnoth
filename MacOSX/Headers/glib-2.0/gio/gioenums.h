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
 */

#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __GIO_ENUMS_H__
#define __GIO_ENUMS_H__

#include <glib-object.h>

G_BEGIN_DECLS


/**
 * GAppInfoCreateFlags:
 * @G_APP_INFO_CREATE_NONE: No flags.
 * @G_APP_INFO_CREATE_NEEDS_TERMINAL: Application opens in a terminal window.
 * @G_APP_INFO_CREATE_SUPPORTS_URIS: Application supports URI arguments.
 *
 * Flags used when creating a #GAppInfo.
 */
typedef enum {
  G_APP_INFO_CREATE_NONE           = 0,         /*< nick=none >*/
  G_APP_INFO_CREATE_NEEDS_TERMINAL = (1 << 0),  /*< nick=needs-terminal >*/
  G_APP_INFO_CREATE_SUPPORTS_URIS  = (1 << 1)   /*< nick=supports-uris >*/
} GAppInfoCreateFlags;


/**
 * GDataStreamByteOrder:
 * @G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN: Selects Big Endian byte order.
 * @G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN: Selects Little Endian byte order.
 * @G_DATA_STREAM_BYTE_ORDER_HOST_ENDIAN: Selects endianness based on host machine's architecture.
 *
 * #GDataStreamByteOrder is used to ensure proper endianness of streaming data sources
 * across various machine architectures.
 *
 **/
typedef enum {
  G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN,
  G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN,
  G_DATA_STREAM_BYTE_ORDER_HOST_ENDIAN
} GDataStreamByteOrder;


/**
 * GDataStreamNewlineType:
 * @G_DATA_STREAM_NEWLINE_TYPE_LF: Selects "LF" line endings, common on most modern UNIX platforms.
 * @G_DATA_STREAM_NEWLINE_TYPE_CR: Selects "CR" line endings.
 * @G_DATA_STREAM_NEWLINE_TYPE_CR_LF: Selects "CR, LF" line ending, common on Microsoft Windows.
 * @G_DATA_STREAM_NEWLINE_TYPE_ANY: Automatically try to handle any line ending type.
 *
 * #GDataStreamNewlineType is used when checking for or setting the line endings for a given file.
 **/
typedef enum {
  G_DATA_STREAM_NEWLINE_TYPE_LF,
  G_DATA_STREAM_NEWLINE_TYPE_CR,
  G_DATA_STREAM_NEWLINE_TYPE_CR_LF,
  G_DATA_STREAM_NEWLINE_TYPE_ANY
} GDataStreamNewlineType;


/**
 * GFileAttributeType:
 * @G_FILE_ATTRIBUTE_TYPE_INVALID: indicates an invalid or uninitalized type.
 * @G_FILE_ATTRIBUTE_TYPE_STRING: a null terminated UTF8 string.
 * @G_FILE_ATTRIBUTE_TYPE_BYTE_STRING: a zero terminated string of non-zero bytes.
 * @G_FILE_ATTRIBUTE_TYPE_BOOLEAN: a boolean value.
 * @G_FILE_ATTRIBUTE_TYPE_UINT32: an unsigned 4-byte/32-bit integer.
 * @G_FILE_ATTRIBUTE_TYPE_INT32: a signed 4-byte/32-bit integer.
 * @G_FILE_ATTRIBUTE_TYPE_UINT64: an unsigned 8-byte/64-bit integer.
 * @G_FILE_ATTRIBUTE_TYPE_INT64: a signed 8-byte/64-bit integer.
 * @G_FILE_ATTRIBUTE_TYPE_OBJECT: a #GObject.
 *
 * The data types for file attributes.
 **/
typedef enum {
  G_FILE_ATTRIBUTE_TYPE_INVALID = 0,
  G_FILE_ATTRIBUTE_TYPE_STRING,
  G_FILE_ATTRIBUTE_TYPE_BYTE_STRING, /* zero terminated string of non-zero bytes */
  G_FILE_ATTRIBUTE_TYPE_BOOLEAN,
  G_FILE_ATTRIBUTE_TYPE_UINT32,
  G_FILE_ATTRIBUTE_TYPE_INT32,
  G_FILE_ATTRIBUTE_TYPE_UINT64,
  G_FILE_ATTRIBUTE_TYPE_INT64,
  G_FILE_ATTRIBUTE_TYPE_OBJECT
} GFileAttributeType;


/**
 * GFileAttributeInfoFlags:
 * @G_FILE_ATTRIBUTE_INFO_NONE: no flags set.
 * @G_FILE_ATTRIBUTE_INFO_COPY_WITH_FILE: copy the attribute values when the file is copied.
 * @G_FILE_ATTRIBUTE_INFO_COPY_WHEN_MOVED: copy the attribute values when the file is moved.
 *
 * Flags specifying the behaviour of an attribute.
 **/
typedef enum {
  G_FILE_ATTRIBUTE_INFO_NONE            = 0,
  G_FILE_ATTRIBUTE_INFO_COPY_WITH_FILE  = (1 << 0),
  G_FILE_ATTRIBUTE_INFO_COPY_WHEN_MOVED = (1 << 1)
} GFileAttributeInfoFlags;


/**
 * GFileAttributeStatus:
 * @G_FILE_ATTRIBUTE_STATUS_UNSET: Attribute value is unset (empty).
 * @G_FILE_ATTRIBUTE_STATUS_SET: Attribute value is set.
 * @G_FILE_ATTRIBUTE_STATUS_ERROR_SETTING: Indicates an error in setting the value.
 *
 * Used by g_file_set_attributes_from_info() when setting file attributes.
 **/
typedef enum {
  G_FILE_ATTRIBUTE_STATUS_UNSET = 0,
  G_FILE_ATTRIBUTE_STATUS_SET,
  G_FILE_ATTRIBUTE_STATUS_ERROR_SETTING
} GFileAttributeStatus;


/**
 * GFileQueryInfoFlags:
 * @G_FILE_QUERY_INFO_NONE: No flags set.
 * @G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS: Don't follow symlinks.
 *
 * Flags used when querying a #GFileInfo.
 */
typedef enum {
  G_FILE_QUERY_INFO_NONE              = 0,
  G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS = (1 << 0)   /*< nick=nofollow-symlinks >*/
} GFileQueryInfoFlags;


/**
 * GFileCreateFlags:
 * @G_FILE_CREATE_NONE: No flags set.
 * @G_FILE_CREATE_PRIVATE: Create a file that can only be
 *    accessed by the current user.
 *
 * Flags used when an operation may create a file.
 */
typedef enum {
  G_FILE_CREATE_NONE    = 0,
  G_FILE_CREATE_PRIVATE = (1 << 0)
} GFileCreateFlags;


/**
 * GMountMountFlags:
 * @G_MOUNT_MOUNT_NONE: No flags set.
 *
 * Flags used when mounting a mount.
 */
typedef enum {
  G_MOUNT_MOUNT_NONE = 0
} GMountMountFlags;


/**
 * GMountUnmountFlags:
 * @G_MOUNT_UNMOUNT_NONE: No flags set.
 * @G_MOUNT_UNMOUNT_FORCE: Unmount even if there are outstanding
 *  file operations on the mount.
 *
 * Flags used when an unmounting a mount.
 */
typedef enum {
  G_MOUNT_UNMOUNT_NONE  = 0,
  G_MOUNT_UNMOUNT_FORCE = (1 << 0)
} GMountUnmountFlags;


/**
 * GFileCopyFlags:
 * @G_FILE_COPY_NONE: No flags set.
 * @G_FILE_COPY_OVERWRITE: Overwrite any existing files
 * @G_FILE_COPY_BACKUP: Make a backup of any existing files.
 * @G_FILE_COPY_NOFOLLOW_SYMLINKS: Don't follow symlinks.
 * @G_FILE_COPY_ALL_METADATA: Copy all file metadata instead of just default set used for copy (see #GFileInfo).
 * @G_FILE_COPY_NO_FALLBACK_FOR_MOVE: Don't use copy and delete fallback if native move not supported.
 *
 * Flags used when copying or moving files.
 */
typedef enum {
  G_FILE_COPY_NONE                 = 0,          /*< nick=none >*/
  G_FILE_COPY_OVERWRITE            = (1 << 0),
  G_FILE_COPY_BACKUP               = (1 << 1),
  G_FILE_COPY_NOFOLLOW_SYMLINKS    = (1 << 2),
  G_FILE_COPY_ALL_METADATA         = (1 << 3),
  G_FILE_COPY_NO_FALLBACK_FOR_MOVE = (1 << 4)
} GFileCopyFlags;


/**
 * GFileMonitorFlags:
 * @G_FILE_MONITOR_NONE: No flags set.
 * @G_FILE_MONITOR_WATCH_MOUNTS: Watch for mount events.
 *
 * Flags used to set what a #GFileMonitor will watch for.
 */
typedef enum {
  G_FILE_MONITOR_NONE         = 0,
  G_FILE_MONITOR_WATCH_MOUNTS = (1 << 0)
} GFileMonitorFlags;


/**
 * GFileType:
 * @G_FILE_TYPE_UNKNOWN: File's type is unknown.
 * @G_FILE_TYPE_REGULAR: File handle represents a regular file.
 * @G_FILE_TYPE_DIRECTORY: File handle represents a directory.
 * @G_FILE_TYPE_SYMBOLIC_LINK: File handle represents a symbolic link
 *    (Unix systems).
 * @G_FILE_TYPE_SPECIAL: File is a "special" file, such as a socket, fifo,
 *    block device, or character device.
 * @G_FILE_TYPE_SHORTCUT: File is a shortcut (Windows systems).
 * @G_FILE_TYPE_MOUNTABLE: File is a mountable location.
 *
 * Indicates the file's on-disk type.
 **/
typedef enum {
  G_FILE_TYPE_UNKNOWN = 0,
  G_FILE_TYPE_REGULAR,
  G_FILE_TYPE_DIRECTORY,
  G_FILE_TYPE_SYMBOLIC_LINK,
  G_FILE_TYPE_SPECIAL, /* socket, fifo, blockdev, chardev */
  G_FILE_TYPE_SHORTCUT,
  G_FILE_TYPE_MOUNTABLE
} GFileType;


/**
 * GFilesystemPreviewType:
 * @G_FILESYSTEM_PREVIEW_TYPE_IF_ALWAYS: Only preview files if user has explicitly requested it.
 * @G_FILESYSTEM_PREVIEW_TYPE_IF_LOCAL: Preview files if user has requested preview of "local" files.
 * @G_FILESYSTEM_PREVIEW_TYPE_NEVER: Never preview files.
 *
 * Indicates a hint from the file system whether files should be
 * previewed in a file manager. Returned as the value of the key
 * #G_FILE_ATTRIBUTE_FILESYSTEM_USE_PREVIEW.
 **/
typedef enum {
  G_FILESYSTEM_PREVIEW_TYPE_IF_ALWAYS = 0,
  G_FILESYSTEM_PREVIEW_TYPE_IF_LOCAL,
  G_FILESYSTEM_PREVIEW_TYPE_NEVER
} GFilesystemPreviewType;


/**
 * GFileMonitorEvent:
 * @G_FILE_MONITOR_EVENT_CHANGED: a file changed.
 * @G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT: a hint that this was probably the last change in a set of changes.
 * @G_FILE_MONITOR_EVENT_DELETED: a file was deleted.
 * @G_FILE_MONITOR_EVENT_CREATED: a file was created.
 * @G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED: a file attribute was changed.
 * @G_FILE_MONITOR_EVENT_PRE_UNMOUNT: the file location will soon be unmounted.
 * @G_FILE_MONITOR_EVENT_UNMOUNTED: the file location was unmounted.
 *
 * Specifies what type of event a monitor event is.
 **/
typedef enum {
  G_FILE_MONITOR_EVENT_CHANGED,
  G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT,
  G_FILE_MONITOR_EVENT_DELETED,
  G_FILE_MONITOR_EVENT_CREATED,
  G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED,
  G_FILE_MONITOR_EVENT_PRE_UNMOUNT,
  G_FILE_MONITOR_EVENT_UNMOUNTED
} GFileMonitorEvent;


/* This enumeration conflicts with GIOError in giochannel.h. However,
 * that is only used as a return value in some deprecated functions.
 * So, we reuse the same prefix for the enumeration values, but call
 * the actual enumeration (which is rarely used) GIOErrorEnum.
 */
/**
 * GIOErrorEnum:
 * @G_IO_ERROR_FAILED: Generic error condition for when any operation fails.
 * @G_IO_ERROR_NOT_FOUND: File not found error.
 * @G_IO_ERROR_EXISTS: File already exists error.
 * @G_IO_ERROR_IS_DIRECTORY: File is a directory error.
 * @G_IO_ERROR_NOT_DIRECTORY: File is not a directory.
 * @G_IO_ERROR_NOT_EMPTY: File is a directory that isn't empty.
 * @G_IO_ERROR_NOT_REGULAR_FILE: File is not a regular file.
 * @G_IO_ERROR_NOT_SYMBOLIC_LINK: File is not a symbolic link.
 * @G_IO_ERROR_NOT_MOUNTABLE_FILE: File cannot be mounted.
 * @G_IO_ERROR_FILENAME_TOO_LONG: Filename is too many characters.
 * @G_IO_ERROR_INVALID_FILENAME: Filename is invalid or contains invalid characters.
 * @G_IO_ERROR_TOO_MANY_LINKS: File contains too many symbolic links.
 * @G_IO_ERROR_NO_SPACE: No space left on drive.
 * @G_IO_ERROR_INVALID_ARGUMENT: Invalid argument.
 * @G_IO_ERROR_PERMISSION_DENIED: Permission denied.
 * @G_IO_ERROR_NOT_SUPPORTED: Operation not supported for the current backend.
 * @G_IO_ERROR_NOT_MOUNTED: File isn't mounted.
 * @G_IO_ERROR_ALREADY_MOUNTED: File is already mounted.
 * @G_IO_ERROR_CLOSED: File was closed.
 * @G_IO_ERROR_CANCELLED: Operation was cancelled. See #GCancellable.
 * @G_IO_ERROR_PENDING: Operations are still pending.
 * @G_IO_ERROR_READ_ONLY: File is read only.
 * @G_IO_ERROR_CANT_CREATE_BACKUP: Backup couldn't be created.
 * @G_IO_ERROR_WRONG_ETAG: File's Entity Tag was incorrect.
 * @G_IO_ERROR_TIMED_OUT: Operation timed out.
 * @G_IO_ERROR_WOULD_RECURSE: Operation would be recursive.
 * @G_IO_ERROR_BUSY: File is busy.
 * @G_IO_ERROR_WOULD_BLOCK: Operation would block.
 * @G_IO_ERROR_HOST_NOT_FOUND: Host couldn't be found (remote operations).
 * @G_IO_ERROR_WOULD_MERGE: Operation would merge files.
 * @G_IO_ERROR_FAILED_HANDLED: Operation failed and a helper program has already interacted with the user. Do not display any error dialog.
 *
 * Error codes returned by GIO functions.
 *
 **/
typedef enum {
  G_IO_ERROR_FAILED,
  G_IO_ERROR_NOT_FOUND,
  G_IO_ERROR_EXISTS,
  G_IO_ERROR_IS_DIRECTORY,
  G_IO_ERROR_NOT_DIRECTORY,
  G_IO_ERROR_NOT_EMPTY,
  G_IO_ERROR_NOT_REGULAR_FILE,
  G_IO_ERROR_NOT_SYMBOLIC_LINK,
  G_IO_ERROR_NOT_MOUNTABLE_FILE,
  G_IO_ERROR_FILENAME_TOO_LONG,
  G_IO_ERROR_INVALID_FILENAME,
  G_IO_ERROR_TOO_MANY_LINKS,
  G_IO_ERROR_NO_SPACE,
  G_IO_ERROR_INVALID_ARGUMENT,
  G_IO_ERROR_PERMISSION_DENIED,
  G_IO_ERROR_NOT_SUPPORTED,
  G_IO_ERROR_NOT_MOUNTED,
  G_IO_ERROR_ALREADY_MOUNTED,
  G_IO_ERROR_CLOSED,
  G_IO_ERROR_CANCELLED,
  G_IO_ERROR_PENDING,
  G_IO_ERROR_READ_ONLY,
  G_IO_ERROR_CANT_CREATE_BACKUP,
  G_IO_ERROR_WRONG_ETAG,
  G_IO_ERROR_TIMED_OUT,
  G_IO_ERROR_WOULD_RECURSE,
  G_IO_ERROR_BUSY,
  G_IO_ERROR_WOULD_BLOCK,
  G_IO_ERROR_HOST_NOT_FOUND,
  G_IO_ERROR_WOULD_MERGE,
  G_IO_ERROR_FAILED_HANDLED
} GIOErrorEnum;


/**
 * GAskPasswordFlags:
 * @G_ASK_PASSWORD_NEED_PASSWORD: operation requires a password.
 * @G_ASK_PASSWORD_NEED_USERNAME: operation requires a username.
 * @G_ASK_PASSWORD_NEED_DOMAIN: operation requires a domain.
 * @G_ASK_PASSWORD_SAVING_SUPPORTED: operation supports saving settings.
 * @G_ASK_PASSWORD_ANONYMOUS_SUPPORTED: operation supports anonymous users.
 *
 * #GAskPasswordFlags are used to request specific information from the
 * user, or to notify the user of their choices in an authentication
 * situation.
 **/
typedef enum {
  G_ASK_PASSWORD_NEED_PASSWORD       = (1 << 0),
  G_ASK_PASSWORD_NEED_USERNAME       = (1 << 1),
  G_ASK_PASSWORD_NEED_DOMAIN         = (1 << 2),
  G_ASK_PASSWORD_SAVING_SUPPORTED    = (1 << 3),
  G_ASK_PASSWORD_ANONYMOUS_SUPPORTED = (1 << 4)
} GAskPasswordFlags;


/**
 * GPasswordSave:
 * @G_PASSWORD_SAVE_NEVER: never save a password.
 * @G_PASSWORD_SAVE_FOR_SESSION: save a password for the session.
 * @G_PASSWORD_SAVE_PERMANENTLY: save a password permanently.
 *
 * #GPasswordSave is used to indicate the lifespan of a saved password.
 *
 * #Gvfs stores passwords in the Gnome keyring when this flag allows it
 * to, and later retrieves it again from there.
 **/
typedef enum {
  G_PASSWORD_SAVE_NEVER,
  G_PASSWORD_SAVE_FOR_SESSION,
  G_PASSWORD_SAVE_PERMANENTLY
} GPasswordSave;


/**
 * GMountOperationResult:
 * @G_MOUNT_OPERATION_HANDLED: The request was fulfilled and the
 *     user specified data is now available
 * @G_MOUNT_OPERATION_ABORTED: The user requested the mount operation
 *     to be aborted
 * @G_MOUNT_OPERATION_UNHANDLED: The request was unhandled (i.e. not
 *     implemented)
 *
 * #GMountOperationResult is returned as a result when a request for
 * information is send by the mounting operation.
 **/
typedef enum {
  G_MOUNT_OPERATION_HANDLED,
  G_MOUNT_OPERATION_ABORTED,
  G_MOUNT_OPERATION_UNHANDLED
} GMountOperationResult;


/**
 * GOutputStreamSpliceFlags:
 * @G_OUTPUT_STREAM_SPLICE_NONE: Do not close either stream.
 * @G_OUTPUT_STREAM_SPLICE_CLOSE_SOURCE: Close the source stream after
 *     the splice.
 * @G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET: Close the target stream after
 *     the splice.
 *
 * GOutputStreamSpliceFlags determine how streams should be spliced.
 **/
typedef enum {
  G_OUTPUT_STREAM_SPLICE_NONE         = 0,
  G_OUTPUT_STREAM_SPLICE_CLOSE_SOURCE = (1 << 0),
  G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET = (1 << 1)
} GOutputStreamSpliceFlags;


/**
 * GEmblemOrigin:
 * @G_EMBLEM_ORIGIN_UNKNOWN: Emblem of unknown origin
 * @G_EMBLEM_ORIGIN_DEVICE: Embleme adds device-specific information
 * @G_EMBLEM_ORIGIN_LIVEMETADATA: Emblem depicts live metadata, such as "readonly"
 * @G_EMBLEM_ORIGIN_TAG: Emblem comes from a user-defined tag, e.g. set by nautilus (in the future)
 *
 * GEmblemOrigin is used to add information about the origin of the emblem
 * to #GEmblem.
 *
 * Since: 2.18
 */
typedef enum  {
  G_EMBLEM_ORIGIN_UNKNOWN,
  G_EMBLEM_ORIGIN_DEVICE,
  G_EMBLEM_ORIGIN_LIVEMETADATA,
  G_EMBLEM_ORIGIN_TAG
} GEmblemOrigin;


G_END_DECLS

#endif /* __GIO_ENUMS_H__ */
