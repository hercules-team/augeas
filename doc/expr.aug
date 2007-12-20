grammar {
  token PLUS '+'
  token TIMES '*'

  # first: '(', 'var'
  # follow: ')'
  expr: term expr1

  # first: PLUS, <>
  # follow: ')'
  expr1: (PLUS term)*
  
  # first: '(', 'var'
  # follow: PLUS, ')'
  term: factor term1

  # first: TIMES, <>
  # follow: PLUS, ')'
  term1: (TIMES factor)*

  # first: '(', 'var'
  # follow: TIMES, PLUS, ')'
  factor: '(' expr ')' | ...
}
