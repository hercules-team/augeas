# A complicated format: cmfm.conf

map
  grammar cmfm
  include '/etc/cmfm.conf' '/system/config/cmfm'
end

grammar cmfm
  # Matches whitespace and comments
  token CWS     /([ \n\t]+|\/\*([^\*]|\*[^\/])*\*\/)*/ = ''

  token WORD    /[a-zA-Z_0-9]+/ = ''

  # Boomerang definition:
  # let pci_elt = "(" . word . ("," . cws_re . word){3} . ")"
  # token PCI_ELT /\(WORD(,CWS WORD){3}\)/
  # let pci_array = "(" . cws_re . pci_elt . ("," . cws_re . pci_elt)* . cws_re . ")"
  # token PCI_ARRAY /\(CWS PCI_ELT (,CWS PCI_ELT)* CWS\)/
  # let value = (word | "\"" . [^"]* . "\"" | pci_array )

  # the above manually expanded and turned into a POSIX regexp since
  # augeas can't concat regexps:
  token VALUE   /([a-zA-Z_0-9]+|"[^"]*"|\(([ \n\t]+|\/\*([^\*]|\*[^\/])*\*\/)*\([a-zA-Z_0-9]+(,([ \n\t]+|\/\*([^\*]|\*[^\/])*\*\/)*[a-zA-Z_0-9]+){3}\)(,([ \n\t]+|\/\*([^\*]|\*[^\/])*\*\/)*\([a-zA-Z_0-9]+(,([ \n\t]+|\/\*([^\*]|\*[^\/])*\*\/)*[a-zA-Z_0-9]+){3}\))*([ \n\t]+|\/\*([^\*]|\*[^\/])*\*\/)*\))/ = '""'

  file: CWS . (entry3)*

  assignment: [ key WORD . CWS . '=' . CWS . store VALUE . ';' . CWS ]

  # Approximate the nesting of sections up to a max of 3
  entry1: assignment | sec1
  sec1: [label '{1}' . [ key WORD . ':' . CWS . '{' . CWS . (assignment)* . '}' . ';' . CWS] ]

  entry2: assignment | sec2
  sec2: [label '{2}' . [ key WORD . ':' . CWS . '{' . CWS . (entry1)* . '}' . ';' . CWS] ]

  entry3: assignment | sec3
  sec3: [ label '{3}' . [ key WORD . ':' . CWS . '{' . CWS . (entry2)* . '}' . ';' . CWS] ]
end
