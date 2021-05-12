#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <dbus/dbus.h>
#include <glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include "test/lib/util.h"

DBusConnection *bus;
GMainContext *main_context;

typedef struct _SpiReentrantCallClosure
{
  GMainLoop   *loop;
  DBusMessage *reply;
} SpiReentrantCallClosure;

static void
set_reply (DBusPendingCall * pending, void *user_data)
{
  SpiReentrantCallClosure* closure = (SpiReentrantCallClosure *) user_data;

  closure->reply = dbus_pending_call_steal_reply (pending);
  dbus_connection_setup_with_g_main (bus, NULL);

  g_main_loop_quit (closure->loop);
}

static DBusMessage *
send_and_allow_reentry (DBusConnection * bus, DBusMessage * message,
                        dbus_bool_t switch_after_send)
{
  DBusPendingCall *pending;
  SpiReentrantCallClosure closure;

  closure.loop = g_main_loop_new (main_context, FALSE);
  dbus_connection_setup_with_g_main (bus, (switch_after_send ? NULL :
                                                               main_context));

  if (!dbus_connection_send_with_reply (bus, message, &pending, 3000))
    {
  dbus_connection_setup_with_g_main (bus, NULL);
      return NULL;
    }
  dbus_pending_call_set_notify (pending, set_reply, (void *) &closure, NULL);
  if (switch_after_send)
    dbus_connection_setup_with_g_main (bus, main_context);
  g_main_loop_run  (closure.loop);

  g_main_loop_unref (closure.loop);
  dbus_pending_call_unref (pending);
  return closure.reply;
}

static void
send_test_message (dbus_bool_t switch_after_send)
{
  DBusMessage *message, *reply;
  const char *str;
  DBusError error;

  dbus_error_init (&error);
  message = dbus_message_new_method_call ("org.freedesktop.DBus",
                                          "/org/freedesktop/DBus",
                                          DBUS_INTERFACE_DBUS, "GetId");
  reply = send_and_allow_reentry (bus, message, switch_after_send);
  if (!reply)
  {
    fprintf(stderr, "Got no reply from send_and_allow_reentry\n");
    exit(1);
  }
  if (dbus_message_get_type (reply) == DBUS_MESSAGE_TYPE_ERROR)
  {
    char *err = NULL;
    dbus_message_get_args (reply, NULL, DBUS_TYPE_STRING, &err, DBUS_TYPE_INVALID);
    fprintf (stderr, "Got error: %s\n", err);
    exit(1);
  }
  if (!dbus_message_get_args (reply, &error, DBUS_TYPE_STRING, &str, DBUS_TYPE_INVALID))
  {
    fprintf(stderr, "Sorry; can't communicate: %s\n", error.message);
    exit(1);
  }
  dbus_message_unref (reply);
  dbus_message_unref (message);
}

int
main(int argc, const char *argv[])
{
  DBusError error;

  main_context = g_main_context_new ();
  dbus_error_init (&error);
  bus = dbus_bus_get_private (DBUS_BUS_SESSION, &error);
  if (!bus)
  {
    fprintf(stderr, "Couldn't connect to bus: %s\n", error.name);
    return 1;
  }
  dbus_connection_setup_with_g_main (bus, NULL);
  send_test_message (FALSE);
  send_test_message (FALSE);
  send_test_message (TRUE);

  test_run_until_disconnected (dbus_connection_get_g_connection (bus),
                               NULL);
  dbus_connection_unref (bus);

  dbus_shutdown ();
  g_main_context_unref (main_context);

  return 0;
}
