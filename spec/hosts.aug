# Parsing /etc/hosts

# This file creates a subtree that looks like
# hosts/
#       0/
#         ipaddr = 127.0.0.1
#         canonical = localhost
#         aliases/
#           0 = localhost.localdomain
#           1 = host.example.com
#       1/
#         ... similar subtree for the next line from /etc/hosts
 
# A map describes which files are parsed with which grammar. There
# can be several include statements and they can contain glob patterns
map {
  grammar hosts
  include '/etc/hosts' '/system/config/hosts'
}

# A grammar describes how a config file is to be parsed and how it
# gets mapped into the tree. It consists of a number of token definitions
# and grammar rules.
grammar hosts {

  # Token definitions are of the form 'token NAME (REGEX|QUOTED) (= QUOTED)?'
  # When the token is defined as a regular expression, the user needs to 
  # indicate what the value for a brand new token is for the case that
  # entries are added to the tree that were not in the initial config
  # file.
  token SEP /[ \t]+/ = '\t'
  token SEP2 /[ \t]+/ = ' '
  token OPT_WS /[ \t]*/ = '\t'
  token EOR '\n'
  token POUND_TO_EOL /#.*\n/ = '# '

  # Rules are productions in a context free grammar. The RHS can contain
  # constructs such as 'a|b', '(a)*', (a)+', '(a)?' with the same meaning
  # as in regexps.
  #
  # Each rule can have a number of actions attached to it (in '{ .. }') An
  # action describes how entries from the config file are mapped into the
  # tree during parsing. Each action is of the form '@FIELD { PATH [ =
  # VALUE ] }' FIELD is either $N denoting the N-th field (starting from 1)
  # on the RHS of the rule or a number N denoting the N-th parenthesized
  # group on the RHS where group 0 is the whole RHS. During parsing, the
  # parser maintains a current node in the tree. When an action specifies a
  # PATH, that path is added to the current node and gives the current node
  # during parsing of the construct for the action. The VALUE specifies
  # what is assigned to the current node right after the corresponding
  # construct has been parsed.
  #
  # The first rule is always the start symbol of the grammar: a hosts file
  # consists of any number of comments and records. The action for this
  # rule says 'during parsing of 'file', the current node is whatever the
  # parser was started with ('/system/config' right now) + 'hosts'
  file: ( comment | record ) *

  # A comment has no action, and doesn't map into the tree at all. Whatever
  # was parsed for it is kept though and written back out when the subtree
  # for /etc/hosts is written out
  comment: POUND_TO_EOL

  # A record consists of a few entries seprated by SEP. The '...' matches
  # anything up to the following token. The @0 action makes sure that each
  # record is parsed into a new subtree. $seq creates a new node whose name
  # is a number, the remaining actions assign field values from the record
  # to the tree.
  record: ... SEP ... ( SEP2 ... ) * EOR { 
          @0  { $seq }
          @$1 { 'ipaddr' = $1 }
          @$3 { 'canonical' = $3 }
          @$5 { 'aliases' $seq = $5 } 
  }

}

# Alternative syntax:
# module hosts
#   def apply = lens (include '/etc/hosts')
#
#   def SEP(default:string) = match /[ \t]+/ default
#   def SEPTAB = SEP '\t'
#   def SEPSPC = SEP ' '
#   def OPT_WS = match /[ \t]*/ '\t'
#   def EOL = match '\n'
#   def POUND_TO_EOL = match /#.*\n/ '# '
#
#   def assign path value = begin path . store value . end
#   def lens    = begin :hosts . ( comment | record ) * . end
#   def comment = POUND_TO_EOL
#   def record  = begin $seq . assign :ipaddr ... . SEPTAB 
#                            . assign :canonical ... 
#                            . begin :aliases 
#                                    . (SEPSPC . assign $seq ...) *
#                            . end . EOL .
#                 end
# hosts.apply
