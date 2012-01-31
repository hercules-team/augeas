(*
Module: Test_Anacron
  Provides unit tests and examples for the <Anacron> lens.
*)

module Test_anacron =

(* Variable: conf *)
let conf = "# /etc/anacrontab: configuration file for anacron

SHELL=/bin/sh
PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin

# These replace cron's entries
1	5	cron.daily	 nice run-parts --report /etc/cron.daily
7	10	cron.weekly	 nice run-parts --report /etc/cron.weekly
@monthly	15	cron.monthly nice run-parts --report /etc/cron.monthly
"

(* Test: Anacron.lns *)
test Anacron.lns get conf =
  { "#comment" = "/etc/anacrontab: configuration file for anacron" }
  { }
  { "SHELL" = "/bin/sh" }
  { "PATH" = "/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin" }
  { }
  { "#comment" = "These replace cron's entries" }
  { "entry" = "nice run-parts --report /etc/cron.daily"
     { "period" = "1" }
     { "delay" = "5" }
     { "job-identifier" = "cron.daily" } }
  { "entry" = "nice run-parts --report /etc/cron.weekly"
     { "period" = "7" }
     { "delay" = "10" }
     { "job-identifier" = "cron.weekly" } }
  { "entry" = "nice run-parts --report /etc/cron.monthly"
     { "period_name" = "monthly" }
     { "delay" = "15" }
     { "job-identifier" = "cron.monthly" } }
