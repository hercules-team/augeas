module NagiosCfg =
  autoload xfm

  let filter = incl "/etc/nagios3/nagios.cfg"

  let lns = Sysctl.lns

  let xfm = transform lns filter
