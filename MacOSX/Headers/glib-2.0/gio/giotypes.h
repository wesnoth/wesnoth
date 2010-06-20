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

#ifndef __GIO_TYPES_H__
#define __GIO_TYPES_H__

#include <gio/gioenums.h>

G_BEGIN_DECLS

typedef struct _GAppLaunchContext             GAppLaunchContext;
typedef struct _GAppInfo                      GAppInfo; /* Dummy typedef */
typedef struct _GAsyncResult                  GAsyncResult; /* Dummy typedef */
typedef struct _GBufferedInputStream          GBufferedInputStream;
typedef struct _GBufferedOutputStream         GBufferedOutputStream;
typedef struct _GCancellable                  GCancellable;
typedef struct _GDataInputStream              GDataInputStream;

/**
 * GDrive:
 *
 * Opaque drive object.
 **/
typedef struct _GDrive                        GDrive; /* Dummy typedef */
typedef struct _GFileEnumerator               GFileEnumerator;
typedef struct _GFileMonitor                  GFileMonitor;
typedef struct _GFilterInputStream            GFilterInputStream;
typedef struct _GFilterOutputStream           GFilterOutputStream;

/**
 * GFile:
 *
 * A handle to an object implementing the #GFileIface interface.
 * Generally stores a location within the file system. Handles do not
 * necessarily represent files or directories that currently exist.
 **/
typedef struct _GFile                         GFile; /* Dummy typedef */
typedef struct _GFileInfo                     GFileInfo;

/**
 * GFileAttributeMatcher:
 *
 * Determines if a string matches a file attribute.
 **/
typedef struct _GFileAttributeMatcher         GFileAttributeMatcher;
typedef struct _GFileAttributeInfo            GFileAttributeInfo;
typedef struct _GFileAttributeInfoList        GFileAttributeInfoList;
typedef struct _GFileInputStream              GFileInputStream;
typedef struct _GFileOutputStream             GFileOutputStream;
typedef struct _GFileIcon                     GFileIcon;
typedef struct _GFilenameCompleter            GFilenameCompleter;


typedef struct _GIcon                         GIcon; /* Dummy typedef */
typedef struct _GInputStream                  GInputStream;
typedef struct _GIOModule                     GIOModule;
typedef struct _GIOExtensionPoint             GIOExtensionPoint;
typedef struct _GIOExtension                  GIOExtension;

/**
 * GIOSchedulerJob:
 *
 * Opaque class for definining and scheduling IO jobs.
 **/
typedef struct _GIOSchedulerJob               GIOSchedulerJob;
typedef struct _GLoadableIcon                 GLoadableIcon; /* Dummy typedef */
typedef struct _GMemoryInputStream            GMemoryInputStream;
typedef struct _GMemoryOutputStream           GMemoryOutputStream;

/**
 * GMount:
 *
 * A handle to an object implementing the #GMountIface interface.
 **/
typedef struct _GMount                        GMount; /* Dummy typedef */
typedef struct _GMountOperation               GMountOperation;
typedef struct _GOutputStream                 GOutputStream;
typedef struct _GSeekable                     GSeekable;
typedef struct _GSimpleAsyncResult            GSimpleAsyncResult;
typedef struct _GThemedIcon                   GThemedIcon;
typedef struct _GVfs                          GVfs; /* Dummy typedef */

/**
 * GVolume:
 *
 * Opaque mountable volume object.
 **/
typedef struct _GVolume                       GVolume; /* Dummy typedef */
typedef struct _GVolumeMonitor                GVolumeMonitor;

/**
 * GAsyncReadyCallback:
 * @source_object: the object the asynchronous operation was started with.
 * @res: a #GAsyncResult.
 * @user_data: user data passed to the callback.
 *
 * Type definition for a function that will be called back when an asynchronous
 * operation within GIO has been completed.
 **/
typedef void (*GAsyncReadyCallback) (GObject *source_object,
				     GAsyncResult *res,
				     gpointer user_data);

/**
 * GFileProgressCallback:
 * @current_num_bytes: the current number of bytes in the operation.
 * @total_num_bytes: the total number of bytes in the operation.
 * @user_data: user data passed to the callback.
 *
 * When doing file operations that may take a while, such as moving
 * a file or copying a file, a progress callback is used to pass how
 * far along that operation is to the application.
 **/
typedef void (*GFileProgressCallback) (goffset current_num_bytes,
                                       goffset total_num_bytes,
                                       gpointer user_data);

/**
 * GFileReadMoreCallback:
 * @file_contents: the data as currently read.
 * @file_size: the size of the data currently read.
 * @callback_data: data passed to the callback.
 *
 * When loading the partial contents of a file with g_file_read_partial_contents(),
 * it may become necessary to determine if any more data from the file should be loaded.
 * A #GFileReadMoreCallback function facilitates this by returning %TRUE if more data
 * should be read, or %FALSE otherwise.
 *
 * Returns: %TRUE if more data should be read back. %FALSE otherwise.
 **/
typedef gboolean (* GFileReadMoreCallback) (const char *file_contents,
                                            goffset file_size,
                                            gpointer callback_data);


/**
 * GIOSchedulerJobFunc:
 * @job: a #GIOSchedulerJob.
 * @cancellable: optional #GCancellable object, %NULL to ignore.
 * @user_data: the data to pass to callback function
 *
 * I/O Job function.
 *
 * Note that depending on whether threads are available, the
 * #GIOScheduler may run jobs in separate threads or in an idle
 * in the mainloop.
 *
 * Long-running jobs should periodically check the @cancellable
 * to see if they have been cancelled.
 *
 * Returns: %TRUE if this function should be called again to
 *    complete the job, %FALSE if the job is complete (or cancelled)
 **/
typedef gboolean (*GIOSchedulerJobFunc) (GIOSchedulerJob *job,
					 GCancellable    *cancellable,
					 gpointer         user_data);

/**
 * GSimpleAsyncThreadFunc:
 * @res: a #GSimpleAsyncResult.
 * @object: a #GObject.
 * @cancellable: optional #GCancellable object, %NULL to ignore.
 *
 * Simple thread function that runs an asynchronous operation and
 * checks for cancellation.
 **/
typedef void (*GSimpleAsyncThreadFunc) (GSimpleAsyncResult *res,
                                        GObject *object,
                                        GCancellable *cancellable);

G_END_DECLS

#endif /* __GIO_TYPES_H__ */
