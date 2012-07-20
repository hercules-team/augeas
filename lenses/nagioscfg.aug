(*
Module: NagiosConfig
  Parses /etc/nagios3/*.cfg

Authors: Sebastien Aperghis-Tramoni <sebastien@aperghis.net>
         Raphaël Pinson <raphink@gmail.com>

About: License
  This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
  To be documented

About: Configuration files
  This lens applies to /etc/nagios3/*.cfg. See <filter>.
*)

module NagiosCfg =
    autoload xfm

    (* View: param_def
        define a field *)
    let param_def =
         let space_in  = /[^ \t\n][^\n=]*[^ \t\n]|[^ \t\n]/
      in key /[A-Za-z0-9_]+/
       . Sep.space_equal
       . store space_in

    (* View: param
        Params can have sub params *)
    let param =
         [ Util.indent . param_def
         . [ Sep.space . param_def ]*
         . Util.eol ]

    (* View: lns
        main structure *)
    let lns = ( Util.empty | Util.comment | param )*

    (* View: filter *)
    let filter = incl "/etc/nagios3/*.cfg"
               . excl "/etc/nagios3/commands.cfg"
               . excl "/etc/nagios3/resource.cfg"
               . Util.stdexcl

    let xfm = transform lns filter

