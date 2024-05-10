# scheme-tokenizer

Это первая часть scheme. 

## Что сделано?

Написан токенизатор языка Scheme. Токенизатор принимает на вход последовательность символов и возвращает последовательность токенов.

## Виды токенов

- **Число:** `42`, `-4` или `+10`
- **Скобка:** `(` или `)`
- **Quote:** `'`
- **Dot:** `.`
- **Symbol:** Начинается с символов `[a-zA-Z<=>*/#]` и может содержать внутри символы `[a-zA-Z<=>*/#0-9?!-]`. Отдельные
  знаки `+` и `-` тоже считаются символами.
- **Boolean** - либо `#t`, либо `#f`.

### Пример выражения и потока токенов

```
'(+ 4 -5)

Quote OpenParen Symbol(+) Const(4) Const(-5) CloseParen
```

