(*
Module: Test_Dovecot
  Provides unit tests and examples for the <Dovecot> lens.
*)

module Test_Dovecot =

(* *********************** /etc/dovecot.conf ******************************** *)

let dovecot_conf = "# Dovecot configuration file

# If you're in a hurry, see http://wiki2.dovecot.org/QuickConfiguration

# Default values are shown for each setting, it's not required to uncomment
# those. These are exceptions to this though: No sections (e.g. namespace {})
# or plugin settings are added by default, they're listed only as examples.
# Paths are also just examples with the real defaults being based on configure
# options. The paths listed here are for configure --prefix=/usr
# --sysconfdir=/etc --localstatedir=/var

# include_try command
!include_try /usr/share/dovecot/protocols.d/*.protocol

# Wildcard, comma and space in value
listen = *, ::

# Filesystem path in value
base_dir = /var/run/dovecot/
instance_name = dovecot

# Space and dot in value
login_greeting = Dovecot ready.

# Empty values
login_trusted_networks =
login_access_sockets = 

# Simple values
verbose_proctitle = no
shutdown_clients = yes

# Number in value
doveadm_worker_count = 0
# Dash in value
doveadm_socket_path = doveadm-server

import_environment = TZ

##
## Comment
##

# Simple commented dict block
dict {
  #quota = mysql:/etc/dovecot/dovecot-dict-sql.conf.ext
  #expire = sqlite:/etc/dovecot/dovecot-dict-sql.conf.ext
}

# Simple uncommented dict block
dict {
  quota = mysql:/etc/dovecot/dovecot-dict-sql.conf.ext
  expire = sqlite:/etc/dovecot/dovecot-dict-sql.conf.ext
}

# Include command
!include conf.d/*.conf

# Include_try command
!include_try local.conf

"

test Dovecot.lns get dovecot_conf =  
  { "#comment" = "Dovecot configuration file" }
  {  }
  { "#comment" = "If you're in a hurry, see http://wiki2.dovecot.org/QuickConfiguration" }
  {  }
  { "#comment" = "Default values are shown for each setting, it's not required to uncomment" }
  { "#comment" = "those. These are exceptions to this though: No sections (e.g. namespace {})" }
  { "#comment" = "or plugin settings are added by default, they're listed only as examples." }
  { "#comment" = "Paths are also just examples with the real defaults being based on configure" }
  { "#comment" = "options. The paths listed here are for configure --prefix=/usr" }
  { "#comment" = "--sysconfdir=/etc --localstatedir=/var" }
  {  }
  { "#comment" = "include_try command" }
  { "include_try" = "/usr/share/dovecot/protocols.d/*.protocol" }
  {  }
  { "#comment" = "Wildcard, comma and space in value" }
  { "listen" = "*, ::" }
  {  }
  { "#comment" = "Filesystem path in value" }
  { "base_dir" = "/var/run/dovecot/" }
  { "instance_name" = "dovecot" }
  {  }
  { "#comment" = "Space and dot in value" }
  { "login_greeting" = "Dovecot ready." }
  {  }
  { "#comment" = "Empty values" }
  { "login_trusted_networks" }
  { "login_access_sockets" }
  {  }
  { "#comment" = "Simple values" }
  { "verbose_proctitle" = "no" }
  { "shutdown_clients" = "yes" }
  {  }
  { "#comment" = "Number in value" }
  { "doveadm_worker_count" = "0" }
  { "#comment" = "Dash in value" }
  { "doveadm_socket_path" = "doveadm-server" }
  {  }
  { "import_environment" = "TZ" }
  {  }
  { "#comment" = "#" }
  { "#comment" = "# Comment" }
  { "#comment" = "#" }
  {  }
  { "#comment" = "Simple commented dict block" }
  { "dict"
    { "#comment" = "quota = mysql:/etc/dovecot/dovecot-dict-sql.conf.ext" }
    { "#comment" = "expire = sqlite:/etc/dovecot/dovecot-dict-sql.conf.ext" }
  }
  {  }
  { "#comment" = "Simple uncommented dict block" }
  { "dict"
    { "quota" = "mysql:/etc/dovecot/dovecot-dict-sql.conf.ext" }
    { "expire" = "sqlite:/etc/dovecot/dovecot-dict-sql.conf.ext" }
  }
  {  }
  { "#comment" = "Include command" }
  { "include" = "conf.d/*.conf" }
  {  }
  { "#comment" = "Include_try command" }
  { "include_try" = "local.conf" }
  {  }



(* *********************************** dict ********************************* *)

let dovecot_dict_sql_conf = "connect = host=localhost dbname=mails user=testuser password=pass

# CREATE TABLE quota (
#   username varchar(100) not null,
#   bytes bigint not null default 0,
#   messages integer not null default 0,
#   primary key (username)
# );

map {
  pattern = priv/quota/storage
  table = quota
  username_field = username
  value_field = bytes
}
map {
  pattern = priv/quota/messages
  table = quota
  username_field = username
  value_field = messages
}

# CREATE TABLE expires (
#   username varchar(100) not null,
#   mailbox varchar(255) not null,
#   expire_stamp integer not null,
#   primary key (username, mailbox)
# );

map {
  pattern = shared/expire/$user/$mailbox
  table = expires
  value_field = expire_stamp

  fields {
    username = $user
    mailbox = $mailbox
  }
}
"

test Dovecot.lns get dovecot_dict_sql_conf =
  { "connect" = "host=localhost dbname=mails user=testuser password=pass" }
  {  }
  { "#comment" = "CREATE TABLE quota (" }
  { "#comment" = "username varchar(100) not null," }
  { "#comment" = "bytes bigint not null default 0," }
  { "#comment" = "messages integer not null default 0," }
  { "#comment" = "primary key (username)" }
  { "#comment" = ");" }
  {  }
  { "map"
    { "pattern" = "priv/quota/storage" }
    { "table" = "quota" }
    { "username_field" = "username" }
    { "value_field" = "bytes" }
  }
  { "map"
    { "pattern" = "priv/quota/messages" }
    { "table" = "quota" }
    { "username_field" = "username" }
    { "value_field" = "messages" }
  }
  {  }
  { "#comment" = "CREATE TABLE expires (" }
  { "#comment" = "username varchar(100) not null," }
  { "#comment" = "mailbox varchar(255) not null," }
  { "#comment" = "expire_stamp integer not null," }
  { "#comment" = "primary key (username, mailbox)" }
  { "#comment" = ");" }
  {  }
  { "map"
    { "pattern" = "shared/expire/$user/$mailbox" }
    { "table" = "expires" }
    { "value_field" = "expire_stamp" }
    {  }
    { "fields"
      { "username" = "$user" }
      { "mailbox" = "$mailbox" }
    }
  }

(* ********************************** auth ********************************** *)

let auth_conf = "## Authentication processes

disable_plaintext_auth = yes
auth_cache_size = 0
auth_cache_ttl = 1 hour
auth_cache_negative_ttl = 1 hour
auth_realms =
auth_default_realm = 
auth_username_chars = abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890.-_@
auth_username_translation =
auth_username_format =
auth_master_user_separator =
auth_anonymous_username = anonymous
auth_worker_max_count = 30
auth_gssapi_hostname =
auth_krb5_keytab = 
auth_use_winbind = no
auth_winbind_helper_path = /usr/bin/ntlm_auth
auth_failure_delay = 2 secs
auth_ssl_require_client_cert = no
auth_ssl_username_from_cert = no
auth_mechanisms = plain

!include auth-deny.conf.ext
!include auth-master.conf.ext
!include auth-system.conf.ext
!include auth-sql.conf.ext
!include auth-ldap.conf.ext
!include auth-passwdfile.conf.ext
!include auth-checkpassword.conf.ext
!include auth-vpopmail.conf.ext
!include auth-static.conf.ext

passdb {
  driver = passwd-file
  deny = yes

  # File contains a list of usernames, one per line
  args = /etc/dovecot/deny-users
}

passdb {
  driver = passwd-file
  master = yes
  args = /etc/dovecot/master-users

  # Unless you're using PAM, you probably still want the destination user to
  # be looked up from passdb that it really exists. pass=yes does that.
  pass = yes
}

userdb {
  driver = passwd-file
  args = username_format=%u /etc/dovecot/users
}
"

test Dovecot.lns get auth_conf =
  { "#comment" = "# Authentication processes" }
  {  }
  { "disable_plaintext_auth" = "yes" }
  { "auth_cache_size" = "0" }
  { "auth_cache_ttl" = "1 hour" }
  { "auth_cache_negative_ttl" = "1 hour" }
  { "auth_realms" }
  { "auth_default_realm" }
  { "auth_username_chars" = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890.-_@" }
  { "auth_username_translation" }
  { "auth_username_format" }
  { "auth_master_user_separator" }
  { "auth_anonymous_username" = "anonymous" }
  { "auth_worker_max_count" = "30" }
  { "auth_gssapi_hostname" }
  { "auth_krb5_keytab" }
  { "auth_use_winbind" = "no" }
  { "auth_winbind_helper_path" = "/usr/bin/ntlm_auth" }
  { "auth_failure_delay" = "2 secs" }
  { "auth_ssl_require_client_cert" = "no" }
  { "auth_ssl_username_from_cert" = "no" }
  { "auth_mechanisms" = "plain" }
  {  }
  { "include" = "auth-deny.conf.ext" }
  { "include" = "auth-master.conf.ext" }
  { "include" = "auth-system.conf.ext" }
  { "include" = "auth-sql.conf.ext" }
  { "include" = "auth-ldap.conf.ext" }
  { "include" = "auth-passwdfile.conf.ext" }
  { "include" = "auth-checkpassword.conf.ext" }
  { "include" = "auth-vpopmail.conf.ext" }
  { "include" = "auth-static.conf.ext" }
  {  }
  { "passdb"
    { "driver" = "passwd-file" }
    { "deny" = "yes" }
    {  }
    { "#comment" = "File contains a list of usernames, one per line" }
    { "args" = "/etc/dovecot/deny-users" }
  }
  {  }
  { "passdb"
    { "driver" = "passwd-file" }
    { "master" = "yes" }
    { "args" = "/etc/dovecot/master-users" }
    {  }
    { "#comment" = "Unless you're using PAM, you probably still want the destination user to" }
    { "#comment" = "be looked up from passdb that it really exists. pass=yes does that." }
    { "pass" = "yes" }
  }
  {  }
  { "userdb"
    { "driver" = "passwd-file" }
    { "args" = "username_format=%u /etc/dovecot/users" }
  }

(* ******************************** director ******************************** *)

let director_conf = "## Director-specific settings.
director_servers = 
director_mail_servers = 
director_user_expire = 15 min
director_doveadm_port = 0

service director {
  unix_listener login/director {
    mode = 0666
  }
  fifo_listener login/proxy-notify {
    mode = 0666
  }
  unix_listener director-userdb {
    #mode = 0600
  }
  inet_listener {
    port = 
  }
}

service imap-login {
  executable = imap-login director
}
service pop3-login {
  executable = pop3-login director
}
protocol lmtp {
  auth_socket_path = director-userdb
}
"

test Dovecot.lns get director_conf =
  { "#comment" = "# Director-specific settings." }
  { "director_servers" }
  { "director_mail_servers" }
  { "director_user_expire" = "15 min" }
  { "director_doveadm_port" = "0" }
  {  }
  { "service" = "director"
    { "unix_listener" = "login/director"
      { "mode" = "0666" }
    }
    { "fifo_listener" = "login/proxy-notify"
      { "mode" = "0666" }
    }
    { "unix_listener" = "director-userdb"
      { "#comment" = "mode = 0600" }
    }
    { "inet_listener"
      { "port" }
    }
  }
  {  }
  { "service" = "imap-login"
    { "executable" = "imap-login director" }
  }
  { "service" = "pop3-login"
    { "executable" = "pop3-login director" }
  }
  { "protocol" = "lmtp"
    { "auth_socket_path" = "director-userdb" }
  }

(* ********************************* logging ******************************** *)

let logging_conf = "## Log destination.
log_path = syslog
info_log_path = 
debug_log_path = 
syslog_facility = mail
auth_verbose = no
auth_verbose_passwords = no
auth_debug = no
auth_debug_passwords = no
mail_debug = no
verbose_ssl = no

plugin {
  mail_log_events = delete undelete expunge copy mailbox_delete mailbox_rename
  mail_log_fields = uid box msgid size
}

log_timestamp = \"%b %d %H:%M:%S \"
login_log_format_elements = user=<%u> method=%m rip=%r lip=%l mpid=%e %c
login_log_format = %$: %s
mail_log_prefix = \"%s(%u): \"
deliver_log_format = msgid=%m: %$
"

test Dovecot.lns get logging_conf =
  { "#comment" = "# Log destination." }
  { "log_path" = "syslog" }
  { "info_log_path" }
  { "debug_log_path" }
  { "syslog_facility" = "mail" }
  { "auth_verbose" = "no" }
  { "auth_verbose_passwords" = "no" }
  { "auth_debug" = "no" }
  { "auth_debug_passwords" = "no" }
  { "mail_debug" = "no" }
  { "verbose_ssl" = "no" }
  {  }
  { "plugin"
    { "mail_log_events" = "delete undelete expunge copy mailbox_delete mailbox_rename" }
    { "mail_log_fields" = "uid box msgid size" }
  }
  {  }
  { "log_timestamp" = "\"%b %d %H:%M:%S \"" }
  { "login_log_format_elements" = "user=<%u> method=%m rip=%r lip=%l mpid=%e %c" }
  { "login_log_format" = "%$: %s" }
  { "mail_log_prefix" = "\"%s(%u): \"" }
  { "deliver_log_format" = "msgid=%m: %$" }


(* ********************************** mail ********************************** *)

let mail_conf = "## Mailbox locations and namespaces
mail_location = 
namespace {
  type = private
  separator = 
  prefix = 
  location =
  inbox = no
  hidden = no
  list = yes
  subscriptions = yes
  mailbox \"Sent Messages\" {
    special_use = \Sent
  }
}

# Example shared namespace configuration
namespace {
  type = shared
  separator = /
  prefix = shared/%%u/
  location = maildir:%%h/Maildir:INDEX=~/Maildir/shared/%%u
  subscriptions = no
  list = children
}

mail_uid =
mail_gid =
mail_privileged_group =
mail_access_groups =
mail_full_filesystem_access = no
mmap_disable = no
dotlock_use_excl = yes
mail_fsync = optimized
mail_nfs_storage = no
mail_nfs_index = no
lock_method = fcntl
mail_temp_dir = /tmp
first_valid_uid = 500
last_valid_uid = 0
first_valid_gid = 1
last_valid_gid = 0
mail_max_keyword_length = 50
valid_chroot_dirs = 
mail_chroot = 
auth_socket_path = /var/run/dovecot/auth-userdb
mail_plugin_dir = /usr/lib/dovecot/modules
mail_plugins = 
mail_cache_min_mail_count = 0
mailbox_idle_check_interval = 30 secs
mail_save_crlf = no
maildir_stat_dirs = no
maildir_copy_with_hardlinks = yes
maildir_very_dirty_syncs = no
mbox_read_locks = fcntl
mbox_write_locks = dotlock fcntl
mbox_lock_timeout = 5 mins
mbox_dotlock_change_timeout = 2 mins
mbox_dirty_syncs = yes
mbox_very_dirty_syncs = no
mbox_lazy_writes = yes
mbox_min_index_size = 0
mdbox_rotate_size = 2M
mdbox_rotate_interval = 0
mdbox_preallocate_space = no
mail_attachment_dir =
mail_attachment_min_size = 128k
mail_attachment_fs = sis posix
mail_attachment_hash = %{sha1}
"
test Dovecot.lns get mail_conf =
  { "#comment" = "# Mailbox locations and namespaces" }
  { "mail_location" }
  { "namespace"
    { "type" = "private" }
    { "separator" }
    { "prefix" }
    { "location" }
    { "inbox" = "no" }
    { "hidden" = "no" }
    { "list" = "yes" }
    { "subscriptions" = "yes" }
    { "mailbox" = "Sent Messages" 
      { "special_use" = "\Sent" }
    }
  }
  {  }
  { "#comment" = "Example shared namespace configuration" }
  { "namespace"
    { "type" = "shared" }
    { "separator" = "/" }
    { "prefix" = "shared/%%u/" }
    { "location" = "maildir:%%h/Maildir:INDEX=~/Maildir/shared/%%u" }
    { "subscriptions" = "no" }
    { "list" = "children" }
  }
  {  }
  { "mail_uid" }
  { "mail_gid" }
  { "mail_privileged_group" }
  { "mail_access_groups" }
  { "mail_full_filesystem_access" = "no" }
  { "mmap_disable" = "no" }
  { "dotlock_use_excl" = "yes" }
  { "mail_fsync" = "optimized" }
  { "mail_nfs_storage" = "no" }
  { "mail_nfs_index" = "no" }
  { "lock_method" = "fcntl" }
  { "mail_temp_dir" = "/tmp" }
  { "first_valid_uid" = "500" }
  { "last_valid_uid" = "0" }
  { "first_valid_gid" = "1" }
  { "last_valid_gid" = "0" }
  { "mail_max_keyword_length" = "50" }
  { "valid_chroot_dirs" }
  { "mail_chroot" }
  { "auth_socket_path" = "/var/run/dovecot/auth-userdb" }
  { "mail_plugin_dir" = "/usr/lib/dovecot/modules" }
  { "mail_plugins" }
  { "mail_cache_min_mail_count" = "0" }
  { "mailbox_idle_check_interval" = "30 secs" }
  { "mail_save_crlf" = "no" }
  { "maildir_stat_dirs" = "no" }
  { "maildir_copy_with_hardlinks" = "yes" }
  { "maildir_very_dirty_syncs" = "no" }
  { "mbox_read_locks" = "fcntl" }
  { "mbox_write_locks" = "dotlock fcntl" }
  { "mbox_lock_timeout" = "5 mins" }
  { "mbox_dotlock_change_timeout" = "2 mins" }
  { "mbox_dirty_syncs" = "yes" }
  { "mbox_very_dirty_syncs" = "no" }
  { "mbox_lazy_writes" = "yes" }
  { "mbox_min_index_size" = "0" }
  { "mdbox_rotate_size" = "2M" }
  { "mdbox_rotate_interval" = "0" }
  { "mdbox_preallocate_space" = "no" }
  { "mail_attachment_dir" }
  { "mail_attachment_min_size" = "128k" }
  { "mail_attachment_fs" = "sis posix" }
  { "mail_attachment_hash" = "%{sha1}" }


(* ********************************* master ********************************* *)

let master_conf = "
default_process_limit = 100
default_client_limit = 1000
default_vsz_limit = 256M
default_login_user = dovenull
default_internal_user = dovecot

service imap-login {
  inet_listener imap {
    port = 143
  }
  inet_listener imaps {
    port = 993
    ssl = yes
  }
  service_count = 1
  process_min_avail = 0
  vsz_limit = 64M
}

service pop3-login {
  inet_listener pop3 {
    port = 110
  }
  inet_listener pop3s {
    port = 995
    ssl = yes
  }
}

service lmtp {
  unix_listener lmtp {
    mode = 0666
  }
  inet_listener lmtp {
    address =
    port = 
  }
}

service imap {
  vsz_limit = 256M
  process_limit = 1024
}

service auth {
  unix_listener auth-userdb {
    mode = 0600
    user = 
    group = 
  }
}

service auth-worker {
  user = root
}

service dict {
  unix_listener dict {
    mode = 0600
    user = 
    group = 
  }
}
"

test Dovecot.lns get master_conf =
  {  }
  { "default_process_limit" = "100" }
  { "default_client_limit" = "1000" }
  { "default_vsz_limit" = "256M" }
  { "default_login_user" = "dovenull" }
  { "default_internal_user" = "dovecot" }
  {  }
  { "service" = "imap-login"
    { "inet_listener" = "imap"
      { "port" = "143" }
    }
    { "inet_listener" = "imaps"
      { "port" = "993" }
      { "ssl" = "yes" }
    }
    { "service_count" = "1" }
    { "process_min_avail" = "0" }
    { "vsz_limit" = "64M" }
  }
  {  }
  { "service" = "pop3-login"
    { "inet_listener" = "pop3"
      { "port" = "110" }
    }
    { "inet_listener" = "pop3s"
      { "port" = "995" }
      { "ssl" = "yes" }
    }
  }
  {  }
  { "service" = "lmtp"
    { "unix_listener" = "lmtp"
      { "mode" = "0666" }
    }
    { "inet_listener" = "lmtp"
      { "address" }
      { "port" }
    }
  }
  {  }
  { "service" = "imap"
    { "vsz_limit" = "256M" }
    { "process_limit" = "1024" }
  }
  {  }
  { "service" = "auth"
    { "unix_listener" = "auth-userdb"
      { "mode" = "0600" }
      { "user" }
      { "group" }
    }
  }
  {  }
  { "service" = "auth-worker"
    { "user" = "root" }
  }
  {  }
  { "service" = "dict"
    { "unix_listener" = "dict"
      { "mode" = "0600" }
      { "user" }
      { "group" }
    }
  }

(* *********************************** ssl ********************************** *)

let ssl_conf = "## SSL settings
ssl = yes
ssl_cert = </etc/ssl/certs/dovecot.pem
ssl_key = </etc/ssl/private/dovecot.pem
ssl_key_password =
ssl_ca = 
ssl_verify_client_cert = no
ssl_cert_username_field = commonName
ssl_parameters_regenerate = 168
ssl_cipher_list = ALL:!LOW:!SSLv2:!EXP:!aNULL
"
test Dovecot.lns get ssl_conf =
  { "#comment" = "# SSL settings" }
  { "ssl" = "yes" }
  { "ssl_cert" = "</etc/ssl/certs/dovecot.pem" }
  { "ssl_key" = "</etc/ssl/private/dovecot.pem" }
  { "ssl_key_password" }
  { "ssl_ca" }
  { "ssl_verify_client_cert" = "no" }
  { "ssl_cert_username_field" = "commonName" }
  { "ssl_parameters_regenerate" = "168" }
  { "ssl_cipher_list" = "ALL:!LOW:!SSLv2:!EXP:!aNULL" }

(* ********************* /etc/dovecot/conf.d/15-lda.conf ******************** *)

let lda_conf = "## LDA specific settings (also used by LMTP)
postmaster_address =
hostname = 
quota_full_tempfail = no
sendmail_path = /usr/sbin/sendmail
submission_host =
rejection_subject = Rejected: %s
rejection_reason = Your message to <%t> was automatically rejected:%n%r
recipient_delimiter = +
lda_original_recipient_header =
lda_mailbox_autocreate = no
lda_mailbox_autosubscribe = no

protocol lda {
  mail_plugins = $mail_plugins
}
"
test Dovecot.lns get lda_conf =
  { "#comment" = "# LDA specific settings (also used by LMTP)" }
  { "postmaster_address" }
  { "hostname" }
  { "quota_full_tempfail" = "no" }
  { "sendmail_path" = "/usr/sbin/sendmail" }
  { "submission_host" }
  { "rejection_subject" = "Rejected: %s" }
  { "rejection_reason" = "Your message to <%t> was automatically rejected:%n%r" }
  { "recipient_delimiter" = "+" }
  { "lda_original_recipient_header" }
  { "lda_mailbox_autocreate" = "no" }
  { "lda_mailbox_autosubscribe" = "no" }
  {  }
  { "protocol" = "lda"
    { "mail_plugins" = "$mail_plugins" }
  }

(* *********************************** acl ********************************** *)

let acl_conf = "## Mailbox access control lists.
plugin {
  acl = vfile:/etc/dovecot/global-acls:cache_secs=300
}
plugin {
  acl_shared_dict = file:/var/lib/dovecot/shared-mailboxes
}
"

test Dovecot.lns get acl_conf =
  { "#comment" = "# Mailbox access control lists." }
  { "plugin"
    { "acl" = "vfile:/etc/dovecot/global-acls:cache_secs=300" }
  }
  { "plugin"
    { "acl_shared_dict" = "file:/var/lib/dovecot/shared-mailboxes" }
  }

(* ******************************** plugins ********************************* *)

let plugins_conf = "
plugin {
  quota_rule = *:storage=1G
  quota_rule2 = Trash:storage=+100M
}
plugin {
  quota_warning = storage=95%% quota-warning 95 %u
  quota_warning2 = storage=80%% quota-warning 80 %u
}
service quota-warning {
  executable = script /usr/local/bin/quota-warning.sh
  user = dovecot
  unix_listener quota-warning {
    user = vmail
  }
}
plugin {
  quota = dirsize:User quota
  quota = maildir:User quota
  quota = dict:User quota::proxy::quota
  quota = fs:User quota
}
plugin {
  quota = dict:user::proxy::quota
  quota2 = dict:domain:%d:proxy::quota_domain
  quota_rule = *:storage=102400
  quota2_rule = *:storage=1048576
}
plugin {
  acl = vfile:/etc/dovecot/global-acls:cache_secs=300
}
plugin {
  acl_shared_dict = file:/var/lib/dovecot/shared-mailboxes
}
"
test Dovecot.lns get plugins_conf =
  {  }
  { "plugin"
    { "quota_rule" = "*:storage=1G" }
    { "quota_rule2" = "Trash:storage=+100M" }
  }
  { "plugin"
    { "quota_warning" = "storage=95%% quota-warning 95 %u" }
    { "quota_warning2" = "storage=80%% quota-warning 80 %u" }
  }
  { "service" = "quota-warning"
    { "executable" = "script /usr/local/bin/quota-warning.sh" }
    { "user" = "dovecot" }
    { "unix_listener" = "quota-warning"
      { "user" = "vmail" }
    }
  }
  { "plugin"
    { "quota" = "dirsize:User quota" }
    { "quota" = "maildir:User quota" }
    { "quota" = "dict:User quota::proxy::quota" }
    { "quota" = "fs:User quota" }
  }
  { "plugin"
    { "quota" = "dict:user::proxy::quota" }
    { "quota2" = "dict:domain:%d:proxy::quota_domain" }
    { "quota_rule" = "*:storage=102400" }
    { "quota2_rule" = "*:storage=1048576" }
  }
  { "plugin"
    { "acl" = "vfile:/etc/dovecot/global-acls:cache_secs=300" }
  }
  { "plugin"
    { "acl_shared_dict" = "file:/var/lib/dovecot/shared-mailboxes" }
  }
