(*
Module: NagiosConfig
  Parses /etc/nagios3/*.cfg

Authors: Sebastien Aperghis-Tramoni <sebastien@aperghis.net>
         Raphaël Pinson <raphink@gmail.com>

About: License
  This file is licenced under the LGPLv2+, like the rest of Augeas.

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
               key /[A-Za-z0-9_]+/
             . Sep.opt_space . Sep.equal
             . Sep.opt_space . store Rx.no_spaces

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
               . Util.stdexcl

    let xfm = transform lns filter

