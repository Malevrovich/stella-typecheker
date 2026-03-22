# Golden Tests for Stella Typechecker

Функциональное регрессионное тестирование типчекера языка Stella с использованием Golden Test Framework.

## Использование

### Запуск всех тестов

```bash
python3 tests/golden_tester.py
```

Сравнивает вывод типчекера со сохраненными golden файлами (`.golden`).

### Обновление golden файлов

```bash
python3 tests/golden_tester.py --update
```

Перегенерирует все golden файлы с текущим выводом типчекера. **Используйте с осторожностью!**

### Фильтрация тестов

```bash
python3 tests/golden_tester.py --filter simple
```

Запускает только тесты, в названии которых содержится "simple".

### Опции

```
-h, --help              Show help message
--binary PATH           Path to typechecker binary (default: ./build/typechecker/cli/typechecker)
--stella-dir PATH       Directory with .stella test files (default: ./stella-examples)
--golden-dir PATH       Directory with .golden files (default: ./tests/golden)
--update                Update all golden files
--filter PATTERN        Filter tests by name pattern
```

## Структура

### Test Files
- `stella-examples/` - примеры программ на языке Stella (как well-typed, так и ill-typed)
- `golden/` - golden файлы с ожидаемыми результатами

### Формат golden файла

```
STDOUT:
Well-typed

STDERR:
(empty)

EXIT_CODE: 0
```

Каждый golden файл содержит:
- **STDOUT:** - стандартный вывод (или "(empty)" если пусто)
- **STDERR:** - вывод ошибок (или "(empty)" если пусто)
- **EXIT_CODE:** - код выхода программы

## Typechecker Flags

### Обычное использование (без логов)

```bash
./build/typechecker/cli/typechecker filename.stella
```

Выводит только результат типизации (Well-typed) или ошибку.

### Verbose режим (с debug логами)

```bash
./build/typechecker/cli/typechecker -v filename.stella
./build/typechecker/cli/typechecker --verbose filename.stella
```

Включает подробный вывод debug информации для диагностики.

## Примеры тестов

- **simple.stella** - простая well-typed программа
- **error_unexpected_type_for_parameter.stella** - ошибка типизации
- **name_shadowing.stella** - переопределение переменной
- **no_main.stella** - отсутствие функции main
- **functions_confict.stella** - конфликт имен функций
- **semicolon.stella** - синтаксическая ошибка

## How It Works

1. **Нормализация вывода**: Timestamp и адреса памяти удаляются перед сравнением (детерминированное тестирование)
2. **Сравнение**: Фактический вывод типчекера сравнивается с golden файлом
3. **Звуки**: ✅ для успешных тестов, ❌ для ошибок
4. **Отчет**: Итоговый отчет с количеством пройденных/не пройденных тестов

## Tips

- Используйте `--filter` для быстрого тестирования конкретного теста во время разработки
- При добавлении новых тестовых файлов, используйте `--update` для создания golden файлов
- Всегда проверяйте diff перед коммитом golden файлов!
