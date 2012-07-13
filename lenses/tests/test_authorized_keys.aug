(*
Module: Test_Authorized_Keys
  Provides unit tests and examples for the <Authorized_Keys> lens.
*)

module Test_Authorized_Keys =

(* Test: Authorized_Keys *)
test Authorized_Keys.lns get "tunnel=\"0\",no-agent-forwarding,command=\"sh /etc/netstart tun0\",permitopen=\"192.0.2.1:80\",permitopen=\"192.0.2.2:25\" ssh-rsa AAAAB3NzaC1yc2EAAAABIwAAAQEA3RC8whKGFx+b7BMTFtnIWl6t/qyvOvnuqIrMNI9J8+1sEYv8Y/pJRh0vAe2RaSKAgB2hyzXwSJ1Fh+ooraUAJ+q7P2gg2kQF1nCFeGVjtV9m4ZrV5kZARcQMhp0Bp67tPo2TCtnthPYZS/YQG6u/6Aco1XZjPvuKujAQMGSgqNskhKBO9zfhhkAMIcKVryjKYHDfqbDUCCSNzlwFLts3nJ0Hfno6Hz+XxuBIfKOGjHfbzFyUQ7smYnzF23jFs4XhvnjmIGQJcZT4kQAsRwQubyuyDuqmQXqa+2SuQfkKTaPOlVqyuEWJdG2weIF8g3YP12czsBgNppz3jsnhEgstnQ== rpinson on rpinson\n" =
  { "key" = "AAAAB3NzaC1yc2EAAAABIwAAAQEA3RC8whKGFx+b7BMTFtnIWl6t/qyvOvnuqIrMNI9J8+1sEYv8Y/pJRh0vAe2RaSKAgB2hyzXwSJ1Fh+ooraUAJ+q7P2gg2kQF1nCFeGVjtV9m4ZrV5kZARcQMhp0Bp67tPo2TCtnthPYZS/YQG6u/6Aco1XZjPvuKujAQMGSgqNskhKBO9zfhhkAMIcKVryjKYHDfqbDUCCSNzlwFLts3nJ0Hfno6Hz+XxuBIfKOGjHfbzFyUQ7smYnzF23jFs4XhvnjmIGQJcZT4kQAsRwQubyuyDuqmQXqa+2SuQfkKTaPOlVqyuEWJdG2weIF8g3YP12czsBgNppz3jsnhEgstnQ=="
    { "options"
      { "tunnel" = "0" }
      { "no-agent-forwarding" }
      { "command" = "sh /etc/netstart tun0" }
      { "permitopen" = "192.0.2.1:80" }
      { "permitopen" = "192.0.2.2:25" }
    }
    { "type" = "ssh-rsa" }
    { "comment" = "rpinson on rpinson" } } 
