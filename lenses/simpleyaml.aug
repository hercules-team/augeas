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

module Simpleyaml =

(************************************************************************
*            USEFUL PRIMITIVES  
 *************************************************************************)
autoload xfm

let colon = Sep.colon
let space = del Rx.space " "
let label = key /[^,# \n\t]+/
let value = store (/[^ #\r\t\n].*[^ \r\t\n]|[^ \t\n\r]/)
let eol = Util.eol
let indent = Util.indent
let dash = Util.del_str "- " 
let comment = Util.comment
let hyphen = Util.del_str "---" . eol+ 
let empty = Util.empty

(************************************************************************
*            `KEY: VALUE` PAIR 
 *************************************************************************)
let k = label . colon . eol
let v = [ value . eol+ ]
let kv = [ label . colon . space . value . eol+ ]  

(************************************************************************
*            LENS & FILTER
 *************************************************************************)

let entry = (comment . eol* ) | [ indent . dash . label . eol+] | (indent . dash* . kv ) 
let record = [ indent . k . Util.empty* . entry* .  hyphen*]

let lns = ( hyphen* . ( (comment . eol* ) | kv )* . record* ) | empty


let filter = incl "/etc/*.yaml"
           . incl "/etc/*/*.yaml"
           . incl "/usr/share/*.yaml"
           . incl "/var/lib/*.yaml"
           . incl "/usr/share/*/*.yaml"
           . incl "/var/lib/*/*.yaml"

let xfm = transform lns filter