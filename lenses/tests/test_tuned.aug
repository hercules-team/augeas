module Test_tuned =

let conf = "# Global tuned configuration file.

dynamic_tuning = 0
update_interval = 10
"

test Tuned.lns get conf =
  { "#comment" = "Global tuned configuration file." }
  {  }
  { "dynamic_tuning" = "0" }
  { "update_interval" = "10" }
