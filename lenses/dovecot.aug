(*
 * Module: Dovecot
 *     Parses dovecot configuration files
 *
 *  Copyright (c) 2013 Pluron, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * TODO: Support for multiline queries like in dict-sql.conf
 * TODO: Regular expression or function for keys to make entry less strict
 *
 *)

module Dovecot =

   autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol       = Util.eol
let comment   = Util.comment
let empty     = Util.empty
let word      = Rx.word
let indent    = Util.indent
let eq        = del /[ \t]*=/ " ="

let block_open        = del /[ \t]*\{/ "{"
let block_close       = del /\}/ "}"
let command_start     = Util.del_str "!"

(************************************************************************
 *                               ENTRIES
 *************************************************************************)

let any = Rx.no_spaces
let value = any . (Rx.space . any)* 

let keys = 
    "listen" | 
    "base_dir" | 
    "instance_name" | 
    "login_greeting" | 
    "login_trusted_networks" | 
    "login_access_sockets" | 
    "verbose_proctitle" | 
    "shutdown_clients" | 
    "doveadm_worker_count" | 
    "doveadm_socket_path" | 
    "import_environment" | 
    "protocols" | 
    "shutdown_clients" | 
    "connect" |
    "driver" |
    "args" |

    (* ******************* ssl ********************** *)
    "ssl" |
    "ssl_listen" | 
    "ssl_cert" |
    "ssl_key" |
    "ssl_key_password" |
    "ssl_ca" |
    "ssl_verify_client_cert" |
    "ssl_cert_username_field" |
    "ssl_parameters_regenerate" |
    "ssl_cipher_list" |
    "ssl_disable" | 
    "verbose_ssl" | 

    "login_dir" | 
    "login_chroot" | 
    "login_user" | 
    "login_process_size" | 
    "login_process_per_connection" | 
    "login_processes_count" | 
    "login_max_processes_count" | 
    "login_max_connections" | 
    "login_log_format_elements" | 
    "login_log_format" | 

    (* **************** master*********************** *)
    "default_process_limit" |
    "default_client_limit" |
    "default_vsz_limit" |
    "default_login_user" |
    "default_internal_user" |

    (* ****************   mail  ********************* *)
    "mail_location" | 
    "mail_uid" |
    "mail_gid" |
    "mail_privileged_group" | 
    "mail_access_groups" | 
    "mail_full_filesystem_access" | 
    "mail_debug" | 
    "mail_log_prefix" | 
    "mail_log_max_lines_per_sec" | 
    "mmap_disable" | 
    "map_no_write" | 
    "dotlock_use_excl" | 
    "mail_fsync" |
    "mail_nfs_storage" |
    "mail_nfs_index" |
    "fsync_disable" | 
    "lock_method" | 
    "mail_temp_dir" |
    "mail_drop_priv_before_exec" | 
    "first_valid_uid" | 
    "last_valid_uid" | 
    "first_valid_gid" | 
    "last_valid_gid" | 
    "max_mail_processes" | 
    "mail_process_size" | 
    "mail_max_keyword_length" | 
    "valid_chroot_dirs" | 
    "mail_chroot" | 
    "mail_cache_fields" | 
    "mail_never_cache_fields" | 
    "mail_cache_min_mail_count" | 
    "mailbox_idle_check_interval" | 
    "mail_save_crlf" | 
    "maildir_stat_dirs" | 
    "maildir_copy_with_hardlinks" | 
    "maildir_copy_preserve_filename" | 
    "auth_socket_path" |
    "mail_plugin_dir" |
    "mail_plugins" |
    "mail_cache_min_mail_count" |
    "mbox_read_locks" | 
    "mbox_write_locks" | 
    "mbox_lock_timeout" | 
    "mbox_dotlock_change_timeout" | 
    "mbox_dirty_syncs" | 
    "mbox_very_dirty_syncs" | 
    "mbox_lazy_writes" | 
    "mbox_min_index_size" | 
    "mdbox_rotate_size" |
    "mdbox_rotate_interval" |
    "mdbox_preallocate_space" |
    "dbox_rotate_size" | 
    "dbox_rotate_min_size" | 
    "dbox_rotate_days" | 
    "maildir_very_dirty_syncs" |
    "mail_attachment_dir" |
    "mail_attachment_min_size" |
    "mail_attachment_fs" |
    "mail_attachment_hash" |

    (* **************** logging********************** *)
    "log_path" | 
    "info_log_path" | 
    "debug_log_path" |
    "log_timestamp" | 
    "syslog_facility" | 
    "auth_verbose" |
    "auth_verbose_passwords" |
    "auth_debug" |
    "auth_debug_passwords" |
    "mail_debug" |
    "mail_log_events" |
    "mail_log_fields" |
    "log_timestamp" |
    "login_log_format_elements" |
    "login_log_format" |
    "mail_log_prefix" |
    "deliver_log_format" |

    (* **************** auth ************************ *)
    "disable_plaintext_auth" | 
    "auth_executable" | 
    "auth_process_size" | 
    "auth_cache_size" | 
    "auth_cache_ttl" | 
    "auth_cache_negative_ttl" |
    "auth_realms" | 
    "auth_default_realm" | 
    "auth_username_chars" | 
    "auth_username_translation" | 
    "auth_username_format" | 
    "auth_master_user_separator" | 
    "auth_anonymous_username" | 
    "auth_verbose" | 
    "auth_debug" | 
    "auth_debug_passwords" | 
    "auth_worker_max_count" | 
    "auth_gssapi_hostname" | 
    "auth_krb5_keytab" |
    "auth_use_winbind" |
    "auth_winbind_helper_path" |
    "auth_failure_delay" |
    "auth_ssl_require_client_cert" |
    "auth_ssl_username_from_cert" |
    "auth_mechanisms" |
    "deny" |
    "master" |
    "pass" |

    (* ****************** lda *********************** *)
    "postmaster_address" |
    "hostname" |
    "quota_full_tempfail" |
    "sendmail_path" |
    "submission_host" |
    "rejection_subject" |
    "rejection_reason" |
    "recipient_delimiter" |
    "lda_original_recipient_header" |
    "lda_mailbox_autocreate" |
    "lda_mailbox_autosubscribe" |

    (* **************** dict block ****************** *)
    "quota" |
    "expire" |

    (* **************** namespace block************** *)
    "type" |
    "separator" |
    "prefix" |
    "location" |
    "inbox" |
    "hidden" |
    "list" |
    "subscriptions" |

    (* **************** map block ******************* *)
    "pattern" |
    "table" |
    "username_field" |
    "value_field" |
    "username" |
    "mailbox" |
    
    (* **************** director ******************** *)
    "director_servers" |
    "director_mail_servers" |
    "director_user_expire" |
    "director_doveadm_port" |

    (* **************** services ******************** *)
    "service_count" |
    "process_min_avail" |
    "vsz_limit" |
    "process_limit" |
    "mail_plugins" |

    (* **************** listeners ******************* *)
    "mode" |
    "address" |
    "port" |
    "user" |
    "group" |
    "executable" |
    "auth_socket_path" |

    (* **************** plugins ******************** *)
    "acl" |
    "acl_shared_dict" |
    "quota" |
    "quota2" |
    "quota_rule" |
    "quota_rule2" |
    "quota2_rule" |
    "quota_warning" |
    "quota_warning2"


let commands  = ("include" | "include_try")
let block_names = ("dict" | "userdb" | "passdb" | "protocol" | "service" | "plugin" | "namespace" | "map" )
let nested_block_names =  ( "fields" | "unix_listener" | "fifo_listener" | "inet_listener" )

let entry = [ indent . key keys. eq . (Sep.opt_space . store value)? . eol ]
let command = [ command_start . key commands . Sep.space . store Rx.fspath . eol ]

let block_args   = Sep.space . store any

let nested_block =
    [ indent . key nested_block_names . block_args? . block_open . eol
    . (entry | empty | comment)*
    . indent . block_close . eol ]

let block = 
    [ indent . key block_names . block_args? . block_open . eol
    . (entry | empty | comment | nested_block )*
    . indent . block_close . eol ]


(************************************************************************
 *                                LENS
 *************************************************************************)

let lns = (comment|empty|entry|command|block)*

let filter     = incl "/etc/dovecot/dovecot.conf"
               . (incl "/etc/dovecot/conf.d/*.conf")
               . Util.stdexcl

let xfm        = transform lns filter
