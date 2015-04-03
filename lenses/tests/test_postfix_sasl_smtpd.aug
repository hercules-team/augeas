module Test_Postfix_Sasl_Smtpd =


  let conf = "pwcheck_method: auxprop saslauthd
auxprop_plugin: plesk
saslauthd_path: /private/plesk_saslauthd
mech_list: CRAM-MD5 PLAIN LOGIN
sql_engine: intentionally disabled
log_level: 4
"

    
  test Postfix_sasl_smtpd.lns get conf =
    { "pwcheck_method" = "auxprop saslauthd" }
    { "auxprop_plugin" = "plesk" }
    { "saslauthd_path" = "/private/plesk_saslauthd" }
    { "mech_list" = "CRAM-MD5 PLAIN LOGIN" }
    { "sql_engine" = "intentionally disabled" }
    { "log_level" = "4" }
