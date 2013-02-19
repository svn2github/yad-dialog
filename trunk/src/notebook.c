/*
 * This file is part of YAD.
 *
 * YAD is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * YAD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YAD. If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2008-2013, Victor Ananjevsky <ananasik@gmail.com>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include "yad.h"

static GtkWidget *notebook;

GtkWidget *
notebook_create_widget (GtkWidget * dlg)
{
  GtkWidget *w;
  GSList *tab;

  /* get shared memory */
  tabs = get_tabs (options.notebook_data.key, TRUE);
  if (!tabs)
    exit (-1);

  /* screate widget */
  w = notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (w), 5);

  /* add tabs */
  for (tab = options.notebook_data.tabs; tab; tab = tab->next)
    {
      GtkWidget *l, *s;

      l = gtk_label_new (NULL);
      if (!options.data.no_markup)
        gtk_label_set_markup (GTK_LABEL (l), (gchar *) tab->data);
      else
        gtk_label_set_text (GTK_LABEL (l), (gchar *) tab->data);
      gtk_misc_set_alignment (GTK_MISC (l), options.common_data.align, 0.5);

      s = gtk_socket_new ();
      gtk_container_set_border_width (GTK_CONTAINER (s), options.notebook_data.borders);

      gtk_notebook_append_page (GTK_NOTEBOOK (w), s, l);
    }

  return w;
}

void
notebook_swallow_childs (void)
{
  guint i, n_tabs;

  n_tabs = g_slist_length (options.notebook_data.tabs);

  /* wait until all children are register */
  while (tabs[0].xid != n_tabs)
    usleep (1000);

  for (i = 1; i <= n_tabs; i++)
    {
      GtkWidget *s = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), i - 1);

      if (tabs[i].pid != -1)
        gtk_socket_add_id (GTK_SOCKET (s), tabs[i].xid);
    }
}

void
notebook_print_result (void)
{
  guint i, n_tabs;

  n_tabs = g_slist_length (options.notebook_data.tabs);
  for (i = 1; i <= n_tabs; i++)
    {
      if (tabs[i].pid != -1)
        kill (tabs[i].pid, SIGUSR1);
    }
}

void
notebook_close_childs (void)
{
  guint i, n_tabs;
  struct shmid_ds buf;

  gtk_widget_destroy (notebook);

  n_tabs = g_slist_length (options.notebook_data.tabs);
  for (i = 1; i <= n_tabs; i++)
    {
      if (tabs[i].pid != -1)
        kill (tabs[i].pid, SIGUSR2);
    }

  /* cleanup shared memory */
  shmctl (tabs[0].pid, IPC_RMID, &buf);
  shmdt (tabs);
}
