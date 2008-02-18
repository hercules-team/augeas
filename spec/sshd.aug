# /etc/sshd/sshd_config

map
  grammar sshd
  include '/etc/ssh/sshd_config' '/system/config/sshd'
end

grammar sshd
  token EOL /[ \t]*\n/ = '\n'
  token SEP /[ \t]+(?!\n)/ = ' '
  token COMMENT /(#.*|[ \t]*)\n/ = '\n'

  start: (comment | accept_env | allow_groups | allow_users 
          | deny_groups | macs | match | other_entry ) *

  comment: [ COMMENT ]

  other_entry: [ key ... . SEP . store ... . EOL ]

  accept_env: [ key 'AcceptEnv' .
              ([ SEP . seq 'accept_env' . store ...])* .
              EOL]

  allow_groups: [ key 'AllowGroups' .
              ([ SEP . seq 'allow_groups' . store ...])* .
              EOL]
  
  allow_users: [ key 'AllowUsers' .
              ([ SEP . seq 'allow_users' . store ...])* .
              EOL]
  
  deny_groups: [ key 'DenyGroups' .
              ([ SEP . seq 'deny_groups' . store ...])* .
              EOL]
  
  deny_users: [ key 'DenyUsers' .
              ([ SEP . seq 'deny_users' . store ...])* .
              EOL]

  macs: [ key 'MACs' . SEP .
           [ seq 'macs' . store ... ] .
           ([ seq 'macs' . ',' . store ...])* .
          EOL ]

  match: [ key 'Match' .
           [ seq 'match' .
             [ label 'cond' . store ... . EOL ] .
             (other_entry) * ] ]

end