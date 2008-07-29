(* PHP module for Augeas                      *)
(* Author: Raphael Pinson <raphink@gmail.com> *)
(*                                            *)

module PHP =

    autoload xfm

    (* PHP is a standard INI file *)
    let setting = "always_populate_raw_post_data"
                | "asp_tags"
                | "browscap"
                | "child_terminate"
                | "define_syslog_variables"
                | "doc_root"
                | "enable_dl"
                | "engine"
                | "expose_php"
                | "extension_dir"
                | "file_uploads"
		| "gpc_order"
                | "html_errors"
                | "implicit_flush"
                | "include_path"
                | "last_modified"
                | "memory_limit"
                | "open_basedir"
                | "post_max_size"
                | "precision"
                | "serialize_precision"
                | "short_open_tag"
                | "SMTP"
                | "smtp_port"
                | "track_errors"
                | "unserialize_callback_func"
                | "variables_order"
                | "xbithack"
                | "y2k_compliance"
                | /apc\.(cache_by_default|enabled|filters|gc_ttl|mmap_file_mask|num_files_hint|optimization|shm_segments|shm_size|slam_defense|ttl|user_entries_hint|user_ttl)/
                | /apd\.(dumpdir|statement_tracing)/
                | /arg_separator\.(input|output)/
                | /assert\.(active|bail|callback|quiet_eval|warning)/
                | /bcmath\.(scale)/
                | /blenc\.(key_file)/
                | /com\.(allow_dcom|autoregister_casesensitive|autoregister_typelib|autoregister_verbose|code_page|typelib_file)/
                | /crack\.(default_dictionary)/
                | /daffodildb\.(default_host|default_password|default_socket|default_user|port)/
                | /date\.(default_latitude|default_longitude|sunrise_zenith|sunset_zenith)/
                | /dba\.(default_handler)/
                | /dbx\.(colnames_case)/
                | /exif\.(decode_jis_intel|decode_jis_motorola|decode_unicode_intel|decode_unicode_motorola|encode_jis|encode_unicode)/
                | /fbsql\.(allow_persistent|autocommit|batchsize|default_database|default_database_password|default_host|default_password|default_user|generate_warnings|max_connections|max_links|max_persistent|max_results)/
                | /highlight\.(bg|comment|default|html|keyword|string)/
                | /hyperwave\.(allow_persistent|default_port)/
                | /ibase\.(allow_persistent|dateformat|default_charset|default_db|default_password|default_user|max_links|max_persistent|timeformat|timestampformat)/
                | /iconv\.(input_encoding|internal_encoding|output_encoding)/
                | /ifx\.(allow_persistent|blobinfile|byteasvarchar|charasvarchar|default_host|default_password|default_user|max_links|max_persistent|nullformat|textasvarchar)/
                | /ingres\.(allow_persistent|default_database|default_password|default_user|max_links|max_persistent)/
                | /ircg\.(control_user|keep_alive_interval|max_format_message_sets|shared_mem_size|work_dir)/
                | /ldap\.(max_links)/
                | /mail\.(force_extra_parameters)/
                | /mailparse\.(def_charset)/
                | /maxdb\.(default_db|default_host|default_pw|default_user|long_readlen)/
                | /mbstring\.(detect_order|encoding_translation|func_overload|http_input|http_output|internal_encoding|language|script_encoding|substitute_character)/
                | /mcrypt\.(algorithms_dir|modes_dir)/
                | /mime_magic\.(debug|magicfile)/
                | /mssql\.(allow_persistent|batchsize|compatability_mode|connect_timeout|datetimeconvert|max_links|max_persistent|max_procs|min_error_severity|min_message_severity|secure_connection|textlimit|textsize|timeout)/
                | /msql\.(allow_persistent|max_links|max_persistent)/
                | /mysql\.(allow_persistent|connect_timeout|default_host|default_password|default_port|default_socket|default_user|max_links|max_persistent|trace_mode)/
                | /mysqli\.(default_host|default_port|default_pw|default_socket|default_user|max_links|reconnect)/
                | /namazu\.(debugmode|lang|loggingmode|sortmethod|sortorder)/
                | /nsapi\.(read_timeout)/
                | /odbc\.(allow_persistent|check_persistent|defaultbinmode|defaultlrl|default_db|default_pw|default_user|max_links|max_persistent)/
                | /opendirectory\.(max_refs|separator)/
                | /pdo\.(global_value)/
                | /pfpro\.(defaulthost|defaultport|defaulttimeout|proxyaddress|proxylogon|proxypassword|proxyport)/
                | /pgsql\.(allow_persistent|auto_reset_persistent|ignore_notice|log_notice|max_links|max_persistent)/
                | /printer\.(default_printer)/
                | /session\.(auto_start|bug_compat_42|bug_compat_warn|cache_expire|cache_limiter|cookie_(domain|httponly|lifetime|path|secure)|entropy_file|entropy_length|gc_divisor|gc_maxlifetime|gc_probability|hash_bits_per_character|hash_function|name|referer_check|save_handler|save_path|serialize_handler|use_cookies|use_only_cookies|use_trans_sid)/
                | /session_pgsql\.(create_table|db|disable|failover_mode|gc_interval|keep_expired|sem_file_name|serializable|short_circuit|use_app_vars|vacuum_interval)/
                | /simple_cvs\.(authMethod|compressionLevel|cvsRoot|host|moduleName|userName|workingDir)/
                | /soap\.(wsdl_cache_dir|wsdl_cache_enabled|wsdl_cache_ttl)/
                | /sql\.(safe_mode)/
                | /sqlite\.(assoc_case)/
		| /sybase\.(allow_persistent|max_(persistent|links)|interface_file|min_(error|message)_severity|compatability_mode)/
                | /sybct\.(allow_persistent|deadlock_retry_count|hostname|login_timeout|max_links|max_persistent|min_client_severity|min_server_severity)/
                | /tidy\.(clean_output|default_config)/
                | /url_rewriter\.(tags)/
                | /valkyrie\.(auto_validate|config_path)/
                | /xmms\.(path|session)/
                | /yaz\.(keepalive|log_file|max_links)/
                | /zend\.(ze1_compatibility_mode)/
                | /zlib\.(output_compression|output_compression_level|output_handler)/
                | /allow_(call_time_pass_reference|url_fopen|url_include)/
                | /auto_(append_file|detect_line_endings|globals_jit|prepend_file)/
                | /default_(charset|mimetype|socket_timeout)/
		| /display(_startup)?_errors/
                | /disable_(classes|functions)/
                | /docref_(ext|root)/
                | /error_(append_string|log|prepend_string|reporting)/
                | /ignore_(repeated_errors|repeated_source|user_abort)/
                | /log_errors(_max_len)?/
                | /magic_quotes_(gpc|runtime|sybase)/
                | /max_(execution_time|input_time)/
                | /output_(buffering|handler)/
                | /realpath_(cache_size|cache_ttl)/
                | /register_(argc_argv|globals|long_arrays)/
                | /report_(memleaks|zend_debug)/
                | /safe_mode(_(allowed_env_vars|exec_dir|gid|include_dir|protected_env_vars))?/
                | /sendmail_(from|path)/
                | /upload_(max_filesize|tmp_dir)/
                | /user_(agent|dir)/
                | /xmlrpc_error(s|_number)/


	let entry  = IniFile.entry setting

	let record = IniFile.record "section" entry
	let lns    = IniFile.lns record

	let filter = (incl "/etc/php*/*/php.ini")
	           . Util.stdexcl

	let xfm = transform lns filter

