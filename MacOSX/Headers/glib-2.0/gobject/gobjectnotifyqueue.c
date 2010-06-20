/* GObject - GLib Type, Object, Parameter and Signal Library
 * Copyright (C) 1998-1999, 2000-2001 Tim Janik and Red Hat, Inc.
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
 */

#ifndef __G_OBJECT_NOTIFY_QUEUE_H__
#define __G_OBJECT_NOTIFY_QUEUE_H__

#include <string.h> /* memset */

#include <glib-object.h>

G_BEGIN_DECLS


/* --- typedefs --- */
typedef struct _GObjectNotifyContext          GObjectNotifyContext;
typedef struct _GObjectNotifyQueue            GObjectNotifyQueue;
typedef void (*GObjectNotifyQueueDispatcher) (GObject     *object,
					      guint        n_pspecs,
					      GParamSpec **pspecs);


/* --- structures --- */
struct _GObjectNotifyContext
{
  GQuark                       quark_notify_queue;
  GObjectNotifyQueueDispatcher dispatcher;
  GTrashStack                 *_nqueue_trash; /* unused */
};
struct _GObjectNotifyQueue
{
  GObjectNotifyContext *context;
  GSList               *pspecs;
  guint16               n_pspecs;
  guint16               freeze_count;
  /* currently, this structure abuses the GList allocation chain and thus
   * must be <= sizeof (GList)
   */
};


/* --- functions --- */
static void
g_object_notify_queue_free (gpointer data)
{
  GObjectNotifyQueue *nqueue = data;

  g_slist_free (nqueue->pspecs);
  g_list_free_1 ((void*) nqueue);
}

static inline GObjectNotifyQueue*
g_object_notify_queue_freeze (GObject		   *object,
			      GObjectNotifyContext *context)
{
  GObjectNotifyQueue *nqueue;

  nqueue = g_datalist_id_get_data (&object->qdata, context->quark_notify_queue);
  if (!nqueue)
    {
      nqueue = (void*) g_list_alloc ();
      memset (nqueue, 0, sizeof (*nqueue));
      nqueue->context = context;
      g_datalist_id_set_data_full (&object->qdata, context->quark_notify_queue,
				   nqueue, g_object_notify_queue_free);
    }

  g_return_val_if_fail (nqueue->freeze_count < 65535, nqueue);
  nqueue->freeze_count++;

  return nqueue;
}

static inline void
g_object_notify_queue_thaw (GObject            *object,
			    GObjectNotifyQueue *nqueue)
{
  GObjectNotifyContext *context = nqueue->context;
  GParamSpec *pspecs_mem[16], **pspecs, **free_me = NULL;
  GSList *slist;
  guint n_pspecs = 0;

  g_return_if_fail (nqueue->freeze_count > 0);

  nqueue->freeze_count--;
  if (nqueue->freeze_count)
    return;
  g_return_if_fail (object->ref_count > 0);

  pspecs = nqueue->n_pspecs > 16 ? free_me = g_new (GParamSpec*, nqueue->n_pspecs) : pspecs_mem;
  /* set first entry to NULL since it's checked unconditionally */
  pspecs[0] = NULL;
  for (slist = nqueue->pspecs; slist; slist = slist->next)
    {
      GParamSpec *pspec = slist->data;
      guint i = 0;

      /* dedup, make pspecs in the list unique */
    redo_dedup_check:
      if (pspecs[i] == pspec)
	continue;
      if (++i < n_pspecs)
	goto redo_dedup_check;

      pspecs[n_pspecs++] = pspec;
    }
  g_datalist_id_set_data (&object->qdata, context->quark_notify_queue, NULL);

  if (n_pspecs)
    context->dispatcher (object, n_pspecs, pspecs);
  g_free (free_me);
}

static inline void
g_object_notify_queue_clear (GObject            *object,
			     GObjectNotifyQueue *nqueue)
{
  g_return_if_fail (nqueue->freeze_count > 0);

  g_slist_free (nqueue->pspecs);
  nqueue->pspecs = NULL;
  nqueue->n_pspecs = 0;
}

static inline void
g_object_notify_queue_add (GObject            *object,
			   GObjectNotifyQueue *nqueue,
			   GParamSpec	      *pspec)
{
  if (pspec->flags & G_PARAM_READABLE)
    {
      GParamSpec *redirect;

      g_return_if_fail (nqueue->n_pspecs < 65535);

      redirect = g_param_spec_get_redirect_target (pspec);
      if (redirect)
	pspec = redirect;
	    
      /* we do the deduping in _thaw */
      nqueue->pspecs = g_slist_prepend (nqueue->pspecs, pspec);
      nqueue->n_pspecs++;
    }
}

static inline GObjectNotifyQueue*
g_object_notify_queue_from_object (GObject              *object,
				   GObjectNotifyContext *context)
{
  return g_datalist_id_get_data (&object->qdata, context->quark_notify_queue);
}


G_END_DECLS

#endif /* __G_OBJECT_NOTIFY_QUEUE_H__ */
