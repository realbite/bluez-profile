#include <gio/gio.h>
#include <gio/gunixfdlist.h>
#include <stdlib.h>
#include <ruby.h>

// this ruby binding allows a profile to be registered
// with the bluez stack.

#define BLUEZ_BUS_NAME                   "org.bluez"
#define BLUEZ_BUS_PATH                   "/org/bluez"
#define BLUEZ_INTERFACE_DEVICE1          "org.bluez.Device1"
#define BLUEZ_INTERFACE_PROFILE1         "org.bluez.Profile1"
#define BLUEZ_INTERFACE_PROFILEMANAGER1  "org.bluez.ProfileManager1"

static VALUE mBluez;
static VALUE cProfile;
static VALUE eProfileError;

typedef struct t_profile_data{
    GDBusConnection* conn;
    GDBusProxy*      proxy;
    GDBusNodeInfo*   introspection_data;
    guint            id;
    GMainLoop*       loop;
    int              running;
    int              fd;
} t_profile_data;


/* Introspection data for the service we are exporting */
static const gchar introspection_xml[] =
  "<node>"
  "  <interface name='org.bluez.Profile1'>"
  "    <method name='Release'>"
  "    </method>"
  "    <method name='NewConnection'>"
  "      <arg type='o' name='device' direction='in'/>"
  "      <arg type='h' name='fd' direction='in'/>"
  "      <arg type='a{sv}' name='fd_properties' direction='in'/>"
  "    </method>"
  "    <method name='RequestDisconnection'>"
  "      <arg type='o' name='device' direction='in'/>"
  "    </method>"
  "  </interface>"
  "</node>";

// free the c data structure

static void profile_free_data(t_profile_data* p){
    if (p->id !=0 ) g_dbus_connection_unregister_object(p->conn, p->id);
    if (p->introspection_data !=NULL ) g_dbus_node_info_unref (p->introspection_data);
    if (p->proxy !=NULL ) g_object_unref (p->proxy);
    if (p->conn !=NULL ) g_object_unref (p->conn);
    if (p->loop !=NULL ) g_main_loop_unref(p->loop);
    p->introspection_data = NULL;
    p->proxy = NULL;
    p->conn = NULL;
    p->loop = NULL;
    free(p);
}

// allocate the c data structure

static VALUE profile_alloc_data(VALUE self){
  VALUE obj;
  t_profile_data* data = malloc(sizeof(t_profile_data));
  data->conn = NULL;
  data->proxy = NULL;
  data->introspection_data = NULL;
  obj = Data_Wrap_Struct(self, 0, profile_free_data, data);
  return obj;
}

// handle object method calls    ----------------------------------------------
static void
handle_method_call (GDBusConnection       *connection,
                    const gchar           *sender,
                    const gchar           *object_path,
                    const gchar           *interface_name,
                    const gchar           *method_name,
                    GVariant              *parameters,
                    GDBusMethodInvocation *invocation,
                    gpointer               user_data)
{
  //MyObject *myobj = user_data;

  VALUE self;
  GDBusMessage *message;
  GUnixFDList *fd_list;
	GError *error = NULL;
  gchar*      path;
  int         fd;
  GVariantIter *list;


  self = (VALUE) user_data;

	if (g_strcmp0 (method_name, "Release") == 0)
    {
      //printf("Release Called\n");
      rb_funcall(self,  rb_intern("release"), 0 );
    }
  else if (g_strcmp0 (method_name, "NewConnection") == 0)
  // in_signature="oha{sv}"
    {
        g_variant_get (parameters, "(oha{sv})", &path, &fd, &list);
        message = g_dbus_method_invocation_get_message (invocation);
        fd_list = g_dbus_message_get_unix_fd_list (message);
        fd = g_unix_fd_list_get (fd_list, 0, &error);
        if (error!=NULL){
          rb_raise(eProfileError, "invalid file descriptor");
          return;
        }
        rb_funcall(self,  rb_intern("connection"), 2,rb_str_new2(path), INT2NUM(fd) );
    }
  else if (g_strcmp0 (method_name, "RequestDisconnection") == 0)
  // in_signature="o"
    {
      //printf("RequestDisconnection Called\n");
      g_variant_get (parameters, "o", &path);
      rb_funcall(self,  rb_intern("disconnection"), 1,rb_str_new2(path) );
    }

}

// static GVariant *
// handle_get_property (GDBusConnection  *connection,
//                      const gchar      *sender,
//                      const gchar      *object_path,
//                      const gchar      *interface_name,
//                      const gchar      *property_name,
//                      GError          **error,
//                      gpointer          user_data)
// {
//   return NULL;
// }
//
// static gboolean
// handle_set_property (GDBusConnection  *connection,
//                      const gchar      *sender,
//                      const gchar      *object_path,
//                      const gchar      *interface_name,
//                      const gchar      *property_name,
//                      GVariant         *value,
//                      GError          **error,
//                      gpointer          user_data)
// {
//   return TRUE;
// }
//

