(*
Module: Splunk
  Parses /opt/splunk/etc/*

Author: Tim Brigham

About: Reference
  http://docs.splunk.com/Documentation/Splunk/5.0.2/Admin/AboutConfigurationFiles

About: License
   This file is licenced under the LGPL v2+

About: Lens Usage
   Works like IniFile lens, with anonymous section for entries without enclosing section.

About: Configuration files
   This lens applies to conf files under /opt/splunk/etc See <filter>.

About: Examples
   The <Test_Splunk> file contains various examples and tests.
*)

module Splunk =
  autoload xfm

  let comment   = IniFile.comment IniFile.comment_re IniFile.comment_default
  let sep       = IniFile.sep IniFile.sep_re IniFile.sep_default
  let empty     = IniFile.empty

  let setting   = IniFile.entry_re
  let title     =  IniFile.indented_title_label "target" IniFile.record_label_re
  let entry     = [ key IniFile.entry_re . sep . IniFile.sto_to_eol? . IniFile.eol ] | comment


  let record    = IniFile.record title entry
  let anon      = [ label ".anon" . (entry|empty)+ ]
  let lns       = anon . (record)* | (record)*

  let filter    = incl "/opt/splunk/etc/system/local/*.conf"
                . incl "/opt/splunk/etc/apps/*/local/*.conf"
  let xfm       = transform lns filter
