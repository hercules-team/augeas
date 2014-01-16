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

(* Variable: keys *)
let keys = "# Example keys, one of each type
#
ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDpWrKYsEsVUyuwMN4ReBN/TMGsaUWzDKDz/uQr6MlNNM95MDK/BPyJ+DiBiNMFVLpRt3gH3eCJBLJKMuUDaTNy5uym2zNgAaAIVct6M2GHI68W3iY3Ja8/MaRPbyTpMh1O74S+McpAW1SGL2YzFchYMjTnu/kOD3lxiWNiDLvdLFZu0wPOi7CYG37VXR4Thb0cC92zqnCjaP1TwfhpEYUZoowElYkoV2vG+19O6cRm/zduYcf8hmegZKB4GFUJTtZ2gZ18XJDSQd0ykK3KPt/+bKskdrtfiOwSZAmUZmd2YuAlY6+CBn1T3UBdQntueukd0z1xhd6SX7Bl8+qyqLQ3 user@example
ssh-dsa AAAA user@example
ecdsa-sha2-nistp256 AAAA user@example
ssh-ed25519 AAAA user@example

# Example comments
ssh-dsa AAAA
ssh-dsa AAAA user@example
"

(* Test: Authorized_Keys.lns *)
test Authorized_Keys.lns get keys =
  { "#comment" = "Example keys, one of each type" }
  {  }
  { "key" =
"AAAAB3NzaC1yc2EAAAADAQABAAABAQDpWrKYsEsVUyuwMN4ReBN/TMGsaUWzDKDz/uQr6MlNNM95MDK/BPyJ+DiBiNMFVLpRt3gH3eCJBLJKMuUDaTNy5uym2zNgAaAIVct6M2GHI68W3iY3Ja8/MaRPbyTpMh1O74S+McpAW1SGL2YzFchYMjTnu/kOD3lxiWNiDLvdLFZu0wPOi7CYG37VXR4Thb0cC92zqnCjaP1TwfhpEYUZoowElYkoV2vG+19O6cRm/zduYcf8hmegZKB4GFUJTtZ2gZ18XJDSQd0ykK3KPt/+bKskdrtfiOwSZAmUZmd2YuAlY6+CBn1T3UBdQntueukd0z1xhd6SX7Bl8+qyqLQ3"
    { "type" = "ssh-rsa" }
    { "comment" = "user@example" }
  }
  { "key" = "AAAA"
    { "type" = "ssh-dsa" }
    { "comment" = "user@example" }
  }
  { "key" = "AAAA"
    { "type" = "ecdsa-sha2-nistp256" }
    { "comment" = "user@example" }
  }
  { "key" = "AAAA"
    { "type" = "ssh-ed25519" }
    { "comment" = "user@example" }
  }
  {  }
  { "#comment" = "Example comments" }
  { "key" = "AAAA"
    { "type" = "ssh-dsa" }
  }
  { "key" = "AAAA"
    { "type" = "ssh-dsa" }
    { "comment" = "user@example" }
  }

(* Variable: options *)
let options = "# Example options
no-pty ssh-dsa AAAA
no-pty ssh-ed25519 AAAA
no-pty,command=\"foo\" ssh-dsa AAAA
no-pty,command=\"foo bar\" ssh-dsa AAAA
no-pty,from=\"example.com,10.1.1.0/16\" ssh-dsa AAAA
no-pty,environment=\"LANG=en_GB.UTF8\" ssh-dsa AAAA
"

(* Test: Authorized_Keys.lns *)
test Authorized_Keys.lns get options =
  { "#comment" = "Example options" }
  { "key" = "AAAA"
    { "options"
      { "no-pty" }
    }
    { "type" = "ssh-dsa" }
  }
  { "key" = "AAAA"
    { "options"
      { "no-pty" }
    }
    { "type" = "ssh-ed25519" }
  }
  { "key" = "AAAA"
    { "options"
      { "no-pty" }
      { "command" = "foo" }
    }
    { "type" = "ssh-dsa" }
  }
  { "key" = "AAAA"
    { "options"
      { "no-pty" }
      { "command" = "foo bar" }
    }
    { "type" = "ssh-dsa" }
  }
  { "key" = "AAAA"
    { "options"
      { "no-pty" }
      { "from" = "example.com,10.1.1.0/16" }
    }
    { "type" = "ssh-dsa" }
  }
  { "key" = "AAAA"
    { "options"
      { "no-pty" }
      { "environment" = "LANG=en_GB.UTF8" }
    }
    { "type" = "ssh-dsa" }
  }