static const GDBusInterfaceVTable vtable =
{
  handle_method_call,
  NULL,
  NULL
};

// register our profile object with dbus  --------------------------------------

static guint register_object(VALUE self, t_profile_data* data, const gchar* path){
  guint id;
  //gpointer user_data = NULL;
  //GDestroyNotify user_data_free_func;
  GError *error = NULL;

  id = g_dbus_connection_register_object (
    data->conn,
    path,
    data->introspection_data->interfaces[0],
    &vtable,
    (gpointer) self, //user_data,
    NULL, //user_data_free_func,
    &error
  );

  return id;
}

// register our profile with bluez --------------------------------------------

int register_profile(VALUE path, VALUE uuid, VALUE options, GDBusProxy *proxy){

  GVariant *profile;
	GVariantBuilder profile_builder;
	GError *error = NULL;
  VALUE val;

	//printf("register_profile called!\n");

	g_variant_builder_init(&profile_builder, G_VARIANT_TYPE("(osa{sv})"));

	g_variant_builder_add (&profile_builder, "o",StringValueCStr(path));

	g_variant_builder_add (&profile_builder, "s", StringValueCStr(uuid));

	g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("a{sv}"));

  // Name

  val = rb_hash_aref(options, ID2SYM(rb_intern("name")));
  if (val != Qnil){
    g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
  	g_variant_builder_add (&profile_builder, "s", "Name");
  	g_variant_builder_add (&profile_builder, "v",
  			g_variant_new_string(StringValueCStr(val) ));
  	g_variant_builder_close(&profile_builder);
  }

  // Service

  val = rb_hash_aref(options, ID2SYM(rb_intern("service")));
  if (val != Qnil){
    g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
  	g_variant_builder_add (&profile_builder, "s", "Service");
  	g_variant_builder_add (&profile_builder, "v",
  			g_variant_new_string(StringValueCStr(val) ));
  	g_variant_builder_close(&profile_builder);
  }

  // Role

  val = rb_hash_aref(options, ID2SYM(rb_intern("role")));
  if (val != Qnil){
    //val = rb_const_get_at(rb_mMath, rb_intern("PI_ISH"));
    g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
    g_variant_builder_add (&profile_builder, "s", "Role");
    g_variant_builder_add (&profile_builder, "v",
        g_variant_new_string(StringValueCStr(val)  ));
    g_variant_builder_close(&profile_builder);
  }

  // Channel

  val = rb_hash_aref(options, ID2SYM(rb_intern("channel")));
  if (val != Qnil){
  	g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
  	g_variant_builder_add (&profile_builder, "s", "Channel");
  	g_variant_builder_add (&profile_builder, "v", g_variant_new_uint16(FIX2INT(val)));
  	g_variant_builder_close(&profile_builder);
  }

  // PSM

  val = rb_hash_aref(options, ID2SYM(rb_intern("psm")));
  if (val != Qnil){
  	g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
  	g_variant_builder_add (&profile_builder, "s", "PSM");
  	g_variant_builder_add (&profile_builder, "v", g_variant_new_uint16(FIX2INT(val)));
  	g_variant_builder_close(&profile_builder);
  }

  // RequireAuthentication

  val = rb_hash_aref(options, ID2SYM(rb_intern("authentication")));
  if (val != Qnil){
    g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
    g_variant_builder_add (&profile_builder, "s", "RequireAuthentication");
    g_variant_builder_add (&profile_builder, "v", g_variant_new_boolean(RTEST(val) ));
    g_variant_builder_close(&profile_builder);
  }

  // RequireAuthorization

  val = rb_hash_aref(options, ID2SYM(rb_intern("authorization")));
  if (val != Qnil){
    g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
    g_variant_builder_add (&profile_builder, "s", "RequireAuthorization");
    g_variant_builder_add (&profile_builder, "v", g_variant_new_boolean(RTEST(val) ));
    g_variant_builder_close(&profile_builder);
  }

  // AutoConnect

  val = rb_hash_aref(options, ID2SYM(rb_intern("connect")));
  if (val != Qnil){
    g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
    g_variant_builder_add (&profile_builder, "s", "AutoConnect");
    g_variant_builder_add (&profile_builder, "v", g_variant_new_boolean(RTEST(val) ));
    g_variant_builder_close(&profile_builder);
  }

  // ServiceRecord

  val = rb_hash_aref(options, ID2SYM(rb_intern("record")));
  if (val != Qnil){
    g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
  	g_variant_builder_add (&profile_builder, "s", "ServiceRecord");
  	g_variant_builder_add (&profile_builder, "v",
  			g_variant_new_string(StringValueCStr(val) ));
  	g_variant_builder_close(&profile_builder);
  }

  // Version

  val = rb_hash_aref(options, ID2SYM(rb_intern("version")));
  if (val != Qnil){
  	g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
  	g_variant_builder_add (&profile_builder, "s", "Version");
  	g_variant_builder_add (&profile_builder, "v", g_variant_new_uint16(FIX2INT(val)));
  	g_variant_builder_close(&profile_builder);
  }

  // Features

  val = rb_hash_aref(options, ID2SYM(rb_intern("features")));
  if (val != Qnil){
  	g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
  	g_variant_builder_add (&profile_builder, "s", "Features");
  	g_variant_builder_add (&profile_builder, "v", g_variant_new_uint16(FIX2INT(val)));
  	g_variant_builder_close(&profile_builder);
  }



	// g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
	// g_variant_builder_add (&profile_builder, "s", "Role");
  //
	// if (1) {
	// 	g_variant_builder_add (&profile_builder, "v",
	// 			g_variant_new_string("server"));
	// } else {
	// 	g_variant_builder_add (&profile_builder, "v",
	// 			g_variant_new_string("client"));
	// }
  //
	// g_variant_builder_close(&profile_builder);


	g_variant_builder_close(&profile_builder);
	profile = g_variant_builder_end(&profile_builder);

  g_dbus_proxy_call_sync (proxy,
				"RegisterProfile",
				profile,
				G_DBUS_CALL_FLAGS_NONE,
				-1,
				NULL,
				&error);

  if (error!=NULL){
    return 0;
  }

  return 1;
}


