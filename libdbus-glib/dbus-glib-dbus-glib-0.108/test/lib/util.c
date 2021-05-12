/* Regression test utilities
 *
 * Copyright © 2009 Collabora Ltd. <http://www.collabora.co.uk/>
 * Copyright © 2009-2011 Nokia Corporation
 *
 * Licensed under the Academic Free License version 2.1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#include <config.h>

#include "util.h"

static void
destroy_cb (DBusGProxy *proxy G_GNUC_UNUSED,
            gpointer user_data)
{
  gboolean *disconnected = user_data;

  *disconnected = TRUE;
}

void
test_run_until_disconnected (DBusGConnection *connection,
                             GMainContext *context)
{
  gboolean disconnected = FALSE;
  DBusGProxy *proxy;

  g_printerr ("Disconnecting... ");

  dbus_connection_set_exit_on_disconnect (dbus_g_connection_get_connection (connection),
                                          FALSE);

  /* low-level tests might not have called this yet */
  g_type_init ();

  proxy = dbus_g_proxy_new_for_peer (connection, "/",
                                     "org.freedesktop.DBus.Peer");
  g_signal_connect (G_OBJECT (proxy), "destroy", G_CALLBACK (destroy_cb),
                    &disconnected);

  dbus_connection_close (dbus_g_connection_get_connection (connection));

  while (!disconnected)
    {
      g_printerr (".");
      g_main_context_iteration (context, TRUE);
    }

  g_signal_handlers_disconnect_by_func (proxy, destroy_cb, &disconnected);
  g_object_unref (proxy);

  g_printerr (" disconnected\n");
}
