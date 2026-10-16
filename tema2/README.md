# Evaluator eBNF

Programul evaluează expresii cu numere întregi nenegative, adunări și
paranteze, câte o expresie pe linie.

Gramatica din imagine este:

```text
Z ::= E .
E ::= T ( '+' T )* .
T ::= i | '(' E ')' .
```

Punctul încheie regula gramaticală; nu se introduce în expresie.
`i` reprezintă un număr, recunoscut de lexer ca tokenul `INTEGER`.

## Cum funcționează

- `next_symbol()` citește următorul token în variabila `symbol`.
- `Z()` apelează `E()`, verifică sfârșitul liniei sau al fișierului și afișează rezultatul.
- `E()` citește primul termen cu `T()`. Cât timp urmează `+`, consumă semnul și adună următorul termen. Bucla implementează repetiția `*` din eBNF și elimină recursivitatea la stânga din `E -> E+T | T`.
- `T()` salvează valoarea unui număr înainte de a avansa lexerul sau consumă `(`, evaluează recursiv `E()` și cere `)`.
- Lexerul ignoră spațiile și taburile. Liniile goale sunt sărite în `main()`.

Expresiile incorecte produc un mesaj pe stderr și opresc programul cu un cod de eroare.
Calculele folosesc tipul C `int`; exemplul de laborator presupune valori și sume care încap în acest tip.

## Rulare

Ai nevoie de Flex, un compilator C și Make:

```sh
make run
```

Introdu expresiile, fiecare pe o linie. Închide intrarea cu Ctrl+D.

```text
Intrare:       Rezultat:
1+2+3          6
4+(5+6)        15
4              4
```

`make run` generează `lexer.c` și `lexer.h`, compilează executabilul `prog`
și îl pornește. Poți rula apoi direct `./prog` sau `./prog < input.txt`.
`make clean` șterge fișierele generate.
