module Test_termcap =

(* Sample termcap entry with escaped ':''s *)
let termcap = "vt420pc|DEC VT420 w/PC keyboard:\\
        :@7=\\E[4~:F1=\\E[23~:F2=\\E[24~:F3=\\E[11;2~:F4=\\E[12;2~:\\
        :F5=\\E[13;2~:F6=\\E[14;2~:F7=\\E[15;2~:F8=\\E[17;2~:\\
        :F9=\\E[18;2~:FA=\\E[19;2~:FB=\\E[20;2~:FC=\\E[21;2~:\\
        :FD=\\E[23;2~:FE=\\E[24;2~:FF=\\E[23~:FG=\\E[24~:FH=\\E[25~:\\
        :FI=\\E[26~:FJ=\\E[28~:FK=\\E[29~:FL=\\E[31~:FM=\\E[32~:\\
        :FN=\\E[33~:FO=\\E[34~:FP=\\E[35~:FQ=\\E[36~:FR=\\E[23;2~:\\
        :FS=\\E[24;2~:FT=\\E[25;2~:FU=\\E[26;2~:FV=\\E[28;2~:\\
        :FW=\\E[29;2~:FX=\\E[31;2~:FY=\\E[32;2~:FZ=\\E[33;2~:\\
        :Fa=\\E[34;2~:Fb=\\E[35;2~:Fc=\\E[36;2~:\\
        :S6=USR_TERM\\:vt420pcdos\\::k1=\\E[11~:k2=\\E[12~:\\
        :k3=\\E[13~:k4=\\E[14~:k5=\\E[15~:k6=\\E[17~:k7=\\E[18~:\\
        :k8=\\E[19~:k9=\\E[20~:k;=\\E[21~:kD=\\177:kh=\\E[H:\\
        :..px=\\EP1;1|%?%{16}%p1%>%t%{0}%e%{21}%p1%>%t%{1}%e%{25}%p1%>%t%{2}%e%{27}%p1%>%t%{3}%e%{30}%p1%>%t%{4}%e%{5}%;%p1%+%d/%p2%s\E\\\\:\\
        :tc=vt420:
"

test Termcap.lns get termcap =
  { "record"
    { "name" = "vt420pc" }
    { "name" = "DEC VT420 w/PC keyboard" }
    { "capability" = "@7=\\E[4~" }
    { "capability" = "F1=\\E[23~" }
    { "capability" = "F2=\\E[24~" }
    { "capability" = "F3=\\E[11;2~" }
    { "capability" = "F4=\\E[12;2~" }
    { "capability" = "F5=\\E[13;2~" }
    { "capability" = "F6=\\E[14;2~" }
    { "capability" = "F7=\\E[15;2~" }
    { "capability" = "F8=\\E[17;2~" }
    { "capability" = "F9=\\E[18;2~" }
    { "capability" = "FA=\\E[19;2~" }
    { "capability" = "FB=\\E[20;2~" }
    { "capability" = "FC=\\E[21;2~" }
    { "capability" = "FD=\\E[23;2~" }
    { "capability" = "FE=\\E[24;2~" }
    { "capability" = "FF=\\E[23~" }
    { "capability" = "FG=\\E[24~" }
    { "capability" = "FH=\\E[25~" }
    { "capability" = "FI=\\E[26~" }
    { "capability" = "FJ=\\E[28~" }
    { "capability" = "FK=\\E[29~" }
    { "capability" = "FL=\\E[31~" }
    { "capability" = "FM=\\E[32~" }
    { "capability" = "FN=\\E[33~" }
    { "capability" = "FO=\\E[34~" }
    { "capability" = "FP=\\E[35~" }
    { "capability" = "FQ=\\E[36~" }
    { "capability" = "FR=\\E[23;2~" }
    { "capability" = "FS=\\E[24;2~" }
    { "capability" = "FT=\\E[25;2~" }
    { "capability" = "FU=\\E[26;2~" }
    { "capability" = "FV=\\E[28;2~" }
    { "capability" = "FW=\\E[29;2~" }
    { "capability" = "FX=\\E[31;2~" }
    { "capability" = "FY=\\E[32;2~" }
    { "capability" = "FZ=\\E[33;2~" }
    { "capability" = "Fa=\\E[34;2~" }
    { "capability" = "Fb=\\E[35;2~" }
    { "capability" = "Fc=\\E[36;2~" }
    { "capability" = "S6=USR_TERM\\:vt420pcdos\\:" }
    { "capability" = "k1=\\E[11~" }
    { "capability" = "k2=\\E[12~" }
    { "capability" = "k3=\\E[13~" }
    { "capability" = "k4=\\E[14~" }
    { "capability" = "k5=\\E[15~" }
    { "capability" = "k6=\\E[17~" }
    { "capability" = "k7=\\E[18~" }
    { "capability" = "k8=\\E[19~" }
    { "capability" = "k9=\\E[20~" }
    { "capability" = "k;=\\E[21~" }
    { "capability" = "kD=\\177" }
    { "capability" = "kh=\\E[H" }
    { "capability" = "..px=\\EP1;1|%?%{16}%p1%>%t%{0}%e%{21}%p1%>%t%{1}%e%{25}%p1%>%t%{2}%e%{27}%p1%>%t%{3}%e%{30}%p1%>%t%{4}%e%{5}%;%p1%+%d/%p2%s\\E\\\\" }
    { "capability" = "tc=vt420" }
  }

let termcap2 = "tws-generic|dku7102|Bull Questar tws terminals:\\
        :am:es:hs:mi:ms:xn:xo:xs@:\\
        :co#80:it#8:li#24:ws#80:\\
        :AL=\\E[%dL:DC=\\E[%dP:DL=\\E[%dM:DO=\\E[%dB:LE=\\E[%dD:\\
        :RI=\\E[%dC:UP=\\E[%dA:al=\\E[L:bl=^G:bt=\\E[Z:cd=\\E[J:ce=\\E[K:\\
        :cl=\\E[2J:cm=\\E[%i%d;%df:cr=^M:ct=\\E[3g:dc=\\E[P:dl=\\E[M:\\
        :do=^J:ds=\\EPY99\\:98\\E\\\\\\E[0;98v\\E[2J\\E[v:ei=\\E[4l:\\
        :fs=\\E[v:ho=\\E[H:i1=\\E[?=h\\Ec\\E`\\E[?>h\\EPY99\\:98\\E\\\\\\\\:\\
        :i2=\\Eb\\E[?<h:im=\\E[4h:\\
        :is=\\E[5;>;12;18;?<l\\E[=h\\EP1s\\E\\\\\\E[\\027p:\\
        :k1=\\E[1u\\027:k2=\\E[2u\\027:k3=\\E[3u\\027:k4=\\E[4u\\027:\\
        :k5=\\E[5u\\027:k6=\\E[6u\\027:k7=\\E[7u\\027:k8=\\E[8u\\027:\\
        :kD=\\E[P:kb=^H:kd=\\E[B:kh=\\E[H:kl=\\E[D:kr=\\E[C:ku=\\E[A:\\
        :le=^H:ll=\\E[H\\E[A:mb=\\E[0;5m:me=\\E[0m\\017:mh=\\E[0;2m:\\
        :mr=\\E[0;7m:nd=\\E[C:rs=\\E[?=h\\Ec:se=\\E[m:sf=^J:so=\\E[0;7m:\\
        :st=\\EH:ta=\\E[I:te=\\E[0;98v\\E[2J\\E[v:\\
        :ti=\\E[?>h\\EPY99\\:98\\E\\\\\\\\:\\
        :ts=\\EPY99\\:98\\E\\\\\\E[0;98v\\E[2;7m:ue=\\E[m:up=\\E[A:\\
        :us=\\E[0;4m:ve=\\E[r:vi=\\E[1r:
"

test Termcap.lns get termcap2 =
  { "record"
    { "name" = "tws-generic" }
    { "name" = "dku7102" }
    { "name" = "Bull Questar tws terminals" }
    { "capability" = "am" }
    { "capability" = "es" }
    { "capability" = "hs" }
    { "capability" = "mi" }
    { "capability" = "ms" }
    { "capability" = "xn" }
    { "capability" = "xo" }
    { "capability" = "xs@" }
    { "capability" = "co#80" }
    { "capability" = "it#8" }
    { "capability" = "li#24" }
    { "capability" = "ws#80" }
    { "capability" = "AL=\\E[%dL" }
    { "capability" = "DC=\\E[%dP" }
    { "capability" = "DL=\\E[%dM" }
    { "capability" = "DO=\\E[%dB" }
    { "capability" = "LE=\\E[%dD" }
    { "capability" = "RI=\\E[%dC" }
    { "capability" = "UP=\\E[%dA" }
    { "capability" = "al=\\E[L" }
    { "capability" = "bl=^G" }
    { "capability" = "bt=\\E[Z" }
    { "capability" = "cd=\\E[J" }
    { "capability" = "ce=\\E[K" }
    { "capability" = "cl=\\E[2J" }
    { "capability" = "cm=\\E[%i%d;%df" }
    { "capability" = "cr=^M" }
    { "capability" = "ct=\\E[3g" }
    { "capability" = "dc=\\E[P" }
    { "capability" = "dl=\\E[M" }
    { "capability" = "do=^J" }
    { "capability" = "ds=\\EPY99\\:98\\E\\\\\\E[0;98v\\E[2J\\E[v" }
    { "capability" = "ei=\\E[4l" }
    { "capability" = "fs=\\E[v" }
    { "capability" = "ho=\\E[H" }
    { "capability" = "i1=\\E[?=h\\Ec\\E`\\E[?>h\\EPY99\\:98\\E\\\\\\\\" }
    { "capability" = "i2=\\Eb\\E[?<h" }
    { "capability" = "im=\\E[4h" }
    { "capability" = "is=\\E[5;>;12;18;?<l\\E[=h\\EP1s\\E\\\\\\E[\\027p" }
    { "capability" = "k1=\\E[1u\\027" }
    { "capability" = "k2=\\E[2u\\027" }
    { "capability" = "k3=\\E[3u\\027" }
    { "capability" = "k4=\\E[4u\\027" }
    { "capability" = "k5=\\E[5u\\027" }
    { "capability" = "k6=\\E[6u\\027" }
    { "capability" = "k7=\\E[7u\\027" }
    { "capability" = "k8=\\E[8u\\027" }
    { "capability" = "kD=\\E[P" }
    { "capability" = "kb=^H" }
    { "capability" = "kd=\\E[B" }
    { "capability" = "kh=\\E[H" }
    { "capability" = "kl=\\E[D" }
    { "capability" = "kr=\\E[C" }
    { "capability" = "ku=\\E[A" }
    { "capability" = "le=^H" }
    { "capability" = "ll=\\E[H\\E[A" }
    { "capability" = "mb=\\E[0;5m" }
    { "capability" = "me=\\E[0m\\017" }
    { "capability" = "mh=\\E[0;2m" }
    { "capability" = "mr=\\E[0;7m" }
    { "capability" = "nd=\\E[C" }
    { "capability" = "rs=\\E[?=h\\Ec" }
    { "capability" = "se=\\E[m" }
    { "capability" = "sf=^J" }
    { "capability" = "so=\\E[0;7m" }
    { "capability" = "st=\\EH" }
    { "capability" = "ta=\\E[I" }
    { "capability" = "te=\\E[0;98v\\E[2J\\E[v" }
    { "capability" = "ti=\\E[?>h\\EPY99\\:98\\E\\\\\\\\" }
    { "capability" = "ts=\\EPY99\\:98\\E\\\\\\E[0;98v\\E[2;7m" }
    { "capability" = "ue=\\E[m" }
    { "capability" = "up=\\E[A" }
    { "capability" = "us=\\E[0;4m" }
    { "capability" = "ve=\\E[r" }
    { "capability" = "vi=\\E[1r" }
  }

let termcap3 = "stv52|MiNT virtual console:\\
        :am:ms:\\
        :co#80:it#8:li#30:\\
        :%1=\\EH:&8=\\EK:F1=\\Ep:F2=\\Eq:F3=\\Er:F4=\\Es:F5=\\Et:F6=\\Eu:\\
        :F7=\\Ev:F8=\\Ew:F9=\\Ex:FA=\\Ey:al=\\EL:bl=^G:cd=\\EJ:ce=\\EK:\\
        :cl=\\EE:cm=\\EY%+ %+ :cr=^M:dl=\\EM:do=\\EB:ho=\\EH:k1=\\EP:\\
        :k2=\\EQ:k3=\\ER:k4=\\ES:k5=\\ET:k6=\\EU:k7=\\EV:k8=\\EW:k9=\\EX:\\
        :k;=\\EY:kD=\\177:kI=\\EI:kN=\\Eb:kP=\\Ea:kb=^H:kd=\\EB:kh=\\EE:\\
        :kl=\\ED:kr=\\EC:ku=\\EA:le=^H:mb=\\Er:md=\\EyA:me=\\Ez_:mh=\\Em:\\
        :mr=\\Ep:nd=\\EC:nw=2*\\r\\n:op=\\Eb@\\EcO:r1=\\Ez_\\Eb@\\EcA:\\
        :se=\\Eq:sf=2*\\n:so=\\Ep:sr=2*\\EI:ta=^I:te=\\Ev\\E. \\Ee\\Ez_:\\
        :ti=\\Ev\\Ee\\Ez_:ue=\\EzH:up=\\EA:us=\\EyH:ve=\\E. \\Ee:vi=\\Ef:\\
        :vs=\\E.\":
"

test Termcap.lns get termcap3 =
  { "record"
    { "name" = "stv52" }
    { "name" = "MiNT virtual console" }
    { "capability" = "am" }
    { "capability" = "ms" }
    { "capability" = "co#80" }
    { "capability" = "it#8" }
    { "capability" = "li#30" }
    { "capability" = "%1=\\EH" }
    { "capability" = "&8=\\EK" }
    { "capability" = "F1=\\Ep" }
    { "capability" = "F2=\\Eq" }
    { "capability" = "F3=\\Er" }
    { "capability" = "F4=\\Es" }
    { "capability" = "F5=\\Et" }
    { "capability" = "F6=\\Eu" }
    { "capability" = "F7=\\Ev" }
    { "capability" = "F8=\\Ew" }
    { "capability" = "F9=\\Ex" }
    { "capability" = "FA=\\Ey" }
    { "capability" = "al=\\EL" }
    { "capability" = "bl=^G" }
    { "capability" = "cd=\\EJ" }
    { "capability" = "ce=\\EK" }
    { "capability" = "cl=\\EE" }
    { "capability" = "cm=\\EY%+ %+ " }
    { "capability" = "cr=^M" }
    { "capability" = "dl=\\EM" }
    { "capability" = "do=\\EB" }
    { "capability" = "ho=\\EH" }
    { "capability" = "k1=\\EP" }
    { "capability" = "k2=\\EQ" }
    { "capability" = "k3=\\ER" }
    { "capability" = "k4=\\ES" }
    { "capability" = "k5=\\ET" }
    { "capability" = "k6=\\EU" }
    { "capability" = "k7=\\EV" }
    { "capability" = "k8=\\EW" }
    { "capability" = "k9=\\EX" }
    { "capability" = "k;=\\EY" }
    { "capability" = "kD=\\177" }
    { "capability" = "kI=\\EI" }
    { "capability" = "kN=\\Eb" }
    { "capability" = "kP=\\Ea" }
    { "capability" = "kb=^H" }
    { "capability" = "kd=\\EB" }
    { "capability" = "kh=\\EE" }
    { "capability" = "kl=\\ED" }
    { "capability" = "kr=\\EC" }
    { "capability" = "ku=\\EA" }
    { "capability" = "le=^H" }
    { "capability" = "mb=\\Er" }
    { "capability" = "md=\\EyA" }
    { "capability" = "me=\\Ez_" }
    { "capability" = "mh=\\Em" }
    { "capability" = "mr=\\Ep" }
    { "capability" = "nd=\\EC" }
    { "capability" = "nw=2*\\r\\n" }
    { "capability" = "op=\\Eb@\\EcO" }
    { "capability" = "r1=\\Ez_\\Eb@\\EcA" }
    { "capability" = "se=\\Eq" }
    { "capability" = "sf=2*\\n" }
    { "capability" = "so=\\Ep" }
    { "capability" = "sr=2*\\EI" }
    { "capability" = "ta=^I" }
    { "capability" = "te=\\Ev\\E. \\Ee\\Ez_" }
    { "capability" = "ti=\\Ev\\Ee\\Ez_" }
    { "capability" = "ue=\\EzH" }
    { "capability" = "up=\\EA" }
    { "capability" = "us=\\EyH" }
    { "capability" = "ve=\\E. \\Ee" }
    { "capability" = "vi=\\Ef" }
    { "capability" = "vs=\\E.\"" }
  }

let termcap4 = "rbcomm|IBM PC with RBcomm and EMACS keybindings:\\
        :am:bw:mi:ms:xn:\\
        :co#80:it#8:li#25:\\
        :AL=\\E[%dL:DL=\\E[%dM:al=^K:bl=^G:bt=\\E[Z:cd=^F5:ce=^P^P:\\
        :cl=^L:cm=\\037%r%+ %+ :cr=^M:cs=\\E[%i%d;%dr:dc=^W:dl=^Z:\\
        :dm=:do=^C:ec=\\E[%dX:ed=:ei=^]:im=^\\:\\
        :is=\\017\\035\\E(B\\E)0\\E[?7h\\E[?3l\\E[>8g:kb=^H:kd=^N:\\
        :ke=\\E>:kh=^A:kl=^B:kr=^F:ks=\\E=:ku=^P:le=^H:mb=\\E[5m:\\
        :md=\\E[1m:me=\\E[m:mk=\\E[8m:mr=^R:nd=^B:nw=^M\\ED:\\
        :r1=\\017\\E(B\\E)0\\025\\E[?3l\\E[>8g:rc=\\E8:rp=\\030%.%.:\\
        :sc=\\E7:se=^U:sf=\\ED:so=^R:sr=\\EM:ta=^I:te=:ti=:ue=^U:up=^^:\\
        :us=^T:ve=\\E[?25h:vi=\\E[?25l:
"

test Termcap.lns get termcap4 =
  { "record"
    { "name" = "rbcomm" }
    { "name" = "IBM PC with RBcomm and EMACS keybindings" }
    { "capability" = "am" }
    { "capability" = "bw" }
    { "capability" = "mi" }
    { "capability" = "ms" }
    { "capability" = "xn" }
    { "capability" = "co#80" }
    { "capability" = "it#8" }
    { "capability" = "li#25" }
    { "capability" = "AL=\\E[%dL" }
    { "capability" = "DL=\\E[%dM" }
    { "capability" = "al=^K" }
    { "capability" = "bl=^G" }
    { "capability" = "bt=\\E[Z" }
    { "capability" = "cd=^F5" }
    { "capability" = "ce=^P^P" }
    { "capability" = "cl=^L" }
    { "capability" = "cm=\\037%r%+ %+ " }
    { "capability" = "cr=^M" }
    { "capability" = "cs=\\E[%i%d;%dr" }
    { "capability" = "dc=^W" }
    { "capability" = "dl=^Z" }
    { "capability" = "dm=" }
    { "capability" = "do=^C" }
    { "capability" = "ec=\\E[%dX" }
    { "capability" = "ed=" }
    { "capability" = "ei=^]" }
    { "capability" = "im=^\\" }
    { "capability" = "is=\\017\\035\\E(B\\E)0\\E[?7h\\E[?3l\\E[>8g" }
    { "capability" = "kb=^H" }
    { "capability" = "kd=^N" }
    { "capability" = "ke=\\E>" }
    { "capability" = "kh=^A" }
    { "capability" = "kl=^B" }
    { "capability" = "kr=^F" }
    { "capability" = "ks=\\E=" }
    { "capability" = "ku=^P" }
    { "capability" = "le=^H" }
    { "capability" = "mb=\\E[5m" }
    { "capability" = "md=\\E[1m" }
    { "capability" = "me=\\E[m" }
    { "capability" = "mk=\\E[8m" }
    { "capability" = "mr=^R" }
    { "capability" = "nd=^B" }
    { "capability" = "nw=^M\\ED" }
    { "capability" = "r1=\\017\\E(B\\E)0\\025\\E[?3l\\E[>8g" }
    { "capability" = "rc=\\E8" }
    { "capability" = "rp=\\030%.%." }
    { "capability" = "sc=\\E7" }
    { "capability" = "se=^U" }
    { "capability" = "sf=\\ED" }
    { "capability" = "so=^R" }
    { "capability" = "sr=\\EM" }
    { "capability" = "ta=^I" }
    { "capability" = "te=" }
    { "capability" = "ti=" }
    { "capability" = "ue=^U" }
    { "capability" = "up=^^" }
    { "capability" = "us=^T" }
    { "capability" = "ve=\\E[?25h" }
    { "capability" = "vi=\\E[?25l" }
  }

let termcap5 = "rxvt+pcfkeys|fragment for PC-style fkeys:\\
        :#2=\\E[7$:#3=\\E[2$:#4=\\E[d:%c=\\E[6$:%e=\\E[5$:%i=\\E[c:\\
        :*4=\\E[3$:*6=\\E[4~:*7=\\E[8$:@0=\\E[1~:@7=\\E[8~:F1=\\E[23~:\\
        :F2=\\E[24~:F3=\\E[25~:F4=\\E[26~:F5=\\E[28~:F6=\\E[29~:\\
        :F7=\\E[31~:F8=\\E[32~:F9=\\E[33~:FA=\\E[34~:FB=\\E[23$:\\
        :FC=\\E[24$:FD=\\E[11\\136:FE=\\E[12\\136:FF=\\E[13\\136:FG=\\E[14\\136:\\
        :FH=\\E[15\\136:FI=\\E[17\\136:FJ=\\E[18\\136:FK=\\E[19\\136:FL=\\E[20\\136:\\
        :FM=\\E[21\\136:FN=\\E[23\\136:FO=\\E[24\\136:FP=\\E[25\\136:FQ=\\E[26\\136:\\
        :FR=\\E[28\\136:FS=\\E[29\\136:FT=\\E[31\\136:FU=\\E[32\\136:FV=\\E[33\\136:\\
        :FW=\\E[34\\136:FX=\\E[23@:FY=\\E[24@:k1=\\E[11~:k2=\\E[12~:\\
        :k3=\\E[13~:k4=\\E[14~:k5=\\E[15~:k6=\\E[17~:k7=\\E[18~:\\
        :k8=\\E[19~:k9=\\E[20~:k;=\\E[21~:kD=\\E[3~:kE=\\E[8\\136:kF=\\E[a:\\
        :kI=\\E[2~:kN=\\E[6~:kP=\\E[5~:kR=\\E[b:kd=\\E[B:kh=\\E[7~:\\
        :kl=\\E[D:kr=\\E[C:ku=\\E[A:
"

test Termcap.lns get termcap5 =
  { "record"
    { "name" = "rxvt+pcfkeys" }
    { "name" = "fragment for PC-style fkeys" }
    { "capability" = "#2=\\E[7$" }
    { "capability" = "#3=\\E[2$" }
    { "capability" = "#4=\\E[d" }
    { "capability" = "%c=\\E[6$" }
    { "capability" = "%e=\\E[5$" }
    { "capability" = "%i=\\E[c" }
    { "capability" = "*4=\\E[3$" }
    { "capability" = "*6=\\E[4~" }
    { "capability" = "*7=\\E[8$" }
    { "capability" = "@0=\\E[1~" }
    { "capability" = "@7=\\E[8~" }
    { "capability" = "F1=\\E[23~" }
    { "capability" = "F2=\\E[24~" }
    { "capability" = "F3=\\E[25~" }
    { "capability" = "F4=\\E[26~" }
    { "capability" = "F5=\\E[28~" }
    { "capability" = "F6=\\E[29~" }
    { "capability" = "F7=\\E[31~" }
    { "capability" = "F8=\\E[32~" }
    { "capability" = "F9=\\E[33~" }
    { "capability" = "FA=\\E[34~" }
    { "capability" = "FB=\\E[23$" }
    { "capability" = "FC=\\E[24$" }
    { "capability" = "FD=\\E[11\\136" }
    { "capability" = "FE=\\E[12\\136" }
    { "capability" = "FF=\\E[13\\136" }
    { "capability" = "FG=\\E[14\\136" }
    { "capability" = "FH=\\E[15\\136" }
    { "capability" = "FI=\\E[17\\136" }
    { "capability" = "FJ=\\E[18\\136" }
    { "capability" = "FK=\\E[19\\136" }
    { "capability" = "FL=\\E[20\\136" }
    { "capability" = "FM=\\E[21\\136" }
    { "capability" = "FN=\\E[23\\136" }
    { "capability" = "FO=\\E[24\\136" }
    { "capability" = "FP=\\E[25\\136" }
    { "capability" = "FQ=\\E[26\\136" }
    { "capability" = "FR=\\E[28\\136" }
    { "capability" = "FS=\\E[29\\136" }
    { "capability" = "FT=\\E[31\\136" }
    { "capability" = "FU=\\E[32\\136" }
    { "capability" = "FV=\\E[33\\136" }
    { "capability" = "FW=\\E[34\\136" }
    { "capability" = "FX=\\E[23@" }
    { "capability" = "FY=\\E[24@" }
    { "capability" = "k1=\\E[11~" }
    { "capability" = "k2=\\E[12~" }
    { "capability" = "k3=\\E[13~" }
    { "capability" = "k4=\\E[14~" }
    { "capability" = "k5=\\E[15~" }
    { "capability" = "k6=\\E[17~" }
    { "capability" = "k7=\\E[18~" }
    { "capability" = "k8=\\E[19~" }
    { "capability" = "k9=\\E[20~" }
    { "capability" = "k;=\\E[21~" }
    { "capability" = "kD=\\E[3~" }
    { "capability" = "kE=\\E[8\\136" }
    { "capability" = "kF=\\E[a" }
    { "capability" = "kI=\\E[2~" }
    { "capability" = "kN=\\E[6~" }
    { "capability" = "kP=\\E[5~" }
    { "capability" = "kR=\\E[b" }
    { "capability" = "kd=\\E[B" }
    { "capability" = "kh=\\E[7~" }
    { "capability" = "kl=\\E[D" }
    { "capability" = "kr=\\E[C" }
    { "capability" = "ku=\\E[A" }
  }

