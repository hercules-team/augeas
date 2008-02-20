# /etc/sshd/sshd_config

map
  grammar sshd
  include '/etc/ssh/sshd_config' '/system/config/sshd'
end

grammar sshd
  token EOL /[ \t]*\n/ = '\n'
  token SEP /[ \t]+/ = ' '
  token COMMENT /(#.*|[ \t]*)\n/ = '\n'
  token KEY /[^ \t]+/ = ''
  token VALUE /([^ \t\n]|[ \t][^ \t\n])+/ = ''
  token MAC /([^, \t\n]|[ \t][^, \t\n])+/ = ''
  start: (comment | accept_env | allow_groups | allow_users 
          | deny_groups | deny_users | macs | match | other_entry ) *

  comment: [ COMMENT ]

  other_entry: [ key KEY . SEP . store VALUE . EOL ]

  accept_env: [ key 'AcceptEnv' .
              ([ SEP . seq 'accept_env' . store VALUE])* .
              EOL]

  allow_groups: [ key 'AllowGroups' .
              ([ SEP . seq 'allow_groups' . store VALUE])* .
              EOL]
  
  allow_users: [ key 'AllowUsers' .
              ([ SEP . seq 'allow_users' . store VALUE])* .
              EOL]
  
  deny_groups: [ key 'DenyGroups' .
              ([ SEP . seq 'deny_groups' . store VALUE])* .
              EOL]
  
  deny_users: [ key 'DenyUsers' .
              ([ SEP . seq 'deny_users' . store VALUE])* .
              EOL]

  macs: [ key 'MACs' . SEP .
           [ seq 'macs' . store MAC ] .
           ([ seq 'macs' . ',' . store MAC])* .
          EOL ]

  match: [ key 'Match' .
           [ seq 'match' .
             [ label 'cond' . store VALUE . EOL ] .
             (SEP . other_entry) * ] ]

end
