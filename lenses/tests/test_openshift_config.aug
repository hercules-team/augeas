(*
Module: Test_OpenShift_Config
  Provides unit tests and examples for the <OpenShift_Config> lens.
*)

module Test_OpenShift_Config =

(* Variable: conf *)
let conf = "CLOUD_DOMAIN=\"example.com\"
VALID_GEAR_SIZES=\"small,medium\"
DEFAULT_MAX_GEARS=\"100\"
DEFAULT_GEAR_CAPABILITIES=\"small\"
DEFAULT_GEAR_SIZE=\"small\"
MONGO_HOST_PORT=\"localhost:27017\"
MONGO_USER=\"openshift\"
MONGO_PASSWORD=\"mooo\"
MONGO_DB=\"openshift_broker_dev\"
MONGO_SSL=\"false\"
ENABLE_USAGE_TRACKING_DATASTORE=\"false\"
ENABLE_USAGE_TRACKING_AUDIT_LOG=\"false\"
USAGE_TRACKING_AUDIT_LOG_FILE=\"/var/log/openshift/broker/usage.log\"
ENABLE_ANALYTICS=\"false\"
ENABLE_USER_ACTION_LOG=\"true\"
USER_ACTION_LOG_FILE=\"/var/log/openshift/broker/user_action.log\"
AUTH_PRIVKEYFILE=\"/etc/openshift/server_priv.pem\"
AUTH_PRIVKEYPASS=\"\"
AUTH_PUBKEYFILE=\"/etc/openshift/server_pub.pem\"
AUTH_RSYNC_KEY_FILE=\"/etc/openshift/rsync_id_rsa\"
AUTH_SCOPE_TIMEOUTS=\"session=1.days|7.days, *=1.months|6.months\"
ENABLE_MAINTENANCE_MODE=\"false\"
MAINTENANCE_NOTIFICATION_FILE=\"/etc/openshift/outage_notification.txt\"
DOWNLOAD_CARTRIDGES_ENABLED=\"false\" 
"

(* Variable: new_conf *) 
let new_conf = "CLOUD_DOMAIN=\"rhcloud.com\"
VALID_GEAR_SIZES=\"small,medium\"
DEFAULT_MAX_GEARS=\"100\"
DEFAULT_GEAR_CAPABILITIES=\"small\"
DEFAULT_GEAR_SIZE=\"small\"
MONGO_HOST_PORT=\"localhost:27017\"
MONGO_USER=\"openshift\"
MONGO_PASSWORD=\"mooo\"
MONGO_DB=\"openshift_broker_dev\"
MONGO_SSL=\"false\"
ENABLE_USAGE_TRACKING_DATASTORE=\"false\"
ENABLE_USAGE_TRACKING_AUDIT_LOG=\"false\"
USAGE_TRACKING_AUDIT_LOG_FILE=\"/var/log/openshift/broker/usage.log\"
ENABLE_ANALYTICS=\"false\"
ENABLE_USER_ACTION_LOG=\"true\"
USER_ACTION_LOG_FILE=\"/var/log/openshift/broker/user_action.log\"
AUTH_PRIVKEYFILE=\"/etc/openshift/server_priv.pem\"
AUTH_PRIVKEYPASS=\"\"
AUTH_PUBKEYFILE=\"/etc/openshift/server_pub.pem\"
AUTH_RSYNC_KEY_FILE=\"/etc/openshift/rsync_id_rsa\"
AUTH_SCOPE_TIMEOUTS=\"session=1.days|7.days, *=1.months|6.months\"
ENABLE_MAINTENANCE_MODE=\"false\"
MAINTENANCE_NOTIFICATION_FILE=\"/etc/openshift/outage_notification.txt\"
DOWNLOAD_CARTRIDGES_ENABLED=\"false\" 
"

(* Test: OpenShift_Config.lns *)
test OpenShift_Config.lns get conf =
  { "CLOUD_DOMAIN" = "example.com" }
  { "VALID_GEAR_SIZES" = "small,medium" }
  { "DEFAULT_MAX_GEARS" = "100" }
  { "DEFAULT_GEAR_CAPABILITIES" = "small" }
  { "DEFAULT_GEAR_SIZE" = "small" }
  { "MONGO_HOST_PORT" = "localhost:27017" }
  { "MONGO_USER" = "openshift" }
  { "MONGO_PASSWORD" = "mooo" }
  { "MONGO_DB" = "openshift_broker_dev" }
  { "MONGO_SSL" = "false" }
  { "ENABLE_USAGE_TRACKING_DATASTORE" = "false" }
  { "ENABLE_USAGE_TRACKING_AUDIT_LOG" = "false" }
  { "USAGE_TRACKING_AUDIT_LOG_FILE" = "/var/log/openshift/broker/usage.log" }
  { "ENABLE_ANALYTICS" = "false" }
  { "ENABLE_USER_ACTION_LOG" = "true" }
  { "USER_ACTION_LOG_FILE" = "/var/log/openshift/broker/user_action.log" }
  { "AUTH_PRIVKEYFILE" = "/etc/openshift/server_priv.pem" }
  { "AUTH_PRIVKEYPASS" }
  { "AUTH_PUBKEYFILE" = "/etc/openshift/server_pub.pem" }
  { "AUTH_RSYNC_KEY_FILE" = "/etc/openshift/rsync_id_rsa" }
  { "AUTH_SCOPE_TIMEOUTS" = "session=1.days|7.days, *=1.months|6.months" }
  { "ENABLE_MAINTENANCE_MODE" = "false" }
  { "MAINTENANCE_NOTIFICATION_FILE" = "/etc/openshift/outage_notification.txt" }
  { "DOWNLOAD_CARTRIDGES_ENABLED" = "false" }

(* Test: OpenShift_Config.lns
 * Second get test against OpenShift configs
*) 
test OpenShift_Config.lns get "MONGO_SSL=\"false\"\n" =
  { "MONGO_SSL" = "false" }

(* Test: OpenShift_Config.lns
 * Put test changing CLOUD_DOMAIN to rhcloud.com
*) 
test OpenShift_Config.lns put conf after set "CLOUD_DOMAIN" "rhcloud.com"
  = new_conf

(* vim: set ts=4  expandtab  sw=4: *)