// initialize our profile here. ------------------------------------------------

static VALUE profile_initialize(VALUE self, VALUE path, VALUE uuid, VALUE options){
  t_profile_data* data = NULL;
  GError *error = NULL;
  int ok;
  const gchar * cpath = StringValueCStr(path);
  Data_Get_Struct(self, t_profile_data, data);

  // validate the path

  if (!g_variant_is_object_path(cpath)) {
    rb_raise(eProfileError, "invalid object path");
    return Qnil;
	}

  data->loop = g_main_loop_new (NULL, FALSE);
  data->running = FALSE;

  data->introspection_data = g_dbus_node_info_new_for_xml (introspection_xml, NULL);
  if (data->introspection_data==NULL){
    rb_raise(eProfileError, "error generating introspection data");
    return Qnil;
  }

  // connect to dbus system bus
  data->conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
  if (error!=NULL){
    rb_raise(eProfileError, "error connecting to dbus");
    return Qnil;
  }

  // obtain a proxy to bluez profile manager.

  data->proxy = g_dbus_proxy_new_sync (data->conn,
  				G_DBUS_PROXY_FLAGS_NONE,
  				NULL,/* GDBusInterfaceInfo */
  				BLUEZ_BUS_NAME,/* name */
  				BLUEZ_BUS_PATH,/* object path */
  				BLUEZ_INTERFACE_PROFILEMANAGER1,/* interface */
  				NULL,/* GCancellable */
  				&error);

  if (error!=NULL){
    rb_raise(eProfileError, "error connecting to bluez profile manager");
    return Qnil;
  }

  // register our profile object with dbus

  data->id = register_object(self, data, cpath);

  if (data->id==0){
    rb_raise(eProfileError, "error registering profile object");
    return Qnil;
  }

  // and register the profile with the profile manager.

  ok = register_profile(path, uuid, options, data->proxy);

  if (ok==0){
    rb_raise(eProfileError, "error register profile with bluez");
    return Qnil;
  }

  return self;
}

// run the event loop ----------------------------------------------------------

static VALUE profile_run(VALUE self){
  t_profile_data* data = NULL;
  GMainContext *  context = g_main_context_default();
  Data_Get_Struct(self, t_profile_data, data);
  if (!data->running){
    data->running = TRUE;
    while(data->running){
      g_main_context_iteration (context, FALSE);
      rb_thread_schedule();
    }
  }
  return self;
}

// stop the event loop ---------------------------------------------------------

static VALUE profile_stop(VALUE self){
  t_profile_data* data = NULL;
  Data_Get_Struct(self, t_profile_data, data);
  data->running = FALSE;
  // if (g_main_loop_is_running(data->loop) ){
  //   g_main_loop_quit (data->loop);
  // }
  return self;
}

//=============================== ruby interface definition ====================

void Init_profile() {

  mBluez        = rb_define_module("Bluez");
  cProfile      = rb_define_class_under(mBluez, "Profile", rb_cObject);
  eProfileError = rb_define_class_under(cProfile, "Error",rb_eStandardError);

  rb_define_const(cProfile, "Client", rb_str_new_cstr("client"));
  rb_define_const(cProfile, "Server", rb_str_new_cstr("server"));

  rb_define_alloc_func(cProfile, profile_alloc_data);
  rb_define_method(cProfile, "initialize", profile_initialize, 3);
  rb_define_method(cProfile, "run", profile_run, 0);
  rb_define_method(cProfile, "stop", profile_stop, 0);
}
