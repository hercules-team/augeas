module Test_cgconfig =

let conf="#cgconfig test cofiguration file
mount { 123 = 456; 456 = 789;}
"

test Cgconfig.lns get conf =
  { "#comment" = "cgconfig test cofiguration file" }
  { "mount"
      { "123" = "456" }
      { "456" = "789" } }
  {}

(* white spaces before mount sign *)
let conf2="
 mount { 123 = 456;}
    mount { 123 = 456;}

mount { 123 = 456;}mount { 123 = 456;}
"

test Cgconfig.lns get conf2 =
  { }
  { "mount" { "123" = "456"} }
  { }
  { "mount" { "123" = "456"} }
  { }
  { }
  { "mount" { "123" = "456"} }
  { "mount" { "123"  = "456" } }
  { }

let conf3="#cgconfig test cofiguration file
mount { 123 = 456;
#eswkh
 456 = 789;}
"
test Cgconfig.lns get conf3 =
  { "#comment" = "cgconfig test cofiguration file" }
  { "mount"
    { "123" = "456" }
    {}
    { "#comment" = "eswkh" }
    { "456" = "789" } }
  {}

let conf4="#cgconfig test cofiguration file
mount {
123 = 456;1245=456;
}
mount { 323=324;}mount{324=5343;  }# this is a comment
"

test Cgconfig.lns get conf4 =
  {"#comment" = "cgconfig test cofiguration file" }
  {"mount"
     {  }
     { "123" = "456"}
     { "1245" = "456" }
     {  }}
  { }
  { "mount" { "323" = "324" } }
  { "mount" { "324" = "5343" } }
  { "#comment" = "this is a comment" }

let group1="
group user {
	cpuacct {
      lll = jjj;
	}
	cpu {
	}
}"

test Cgconfig.lns get group1 =
  {  }
  { "group" = "user"
    {  }
    { "controller" = "cpuacct"
      {  }
      { "lll" = "jjj" }
      {  } }
    {  }
    { "controller" = "cpu" {  } }
    {  } }

let group2="
group aa-1{
     perm {
		 task { }
		 admin { }
      }
}"

test Cgconfig.lns get group2 =
  {  }
  { "group" = "aa-1"
    {  }
    { "perm"
      {  }
      { "task" }
      {  }
      { "admin" }
      {  } }
    {  } }


let group3 ="
group xx/www {
 perm {
	task {
		gid = root;
		uid = root;
	}
	admin {
		gid = aaa;
# no aaa
		uid = aaa;
	}
}
}
"

test Cgconfig.lns get group3 =
  {  }
  { "group" = "xx/www"
    {  }
    { "perm"
      {  }
      { "task"
        {  }
        { "gid" = "root" }
        {  }
        { "uid" = "root" }
        {  } }
      {  }
      { "admin"
        {  }
        { "gid" = "aaa" }
        {  }
        { "#comment" = "no aaa" }
        { "uid" = "aaa" }
        {  } }
      {  } }
    {  } }
  {  }

let group4 ="
#group daemons {
#           cpuacct{
#           }
#}

group daemons/ftp {
                     cpuacct{
                                          }
}

   group daemons/www {
        perm {
                task {
                        uid = root;
                        gid = root;
                }
                admin {
                        uid = root;
                        gid = root;
                }
        }
#       cpu {
#               cpu.shares = 1000;
#       }
}
#
#

  mount {
       devices = /mnt/cgroups/devices;cpuacct = /mnt/cgroups/cpuset;
        cpuset = /mnt/cgroups/cpuset;


        cpu = /mnt/cpu;
#        cpuset = /mnt/cgroups/cpuset2;
}
mount   {
devices = /mnt/cgroups/devices;
#       cpuacct = /mnt/cgroups/cpuacct;
        ns = /mnt/cgroups/ns;
#
}

"

test Cgconfig.lns get group4 =
  {  }
  { "#comment" = "group daemons {" }
  { "#comment" = "cpuacct{" }
  { "#comment" = "}" }
  { "#comment" = "}" }
  {  }
  { "group" = "daemons/ftp"
    {  }
    { "controller" = "cpuacct" {  } }
    {  } }
  {  }
  {  }
  { "group" = "daemons/www"
    {  }
    { "perm"
      {  }
      { "task"
        {  }
        { "uid" = "root" }
        {  }
        { "gid" = "root" }
        {  } }
      {  }
      { "admin"
        {  }
        { "uid" = "root" }
        {  }
        { "gid" = "root" }
        {  } }
      {  } }
    {  }
    { "#comment" = "cpu {" }
    { "#comment" = "cpu.shares = 1000;" }
    { "#comment" = "}" } }
  {  }
  {  }
  {  }
  {  }
  { "mount"
    {  }
    { "devices" = "/mnt/cgroups/devices" }
    { "cpuacct" = "/mnt/cgroups/cpuset" }
    {  }
    { "cpuset" = "/mnt/cgroups/cpuset" }
    {  }
    {  }
    {  }
    { "cpu" = "/mnt/cpu" }
    {  }
    { "#comment" = "cpuset = /mnt/cgroups/cpuset2;" } }
  {  }
  { "mount"
    {  }
    { "devices" = "/mnt/cgroups/devices" }
    {  }
    { "#comment" = "cpuacct = /mnt/cgroups/cpuacct;" }
    { "ns" = "/mnt/cgroups/ns" }
    {  }
    {  } }
  {  }
  {  }

test Cgconfig.lns put "group tst {memory {}}" after
  set "/group" "tst2"
= "group tst2 {memory {}}"
