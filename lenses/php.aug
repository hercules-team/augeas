(* PHP module for Augeas                      *)
(* Author: Raphael Pinson <raphink@gmail.com> *)
(*                                            *)

module PHP =

    autoload xfm

    (* Define a special entry when spaces are allowed in values *)
    (* This is the case for error_reporting for example         *)
    let comment_nospace  = IniFile.comment_generic /(#|;)/
    let value_allowspace = del /[ \t]*/ " " . store /[^;# \t\n][^;#\n]*[^;# \t\n]|[^;# \t\n]/
    let entry_allowspace (kw:regexp) = [ key kw . IniFile.value_sepwithcolon . value_allowspace? . (comment_nospace|IniFile.eol) ]

    (* PHP is a standard INI file *)
    let setting = IniFile.entry "always_populate_raw_post_data"
                | IniFile.entry "asp_tags"
                | IniFile.entry "browscap"
                | IniFile.entry "child_terminate"
                | IniFile.entry "define_syslog_variables"
                | IniFile.entry "doc_root"
                | IniFile.entry "enable_dl"
                | IniFile.entry "engine"
                | IniFile.entry "expose_php"
                | IniFile.entry "extension_dir"
                | IniFile.entry "file_uploads"
		| IniFile.entry "gpc_order"
                | IniFile.entry "html_errors"
                | IniFile.entry "implicit_flush"
                | IniFile.entry "include_path"
                | IniFile.entry "last_modified"
                | IniFile.entry "memory_limit"
                | IniFile.entry "open_basedir"
                | IniFile.entry "post_max_size"
                | IniFile.entry "precision"
                | IniFile.entry "serialize_precision"
                | IniFile.entry "short_open_tag"
                | IniFile.entry "SMTP"
                | IniFile.entry "smtp_port"
                | IniFile.entry "track_errors"
                | IniFile.entry "unserialize_callback_func"
                | IniFile.entry "variables_order"
                | IniFile.entry "xbithack"
                | IniFile.entry "y2k_compliance"
                | IniFile.entry /apc\.(cache_by_default|enabled|filters|gc_ttl|mmap_file_mask|num_files_hint|optimization|shm_segments|shm_size|slam_defense|ttl|user_entries_hint|user_ttl)/
                | IniFile.entry /apd\.(dumpdir|statement_tracing)/
                | IniFile.entry /arg_separator\.(input|output)/
                | IniFile.entry /assert\.(active|bail|callback|quiet_eval|warning)/
                | IniFile.entry /bcmath\.(scale)/
                | IniFile.entry /blenc\.(key_file)/
                | IniFile.entry /com\.(allow_dcom|autoregister_casesensitive|autoregister_typelib|autoregister_verbose|code_page|typelib_file)/
                | IniFile.entry /crack\.(default_dictionary)/
                | IniFile.entry /daffodildb\.(default_host|default_password|default_socket|default_user|port)/
                | IniFile.entry /date\.(default_latitude|default_longitude|sunrise_zenith|sunset_zenith)/
                | IniFile.entry /dba\.(default_handler)/
                | IniFile.entry /dbx\.(colnames_case)/
                | IniFile.entry /exif\.(decode_jis_intel|decode_jis_motorola|decode_unicode_intel|decode_unicode_motorola|encode_jis|encode_unicode)/
                | IniFile.entry /fbsql\.(allow_persistent|autocommit|batchsize|default_database|default_database_password|default_host|default_password|default_user|generate_warnings|max_connections|max_links|max_persistent|max_results)/
                | IniFile.entry /highlight\.(bg|comment|default|html|keyword|string)/
                | IniFile.entry /hyperwave\.(allow_persistent|default_port)/
                | IniFile.entry /ibase\.(allow_persistent|dateformat|default_charset|default_db|default_password|default_user|max_links|max_persistent|timeformat|timestampformat)/
                | IniFile.entry /iconv\.(input_encoding|internal_encoding|output_encoding)/
                | IniFile.entry /ifx\.(allow_persistent|blobinfile|byteasvarchar|charasvarchar|default_host|default_password|default_user|max_links|max_persistent|nullformat|textasvarchar)/
                | IniFile.entry /ingres\.(allow_persistent|default_database|default_password|default_user|max_links|max_persistent)/
                | IniFile.entry /ircg\.(control_user|keep_alive_interval|max_format_message_sets|shared_mem_size|work_dir)/
                | IniFile.entry /ldap\.(max_links)/
                | IniFile.entry /mail\.(force_extra_parameters)/
                | IniFile.entry /mailparse\.(def_charset)/
                | IniFile.entry /maxdb\.(default_db|default_host|default_pw|default_user|long_readlen)/
                | IniFile.entry /mbstring\.(detect_order|encoding_translation|func_overload|http_input|http_output|internal_encoding|language|script_encoding|substitute_character)/
                | IniFile.entry /mcrypt\.(algorithms_dir|modes_dir)/
                | IniFile.entry /mime_magic\.(debug|magicfile)/
                | IniFile.entry /mssql\.(allow_persistent|batchsize|compatability_mode|connect_timeout|datetimeconvert|max_links|max_persistent|max_procs|min_error_severity|min_message_severity|secure_connection|textlimit|textsize|timeout)/
                | IniFile.entry /msql\.(allow_persistent|max_links|max_persistent)/
                | IniFile.entry /mysql\.(allow_persistent|connect_timeout|default_host|default_password|default_port|default_socket|default_user|max_links|max_persistent|trace_mode)/
                | IniFile.entry /mysqli\.(default_host|default_port|default_pw|default_socket|default_user|max_links|reconnect)/
                | IniFile.entry /namazu\.(debugmode|lang|loggingmode|sortmethod|sortorder)/
                | IniFile.entry /nsapi\.(read_timeout)/
                | IniFile.entry /odbc\.(allow_persistent|check_persistent|defaultbinmode|defaultlrl|default_db|default_pw|default_user|max_links|max_persistent)/
                | IniFile.entry /opendirectory\.(max_refs|separator)/
                | IniFile.entry /pdo\.(global_value)/
                | IniFile.entry /pfpro\.(defaulthost|defaultport|defaulttimeout|proxyaddress|proxylogon|proxypassword|proxyport)/
                | IniFile.entry /pgsql\.(allow_persistent|auto_reset_persistent|ignore_notice|log_notice|max_links|max_persistent)/
                | IniFile.entry /printer\.(default_printer)/
                | IniFile.entry /session\.(auto_start|bug_compat_42|bug_compat_warn|cache_expire|cache_limiter|cookie_(domain|httponly|lifetime|path|secure)|entropy_file|entropy_length|gc_divisor|gc_maxlifetime|gc_probability|hash_bits_per_character|hash_function|name|referer_check|save_handler|save_path|serialize_handler|use_cookies|use_only_cookies|use_trans_sid)/
                | IniFile.entry /session_pgsql\.(create_table|db|disable|failover_mode|gc_interval|keep_expired|sem_file_name|serializable|short_circuit|use_app_vars|vacuum_interval)/
                | IniFile.entry /simple_cvs\.(authMethod|compressionLevel|cvsRoot|host|moduleName|userName|workingDir)/
                | IniFile.entry /soap\.(wsdl_cache_dir|wsdl_cache_enabled|wsdl_cache_ttl)/
                | IniFile.entry /sql\.(safe_mode)/
                | IniFile.entry /sqlite\.(assoc_case)/
		| IniFile.entry /sybase\.(allow_persistent|max_(persistent|links)|interface_file|min_(error|message)_severity|compatability_mode)/
                | IniFile.entry /sybct\.(allow_persistent|deadlock_retry_count|hostname|login_timeout|max_links|max_persistent|min_client_severity|min_server_severity)/
                | IniFile.entry /tidy\.(clean_output|default_config)/
                | IniFile.entry /url_rewriter\.(tags)/
                | IniFile.entry /valkyrie\.(auto_validate|config_path)/
                | IniFile.entry /xmms\.(path|session)/
                | IniFile.entry /yaz\.(keepalive|log_file|max_links)/
                | IniFile.entry /zend\.(ze1_compatibility_mode)/
                | IniFile.entry /zlib\.(output_compression|output_compression_level|output_handler)/
                | IniFile.entry /allow_(call_time_pass_reference|url_fopen|url_include)/
                | IniFile.entry /auto_(append_file|detect_line_endings|globals_jit|prepend_file)/
                | IniFile.entry /default_(charset|mimetype|socket_timeout)/
		| IniFile.entry /display(_startup)?_errors/
                | IniFile.entry /disable_(classes|functions)/
                | IniFile.entry /docref_(ext|root)/
                | IniFile.entry /error_(append_string|log|prepend_string)/
                | IniFile.entry /ignore_(repeated_errors|repeated_source|user_abort)/
                | IniFile.entry /log_errors(_max_len)?/
                | IniFile.entry /magic_quotes_(gpc|runtime|sybase)/
                | IniFile.entry /max_(execution_time|input_time)/
                | IniFile.entry /output_(buffering|handler)/
                | IniFile.entry /realpath_(cache_size|cache_ttl)/
                | IniFile.entry /register_(argc_argv|globals|long_arrays)/
                | IniFile.entry /report_(memleaks|zend_debug)/
                | IniFile.entry /safe_mode(_(allowed_env_vars|exec_dir|gid|include_dir|protected_env_vars))?/
                | IniFile.entry /sendmail_(from|path)/
                | IniFile.entry /upload_(max_filesize|tmp_dir)/
                | IniFile.entry /user_(agent|dir)/
                | IniFile.entry /xmlrpc_error(s|_number)/
		| entry_allowspace "error_reporting"

	let record = IniFile.record "section" setting
	let lns    = IniFile.lns record

	let filter = (incl "/etc/php*/*/php.ini")
	           . Util.stdexcl

	let xfm = transform lns filter

