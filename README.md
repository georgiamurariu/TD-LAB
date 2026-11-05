# Tema 4 — Parser si generare LLVM IR pentru intregi

Programul citeste o expresie, construieste arborele AST si genereaza `output.ll`.
Pastreaza structura scheletului de laborator si foloseste intregi cu semn pe 32 de biti (`i32`).

## Rulare pe acest Mac

Din folderul proiectului:

```sh
make
make run
```

Scrie o expresie si apasa Enter, de exemplu:

```text
(2 + 3) * 4
```

Vei vedea arborele, codul LLVM si `Result is: 20`.
Poti rula si fara introducere interactiva:

```sh
printf '20 / 3 + 20 %% 3\n' | make run
```

Rezultatul este `8`. In comanda `printf`, `%%` afiseaza un singur `%`.
Pentru a vedea doar generarea codului: `./main`. Codul salvat se vede cu `cat output.ll`.
Pentru curatare: `make clean`.

Makefile-ul detecteaza LLVM 17 instalat cu Homebrew sau `llvm-config-17` / `llvm-config` din PATH.
Daca LLVM este in alta locatie: `make LLVM_CONFIG=/cale/catre/llvm-config`.
Sunt necesare LLVM (inclusiv clang++ si lli), Flex si Make.

## Cum functioneaza

1. `lexer.l` transforma textul in tokenuri: numere, operatori si paranteze. Ignora spatiile.
2. Parserul din `main.cpp` respecta gramatica din PDF:

   ```text
   Z     ::= E_AS
   E_AS  ::= E_MDR (('+' | '-') E_MDR)*
   E_MDR ::= T (('*' | '/' | '%') T)*
   T     ::= numar | '(' E_AS ')'
   ```

3. `NumberASTNode` retine un numar; `BinaryExprAST` retine operatorul si cei doi operanzi.
   Ambii mostenesc `GenericASTNode`. `unique_ptr` elibereaza automat memoria arborelui.
4. `codegen()` parcurge arborele si produce LLVM IR. Numarul devine `ConstantInt`;
   operatorii devin `CreateAdd`, `CreateSub`, `CreateMul`, `CreateSDiv`, `CreateSRem`.
5. `CodeGenTopLevel()` pune expresia intr-o functie `i32 @main()`, verifica functia
   si o scrie in `output.ll`. `lli` executa acest fisier.

Pentru `2 + 3 * 4`, AST-ul este `(2 + (3 * 4))`: inmultirea este construita prima.
Buclele din parser asigura asocierea la stanga: `20 - 5 - 3` inseamna `(20 - 5) - 3`.
`NoFolder` pastreaza instructiunile vizibile in IR, fara evaluarea lor la generare.

## Reguli si limite

- Se citeste o singura expresie, terminata cu Enter sau EOF; `=` este optional inainte de final.
- Literalii sunt intre 0 si 2147483647. Conform gramaticii, minusul unar nu exista;
  pentru un numar negativ scrie `(0 - 7)`.
- Impartirea este intreaga, cu trunchiere spre zero: `7 / 3` da `2`.
- Nu folosi impartire/rest la zero sau `INT_MIN / -1`: operatiile LLVM nu definesc
  un rezultat valid pentru aceste cazuri. Nu exista verificari aritmetice la executie.
- Adunarea, scaderea si inmultirea lucreaza pe 32 de biti; nu raporteaza overflow.
- Rezultatul este returnat ca status de proces: shell-ul afiseaza doar valoarea modulo 256.
  De exemplu, 300 apare ca 44, iar -2 apare ca 254. Pentru exemple usor de verificat,
  foloseste rezultate intre 0 si 255.

## Surse

- https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl02.html — clasele AST.
- https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl03.html — generarea IR,
  adaptata de la `double` / `ConstantFP` / operatii floating-point la `i32` si operatii intregi.
