(* Squid module for Augeas
   Author: Free Ekanayaka <free@64studio.com>

   Reference: the self-documented default squid.conf file

*)

module Squid =
  autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol         = Util.eol
let spc         = Util.del_ws_spc

let word        =  /[A-Za-z0-9!_.-]+(\[[0-9]+\])?/
let sto_to_spc  = store /[^# \t\n]+/

let comment     = Spacevars.comment
let value (kw:string)
                = [ spc . label kw . sto_to_spc ]
let parameters  = [ label "parameters"
                   . counter "parameters"
                   . [ spc . seq "parameters" . sto_to_spc ]+ ]

(************************************************************************
 *                          SPACEVARS SETTINGS
 *************************************************************************)

let entry_re    = "authenticate_cache_garbage_interval"
                | "access_log"
                | "acl_uses_indirect_client"
                | "allow_underscore"
                | "always_direct"
                | "announce_file"
                | "announce_host"
                | "announce_period"
                | "announce_port"
                | "append_domain"
                | "as_whois_server"
                | "authenticate_ip_ttl"
                | "authenticate_ttl"
                | "balance_on_multiple_ip"
                | "broken_posts"
                | "broken_vary_encoding"
                | "buffered_logs"
                | "cache"
                | "cache_dir"
                | "cache_dns_program"
                | "cache_effective_group"
                | "cache_effective_user"
                | "cache_log"
                | "cache_mem"
                | "cache_mgr"
                | "cache_peer"
                | "cache_peer_access"
                | "cache_peer_domain"
                | "cache_replacement_policy"
                | "cache_store_log"
                | "cache_swap_high"
                | "cache_swap_low"
                | "cache_swap_state"
                | "cache_vary"
                | "cachemgr_passwd"
                | "check_hostnames"
                | "chroot"
                | "client_db"
                | "client_lifetime"
                | "client_netmask"
                | "client_persistent_connections"
                | "collapsed_forwarding"
                | "connect_timeout"
                | "coredump_dir"
                | "dead_peer_timeout"
                | "debug_options"
                | "delay_access"
                | "delay_class"
                | "delay_initial_bucket_level"
                | "delay_parameters"
                | "delay_pool_uses_indirect_client"
                | "delay_pools"
                | "deny_info"
                | "detect_broken_pconn"
                | "digest_bits_per_entry"
                | "digest_generation"
                | "digest_rebuild_chunk_percentage"
                | "digest_rebuild_period"
                | "digest_rewrite_period"
                | "digest_swapout_chunk_size"
                | "diskd_program"
                | "dns_children"
                | "dns_defnames"
                | "dns_nameservers"
                | "dns_retransmit_interval"
                | "dns_testnames"
                | "dns_timeout"
                | "emulate_httpd_log"
                | "err_html_text"
                | "error_directory"
                | "error_map"
                | "extension_methods"
                | "external_acl_type"
                | "follow_x_forwarded_for"
                | "forward_log"
                | "forward_timeout"
                | "forwarded_for"
                | "fqdncache_size"
                | "ftp_list_width"
                | "ftp_passive"
                | "ftp_sanitycheck"
                | "ftp_telnet_protocol"
                | "ftp_user"
                | "global_internal_static"
                | "half_closed_clients"
                | "header_access"
                | "header_replace"
                | "hierarchy_stoplist"
                | "high_memory_warning"
                | "high_page_fault_warning"
                | "high_response_time_warning"
                | "hostname_aliases"
                | "hosts_file"
                | "htcp_access"
                | "htcp_clr_access"
                | "htcp_port"
                | "http_access2"
                | "http_port"
                | "http_reply_access"
                | "httpd_accel_no_pmtu_disc"
                | "httpd_suppress_version_string"
                | "https_port"
                | "icon_directory"
                | "icp_access"
                | "icp_hit_stale"
                | "icp_port"
                | "icp_query_timeout"
                | "ident_lookup_access"
                | "ident_timeout"
                | "ie_refresh"
                | "ignore_unknown_nameservers"
                | "incoming_dns_average"
                | "incoming_http_average"
                | "incoming_icp_average"
                | "ipcache_high"
                | "ipcache_low"
                | "ipcache_size"
                | "location_rewrite_access"
                | "location_rewrite_children"
                | "location_rewrite_concurrency"
                | "location_rewrite_program"
                | "log_access"
                | "log_fqdn"
                | "log_icp_queries"
                | "log_ip_on_direct"
                | "log_mime_hdrs"
                | "log_uses_indirect_client"
                | "logfile_rotate"
                | "logformat"
                | "mail_from"
                | "mail_program"
                | "max_open_disk_fds"
                | "maximum_icp_query_timeout"
                | "maximum_object_size"
                | "maximum_object_size_in_memory"
                | "maximum_single_addr_tries"
                | "mcast_groups"
                | "mcast_icp_query_timeout"
                | "mcast_miss_addr"
                | "mcast_miss_encode_key"
                | "mcast_miss_port"
                | "mcast_miss_ttl"
                | "memory_pools"
                | "memory_pools_limit"
                | "memory_replacement_policy"
                | "mime_table"
                | "min_dns_poll_cnt"
                | "min_http_poll_cnt"
                | "min_icp_poll_cnt"
                | "minimum_direct_hops"
                | "minimum_direct_rtt"
                | "minimum_expiry_time"
                | "minimum_object_size"
                | "miss_access"
                | "negative_dns_ttl"
                | "negative_ttl"
                | "neighbor_type_domain"
                | "netdb_high"
                | "netdb_low"
                | "netdb_ping_period"
                | "never_direct"
                | "nonhierarchical_direct"
                | "offline_mode"
                | "pconn_timeout"
                | "peer_connect_timeout"
                | "persistent_connection_after_error"
                | "persistent_request_timeout"
                | "pid_filename"
                | "pinger_program"
                | "pipeline_prefetch"
                | "positive_dns_ttl"
                | "prefer_direct"
                | "query_icmp"
                | "quick_abort_max"
                | "quick_abort_min"
                | "quick_abort_pct"
                | "range_offset_limit"
                | "read_ahead_gap"
                | "read_timeout"
                | "redirector_bypass"
                | "referer_log"
                | "refresh_pattern"
                | "refresh_stale_hit"
                | "relaxed_header_parser"
                | "reload_into_ims"
                | "reply_body_max_size"
                | "reply_header_max_size"
                | "request_body_max_size"
                | "request_entities"
                | "request_header_max_size"
                | "request_timeout"
                | "retry_on_error"
                | "server_persistent_connections"
                | "short_icon_urls"
                | "shutdown_lifetime"
                | "sleep_after_fork"
                | "snmp_access"
                | "snmp_incoming_address"
                | "snmp_outgoing_address"
                | "snmp_port"
                | "ssl_engine"
                | "ssl_unclean_shutdown"
                | "sslpassword_program"
                | "sslproxy_cafile"
                | "sslproxy_capath"
                | "sslproxy_cipher"
                | "sslproxy_client_certificate"
                | "sslproxy_client_key"
                | "sslproxy_flags"
                | "sslproxy_options"
                | "sslproxy_version"
                | "store_avg_object_size"
                | "store_dir_select_algorithm"
                | "store_objects_per_bucket"
                | "strip_query_terms"
                | "tcp_outgoing_address"
                | "tcp_outgoing_tos"
                | "tcp_recv_bufsize"
                | "test_reachability"
                | "udp_incoming_address"
                | "udp_outgoing_address"
                | "umask"
                | "unique_hostname"
                | "unlinkd_program"
                | "uri_whitespace"
                | "url_rewrite_access"
                | "url_rewrite_children"
                | "url_rewrite_concurrency"
                | "url_rewrite_host_header"
                | "url_rewrite_program"
                | "useragent_log"
                | "vary_ignore_expire"
                | "via"
                | "visible_hostname"
                | "wccp2_address"
                | "wccp2_assignment_method"
                | "wccp2_forwarding_method"
                | "wccp2_rebuild_wait"
                | "wccp2_return_method"
                | "wccp2_router"
                | "wccp2_service"
                | "wccp2_service_info"
                | "wccp2_weight"
                | "wccp_address"
                | "wccp_router"
                | "wccp_version"

let entry       = Spacevars.entry entry_re

(************************************************************************
 *                                AUTH
 *************************************************************************)

let auth_re     = "auth_param"
let auth        = [ key "auth_param"
                  . value "scheme"
                  . value "parameter"
                  . (value "setting") ?
                  . (eol|comment) ]

(************************************************************************
 *                                ACL
 *************************************************************************)

let acl_re     = "acl"
let acl        = [ key acl_re . spc
                 . [ key word
                   . value "type"
                   . value "setting"
                   . parameters?
                   . (eol|comment) ] ]

(************************************************************************
 *                             HTTP ACCESS
 *************************************************************************)

let http_access_re
               = "http_access"
let http_access
               = [ key http_access_re
                 . spc
                 . [ key /allow|deny/
                   . spc
                   . sto_to_spc
                   . parameters? ]
                 . (eol|comment) ]

(************************************************************************
 *                               LENS
 *************************************************************************)

let lns         = Spacevars.lns (entry|auth|acl|http_access)

let filter      = Util.stdexcl
                . incl "/etc/squid/squid.conf"

let xfm         = transform lns filter
