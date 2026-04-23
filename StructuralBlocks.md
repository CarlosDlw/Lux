# Structural Blocks System

É um mecanismo de controle de fluxo e escopo com variações semânticas, com 3 construções distintas.

## `#inline {}`

- Semântica:
    - Remove o bloco no AST
    - Injeta statements diretamente no escopo pai
    - Não cria escopo
    - Não altera lifetime

Exemplo:

```lux
int32 main() {
    auto a = 10;
    #inline {
        auto b = 20;
        printf(c"%d\n", a + b); // válido, 'a' é visível e 'b' é injetado no escopo pai, por isso é visivel no escopo pai também.
    }
    ret 0;
}
```

Lowering:

```lux
int32 main() {
    auto a = 10;
    auto b = 20; // injetado no escopo pai
    printf(c"%d\n", a + b);
    ret 0;
}
```

- Features:
    - Zero overhead
    - Útil para organização visual
    - Ideal para macros estruturais sem macro

- Restrições:
    - Proibir redeclaração no mesmo escopo
    - nenhuma proteção de lifetime


## `{}` Escopo normal

- Semântica:
    - Cria escopo léxico
    - Variáveis vivem até o fim do bloco
    - Permite shadowing

Exemplo:

```lux
int32 main() {
    auto a = 10;
    {
        auto a = 20; // shadowing permitido
        printf(c"%d\n", a); // 20
    }
    printf(c"%d\n", a); // 10
    ret 0;
}
```

- Features:
    - Previsível e familiar
    - Compativel com C-like syntax
    - Base para otimizações (stack, SSA)


## `#scope (...) {}`

- Semântica:
    - Cria escopo normal
    - Registral callbacks de saida (defer implicito)
    - Executa sempre ao sair do bloco, mesmo com panics ou returns

Exemplo utilizando junto de raylib

```lux
int32 main() {
    InitWindow(800.0, 600.0, c"Lux Raylib Example");
    
    while !WindowShouldClose() {
        #scope (EndDrawing()) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText(c"Hello, Lux!", 190.0, 200.0, 20.0, LIGHTGRAY);
        }
    }

    ret 0;
}
```

- Features:
    - Multi-calls `#scope (A(), B(), C()) {}`: Ordem de execução é garantida (C, B, A)
    - Segurança: Garante pareamento correto
    - Compatível com early-exit (returns, panics, etc)

Composição

```lux
#scope (A()) {
    #scope (B()) {
        ...
    }
}
```

- Execução: B(), A() garantida mesmo com early exit

Captura de variáveis

```lux
#scope (free(buf)) {
    auto buf = alloc(); // Requesito: variavel deve existir até o momento do defer
}
```

Zero runtime (se bem implementado)
resolvido em compile-time
vira sequência explícita no IR


## Comparação direta

| Feature           | `#inline` | `{}` | `#scope`   |
| ----------------- | --------- | ---- | ---------- |
| Cria escopo       | não       | sim  | sim        |
| Lifetime isolado  | não       | sim  | sim        |
| Execução no exit  | não       | não  | sim        |
| Early-exit seguro | não       | não  | sim        |
| Overhead runtime  | zero      | zero | zero*      |
| Uso principal     | estética  | base | RAII/defer |


## Considerações de implementação

- `#inline` é simples: apenas injeta statements no escopo pai, sem criar escopo ou alterar lifetimes.
- `{}` é o bloco tradicional, com escopo léxico e regras de shadowing.
- `#scope` é o mais complexo: callbacks de saida não são obrigatórios, mas garantem execução mesmo em casos de early exit.

## Conclusão

Isso são `Structural Blocks` — um sistema de blocos com variações semânticas para controle de fluxo, escopo e gerenciamento de recursos. Cada construção tem seu propósito e trade-offs, permitindo que os desenvolvedores escolham a ferramenta certa para cada situação.