(*
Module: Test_Channels
  Provides unit tests and examples for the <Channels> lens.
*)

module Test_Channels =

(* Variable: conf
   A full configuration file *)
let conf = "Direct 8 TV;SES ASTRA:12551:VC56M2O0S0:S19.2E:22000:1111=2:1112=fra@3:1116:0:12174:1:1108:0
:FAVORIS
Direct 8 TV;SES ASTRA:12551:VC56M2O0S0:S19.2E:22000:1111=2:1112=fra@3:1116:0:12175:1:1108:0
TF1;CSAT:11895:VC34M2O0S0:S19.2E:27500:171=2:124=fra+spa@4,125=eng@4;126=deu@4:53:500,1811,1863,100:8371:1:1074:0
:TNT
TF1;SMR6:690167:I999B8C999D999M998T999G999Y0:T:27500:120=2:130=fra@3,131=eng@3,133=qad@3:140;150=fra,151=eng:0:1537:8442:6:0
; this is a comment
France 5;GR1:618167:I999B8C999D999M998T999G999Y0:T:27500:374+320=2:330=fra@3,331=qad@3:0;340=fra:0:260:8442:1:0
CANAL+ FAMILY HD:12012:VC23M5O35S1:S19.2E:27500:164=27:0;98=@106,99=eng@106:0;45=fra+fra:1811,500,1863,100,9C4,9C7,9AF:8825:1:1080:0
"

(* Test: Channels.lns
   Test the full <conf> *)
test Channels.lns get conf =
   { "entry" = "Direct 8 TV"
      { "provider" = "SES ASTRA" }
      { "frequency" = "12551" }
      { "parameter" = "VC56M2O0S0" }
      { "signal_source" = "S19.2E" }
      { "symbol_rate" = "22000" }
      { "vpid" = "1111" { "codec" = "2" } }
      { "apid" = "1112" { "lang" = "fra" } { "codec" = "3" } }
      { "tpid" = "1116" }
      { "caid" = "0" }
      { "sid" = "12174" }
      { "nid" = "1" }
      { "tid" = "1108" }
      { "rid" = "0" }
   }
   { "group" = "FAVORIS"
      { "entry" = "Direct 8 TV"
         { "provider" = "SES ASTRA" }
         { "frequency" = "12551" }
         { "parameter" = "VC56M2O0S0" }
         { "signal_source" = "S19.2E" }
         { "symbol_rate" = "22000" }
         { "vpid" = "1111" { "codec" = "2" } }
         { "apid" = "1112" { "lang" = "fra" } { "codec" = "3" } }
         { "tpid" = "1116" }
         { "caid" = "0" }
         { "sid" = "12175" }
         { "nid" = "1" }
         { "tid" = "1108" }
         { "rid" = "0" }
      }
      { "entry" = "TF1"
         { "provider" = "CSAT" }
         { "frequency" = "11895" }
         { "parameter" = "VC34M2O0S0" }
         { "signal_source" = "S19.2E" }
         { "symbol_rate" = "27500" }
         { "vpid" = "171" { "codec" = "2" } }
         { "apid" = "124" { "lang" = "fra" } { "lang" = "spa" } { "codec" = "4" } }
         { "apid" = "125" { "lang" = "eng" } { "codec" = "4" } }
         { "apid_dolby" = "126" { "lang" = "deu" } { "codec" = "4" } }
         { "tpid" = "53" }
         { "caid" = "500" }
         { "caid" = "1811" }
         { "caid" = "1863" }
         { "caid" = "100" }
         { "sid" = "8371" }
         { "nid" = "1" }
         { "tid" = "1074" }
         { "rid" = "0" }
      }
   }
   { "group" = "TNT"
     { "entry" = "TF1"
       { "provider" = "SMR6" }
       { "frequency" = "690167" }
       { "parameter" = "I999B8C999D999M998T999G999Y0" }
       { "signal_source" = "T" }
       { "symbol_rate" = "27500" }
       { "vpid" = "120" { "codec" = "2" } }
       { "apid" = "130" { "lang" = "fra" } { "codec" = "3" } }
       { "apid" = "131" { "lang" = "eng" } { "codec" = "3" } }
       { "apid" = "133" { "lang" = "qad" } { "codec" = "3" } }
       { "tpid" = "140" }
       { "tpid_bylang" = "150" { "lang" = "fra" } }
       { "tpid_bylang" = "151" { "lang" = "eng" } }
       { "caid" = "0" }
       { "sid" = "1537" }
       { "nid" = "8442" }
       { "tid" = "6" }
       { "rid" = "0" }
     }
     { "#comment" = "this is a comment" }
     { "entry" = "France 5"
       { "provider" = "GR1" }
       { "frequency" = "618167" }
       { "parameter" = "I999B8C999D999M998T999G999Y0" }
       { "signal_source" = "T" }
       { "symbol_rate" = "27500" }
       { "vpid" = "374" }
       { "vpid_pcr" = "320" { "codec" = "2" } }
       { "apid" = "330" { "lang" = "fra" } { "codec" = "3" } }
       { "apid" = "331" { "lang" = "qad" } { "codec" = "3" } }
       { "tpid" = "0" }
       { "tpid_bylang" = "340" { "lang" = "fra" } }
       { "caid" = "0" }
       { "sid" = "260" }
       { "nid" = "8442" }
       { "tid" = "1" }
       { "rid" = "0" }
     }
     { "entry" = "CANAL+ FAMILY HD"
       { "frequency" = "12012" }
       { "parameter" = "VC23M5O35S1" }
       { "signal_source" = "S19.2E" }
       { "symbol_rate" = "27500" }
       { "vpid" = "164" { "codec" = "27" } }
       { "apid" = "0" }
       { "apid_dolby" = "98" { "codec" = "106" } }
       { "apid_dolby" = "99" { "lang" = "eng" } { "codec" = "106" } }
       { "tpid" = "0" }
       { "tpid_bylang" = "45" { "lang" = "fra" } { "lang" = "fra" } }
       { "caid" = "1811" }
       { "caid" = "500" }
       { "caid" = "1863" }
       { "caid" = "100" }
       { "caid" = "9C4" }
       { "caid" = "9C7" }
       { "caid" = "9AF" }
       { "sid" = "8825" }
       { "nid" = "1" }
       { "tid" = "1080" }
       { "rid" = "0" }
     }
   }
