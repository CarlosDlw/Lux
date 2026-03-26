# Operadores em TM

## Aritméticos

- `+` - adição
- `-` - subtração
- `*` - multiplicação
- `/` - divisão
- `%` - módulo

## Atribuição

- `=` - atribuição simples
- `+=` - adição e atribuição
- `-=` - subtração e atribuição
- `*=` - multiplicação e atribuição
- `/=` - divisão e atribuição
- `%=` - módulo e atribuição

- `&=` - AND bit a bit e atribuição
- `|=` - OR bit a bit e atribuição
- `^=` - XOR bit a bit e atribuição
- `<<=` - deslocamento à esquerda e atribuição
- `>>=` - deslocamento à direita e atribuição

## Comparação / relacional

- `==` - igualdade
- `!=` - desigualdade
- `<`  - menor que
- `>`  - maior que
- `<=` - menor ou igual a
- `>=` - maior ou igual a

## Lógicos

- `&&` - AND lógico
- `||` - OR lógico
- `!`  - NOT lógico

## Bit a bit

- `&`  - AND bit a bit
- `|`  - OR bit a bit
- `^`  - XOR bit a bit
- `~`  - NOT bit a bit
- `<<` - deslocamento à esquerda
- `>>` - deslocamento à direita

## Incremento / Decremento

- `++` - incremento (pré e pós)
- `--` - decremento (pré e pós)

## Acesso a referencia

- `.` - Acesso a membro
- `->` - Acesso a membro via ponteiro
- `[]` - Indexação de arrays
- `()` - Chamada de função
- `&` - Endereço de variável
- `*` - Desreferenciação de ponteiro

## Ternário / Condicional

- `? :` - Operador ternário (condição ? valor_se_verdadeiro : valor_se_falso)

## Tipo / Cast

- `as` - Cast de tipo (e.g. `x as int32`)
- `is` - Verificação de tipo (e.g. `x is int32`)
- `sizeof` - Tamanho em bytes de um tipo (e.g. `sizeof(int32)`)
- `typeof` - Tipo de uma expressão (e.g. `typeof(x)`)

## Range / Slicing

- `..` - Operador de intervalo exclusivo (e.g. `1..10` para criar um range de 1 a 9)
- `..=` - Operador de intervalo inclusivo (e.g. `1..=10` para criar um range de 1 a 10)

## Outros / Uteis

- `??` - Operador de coalescência nula (e.g. `x ?? default_value` retorna `x` se não for nulo, caso contrário retorna `default_value`)
- `::` - Operador de escopo para acessar membros de namespaces
- `...` - Spread operator para expandir arrays ou argumentos de função