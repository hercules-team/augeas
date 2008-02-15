# Parsing /etc/hosts

map
  grammar hosts
  include '/etc/hosts' '/system/config/hosts'
end

grammar hosts

  token SEPTAB /[ \t]+/ = '\t'
  token SEPSPC /[ \t]+/ = ' '
  token OPT_WS /[ \t]*/ = '\t'
  token EOR '\n'
  token POUND_TO_EOL /#.*\n/ = '# '

  file: counter 'record' . ( comment | record ) *

  comment: [ store POUND_TO_EOL ]

  record: [ seq 'record' .
            [ label 'ipaddr' . store ... . SEPTAB ] . 
            [ label 'canonical' . store ... ] . 
            [ label 'aliases' . counter 'aliases' .
              ( [ seq 'aliases' . SEPSPC . store ... ] ) *
            ]
            . EOR
         ]
end

# RHS of rules:
#
# match = match_seq | match '|' match_seq
# match_seq = match_prim | match_seq '.' match_prim
# match_prim = '[' match ']'
#            | '(' match ')' match_quant
#            | literal
#            | action
# action = FUNC term
# term = literal | NAME | ANY
#
# Functions:
# counter LITERAL:QUOTED
# seq     LITERAL:QUOTED
# label   LITERAL:QUOTED
# store   LITERAL | ABBREV_REF | ANY
# key     LITERAL | ABBREV_REF | ANY
