(*
Module: Simpleyaml

Parses standard and simple .yaml configuration file

Author: Ostapchuk Liana ssmgroll@gmail.com

Example of .yaml file:

> key: value
> key2:
>   key3: value3
>   key4:
>     key5: value5
>     key6: value6
>   key7: value7:........
>   key8:
>     - host:port
>     - host2:port2
> key9:
>   key10: value10
> key11:
>   key12: value12
>   key13:
>     key14: value14 
>     key15: value15_or_empty
*)

module Test_simpleyaml =

(* GET *)

let test1 ="---
foo: bar
pleh: help
stuff:
  foo: bar
  bar: foo
"

test Simpleyaml.lns get test1 = { "foo" = "bar" }
  { "pleh" = "help" }
  { "stuff"
    { "foo" = "bar" }
    { "bar" = "foo" }
  }

let test2 ="ssh_service:
  enabled: yes
  listen_addr: 1.1.1.1
  labels:
      team: test
      owner: iam
      env: some_env
      project: 
      product:
      cluster:

  commands:
    - name: name1
    command: hostname
    period: 1m0s
    - name: name2
    command: hostname2
    period: 1m0s2
"

test Simpleyaml.lns get test2 =   { "ssh_service"
    { "enabled" = "yes" }
    { "listen_addr" = "1.1.1.1" }
  }
  { "labels"
    { "team" = "test" }
    { "owner" = "iam" }
    { "env" = "some_env" }
  }
  { "project" }
  { "product" }
  { "cluster"
    {  }
  }
  { "commands"
    { "name" = "name1" }
    { "command" = "hostname" }
    { "period" = "1m0s" }
    { "name" = "name2" }
    { "command" = "hostname2" }
    { "period" = "1m0s2" }
  }


let test3 = "version: v2
teleport:
    data_dir: /path/
    log:
      output: /path/teleport.log
      severity: INFO
      format:
        output: json

    auth_servers:
    - hostname:3030
    - hostname3:30303

    join_params:
        token_name: 111dd2f-0a0f-4444d-aa-s-f7-a825bb878db9
        method: name

auth_service:
    enabled: no
proxy_service:
    enabled: no

ssh_service:
    enabled: yes
 #   disable_create_host_user: true
    listen_addr: 1.1.1.1:1111
    pam:
        enabled: true
        service_name: teleport

    commands:
    - name: name
      command: [hostname]
      period: 1m0s

    labels:
        team: name
        owner: ostapchuk.liana
        env: production
        project: may_be_empty
        product: may_be_empty
  #      cluster: may_be_empty
"

test Simpleyaml.lns get test3 =   { "version" = "v2" }
  { "teleport"
    { "data_dir" = "/path/" }
  }
  { "log"
    { "output" = "/path/teleport.log" }
    { "severity" = "INFO" }
  }
  { "format"
    { "output" = "json" }
  }
  { "auth_servers"
    { "hostname:3030" }
    { "hostname3:30303" }
  }
  { "join_params"
    { "token_name" = "111dd2f-0a0f-4444d-aa-s-f7-a825bb878db9" }
    { "method" = "name" }
  }
  { "auth_service"
    { "enabled" = "no" }
  }
  { "proxy_service"
    { "enabled" = "no" }
  }
  { "ssh_service"
    { "enabled" = "yes" }
    { "#comment" = "disable_create_host_user: true" }
    { "listen_addr" = "1.1.1.1:1111" }
  }
  { "pam"
    { "enabled" = "true" }
    { "service_name" = "teleport" }
  }
  { "commands"
    { "name" = "name" }
    { "command" = "[hostname]" }
    { "period" = "1m0s" }
  }
  { "labels"
    { "team" = "name" }
    { "owner" = "ostapchuk.liana" }
    { "env" = "production" }
    { "project" = "may_be_empty" }
    { "product" = "may_be_empty" }
    { "#comment" = "cluster: may_be_empty" }
  }

(* SET *)
test Simpleyaml.lns put test1 after
  set "foo" "changed_foo" = "foo: changed_foo
pleh: help
stuff:
  foo: bar
  bar: foo
"

test Simpleyaml.lns put test2 after
    set "labels/owner" "changed_iam" = "ssh_service:
  enabled: yes
  listen_addr: 1.1.1.1
  labels:
      team: test
      owner: changed_iam
      env: some_env
      project: 
      product:
      cluster:

  commands:
    name: name1
    command: hostname
    period: 1m0s
    name: name2
    command: hostname2
    period: 1m0s2
"

test Simpleyaml.lns put test3 after
    set "labels/new_label" "new" = "version: v2
teleport:
    data_dir: /path/
    log:
      output: /path/teleport.log
      severity: INFO
      format:
        output: json
    auth_servers:
    - hostname:3030
    - hostname3:30303
    join_params:
        token_name: 111dd2f-0a0f-4444d-aa-s-f7-a825bb878db9
        method: name
auth_service:
    enabled: no
proxy_service:
    enabled: no
ssh_service:
    enabled: yes
 #   disable_create_host_user: true
    listen_addr: 1.1.1.1:1111
    pam:
        enabled: true
        service_name: teleport
    commands:
    name: name
      command: [hostname]
      period: 1m0s
    labels:
        team: name
        owner: ostapchuk.liana
        env: production
        project: may_be_empty
        product: may_be_empty
  #      cluster: may_be_empty
new_label: new
"