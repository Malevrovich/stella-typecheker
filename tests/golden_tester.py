#!/usr/bin/env python3
"""
Golden Test Framework for Stella Typechecker

Запускает примеры на типчекере и сравнивает вывод с золотыми файлами (.golden).
"""

import os
import sys
import subprocess
import argparse
import re
from pathlib import Path
from dataclasses import dataclass
from typing import Optional, Tuple


@dataclass
class TestOutput:
    """Результат выполнения типчекера"""
    stdout: str
    stderr: str
    exit_code: int
    input_code: str = ""  # Содержимое исходного .stella файла
    stderr_without_logs: str = ""
    
    def __str__(self) -> str:
        """Форматирование для сравнения"""
        return f"STDOUT:\n{self.stdout}\n\nSTDERR:\n{self.stderr}\n\nEXIT_CODE: {self.exit_code}"


def run_typechecker(binary_path: str, stella_file: str, strip_logs: bool = False) -> TestOutput:
    """
    Запускает типчекер на файле и возвращает результат.
    
    Args:
        binary_path: Путь к бинарнику типчекера
        stella_file: Путь к .stella файлу
        strip_logs: Удалять ли debug-логи из stderr
    
    Returns:
        TestOutput с stdout, stderr и exit code
    """
    try:
        result = subprocess.run(
            [binary_path, stella_file],
            capture_output=True,
            text=True,
            timeout=10
        )
        
        stderr = result.stderr.strip() if result.stderr.strip() else "(empty)"
        stdout = result.stdout.strip() if result.stdout.strip() else "(empty)"
        
        # Читаем содержимое исходного файла
        try:
            with open(stella_file, 'r') as f:
                input_code = f.read()
        except:
            input_code = ""
        
        if strip_logs:
            stderr_clean = strip_debug_logs(result.stderr)
            stderr = stderr_clean
        
        return TestOutput(
            stdout=stdout,
            stderr=stderr,
            exit_code=result.returncode,
            input_code=input_code
        )
    except subprocess.TimeoutExpired:
        return TestOutput(
            stdout="",
            stderr="TIMEOUT: typechecker execution exceeded 10 seconds",
            exit_code=-1
        )
    except Exception as e:
        return TestOutput(
            stdout="",
            stderr=f"ERROR: {str(e)}",
            exit_code=-1
        )


def load_golden_file(golden_path: str) -> TestOutput:
    """
    Загружает ожидаемый результат из золотого файла.
    
    Формат:
    STDOUT:
    (содержимое)
    
    STDERR:
    (содержимое)
    
    EXIT_CODE: 0
    """
    if not os.path.exists(golden_path):
        return None
    
    with open(golden_path, 'r') as f:
        content = f.read()
    
    sections = {}
    in_input = False
    in_stdout = False
    in_stderr = False
    input_lines = []
    stdout_lines = []
    stderr_lines = []
    
    for line in content.split('\n'):
        if line.startswith('INPUT:'):
            in_input = True
            in_stdout = False
            in_stderr = False
        elif line.startswith('STDOUT:'):
            in_input = False
            in_stdout = True
            in_stderr = False
        elif line.startswith('STDERR:'):
            in_input = False
            in_stdout = False
            in_stderr = True
        elif line.startswith('EXIT_CODE:'):
            in_input = False
            in_stdout = False
            in_stderr = False
            sections['exit_code'] = int(line.split(':', 1)[1].strip())
        elif in_input:
            input_lines.append(line)
        elif in_stdout:
            stdout_lines.append(line)
        elif in_stderr:
            stderr_lines.append(line)
    
    # Убираем пустые строки в начале и конце
    input_content = '\n'.join(input_lines).strip()
    stdout_content = '\n'.join(stdout_lines).strip()
    stderr_content = '\n'.join(stderr_lines).strip()
    
    return TestOutput(
        stdout=stdout_content,
        stderr=stderr_content,
        exit_code=sections.get('exit_code', 0),
        input_code=input_content
    )


def save_golden_file(golden_path: str, output: TestOutput) -> None:
    """Сохраняет результат в золотой файл."""
    input_code = output.input_code if output.input_code.strip() else "(empty)"
    stdout = output.stdout if output.stdout.strip() else "(empty)"
    stderr = output.stderr if output.stderr.strip() else "(empty)"
    
    content = f"""INPUT:
{input_code}

STDOUT:
{stdout}

STDERR:
{stderr}

EXIT_CODE: {output.exit_code}
"""
    with open(golden_path, 'w') as f:
        f.write(content)


def format_diff(expected: str, actual: str) -> str:
    """Форматирует разницу между ожидаемым и фактическим результатом."""
    return f"""
Expected:
{expected}

Actual:
{actual}
"""


def compare_outputs(expected: TestOutput, actual: TestOutput) -> Tuple[bool, str]:
    """
    Сравнивает ожидаемый и фактический результаты.
    
    Returns:
        (passed, error_message)
    """
    if expected.stdout != actual.stdout:
        return False, f"STDOUT mismatch:{format_diff(expected.stdout, actual.stdout)}"
    
    if expected.stderr != actual.stderr:
        return False, f"STDERR mismatch:{format_diff(expected.stderr, actual.stderr)}"
    
    if expected.exit_code != actual.exit_code:
        return False, f"EXIT_CODE mismatch: expected {expected.exit_code}, got {actual.exit_code}"
    
    return True, ""


def run_tests(
    typechecker_binary: str,
    stella_dir: str,
    golden_dir: str,
    update_golden: bool = False,
    filter_pattern: str = None
) -> int:
    """
    Запускает все тесты.
    
    Args:
        typechecker_binary: Путь к бинарнику типчекера
        stella_dir: Директория со .stella файлами
        golden_dir: Директория с .golden файлами
        update_golden: Обновить ли золотые файлы
        filter_pattern: Паттерн для фильтрации тестов (например: "simple")
    
    Returns:
        0 если все тесты прошли, 1 если есть ошибки
    """
    stella_files = sorted(Path(stella_dir).glob('**/*.stella'))
    
    if filter_pattern:
        stella_files = [f for f in stella_files if filter_pattern in str(f.relative_to(stella_dir))]
    
    if not stella_files:
        print(f"❌ Не найдено .stella файлов в {stella_dir}")
        return 1
    
    passed = 0
    failed = 0
    updated = 0
    
    print(f"🧪 Запуск {len(stella_files)} тестов...\n")
    
    for stella_file in stella_files:
        # Сохраняем относительный путь для организации golden файлов
        rel_path = stella_file.relative_to(stella_dir)
        rel_dir = rel_path.parent
        test_name = rel_path.stem
        
        # Создаем путь к golden файлу с сохранением структуры подпапок
        golden_subdir = os.path.join(golden_dir, str(rel_dir))
        golden_file = os.path.join(golden_subdir, f"{test_name}.golden")
        
        # Запускаем типчекер
        actual = run_typechecker(typechecker_binary, str(stella_file))
        
        if update_golden:
            # Обновляем золотой файл
            os.makedirs(golden_subdir, exist_ok=True)
            save_golden_file(golden_file, actual)
            display_name = str(rel_path.with_suffix(''))
            print(f"📝 {display_name}: обновлен")
            updated += 1
        else:
            # Сравниваем с золотым файлом
            expected = load_golden_file(golden_file)
            display_name = str(rel_path.with_suffix(''))
            
            if expected is None:
                print(f"⚠️  {display_name}: золотой файл не найден. Создан новый.")
                os.makedirs(golden_subdir, exist_ok=True)
                save_golden_file(golden_file, actual)
                updated += 1
            else:
                passed_test, error_msg = compare_outputs(expected, actual)
                
                if passed_test:
                    print(f"✅ {display_name}: OK")
                    passed += 1
                else:
                    print(f"❌ {display_name}: FAILED")
                    print(error_msg)
                    failed += 1
    
    print(f"\n{'='*50}")
    print(f"Результат: {passed} пройдено, {failed} ошибок", end="")
    if updated > 0:
        print(f", {updated} обновлено", end="")
    print()
    
    return 0 if failed == 0 else 1


def main():
    parser = argparse.ArgumentParser(
        description="Golden Test Framework для Stella Typechecker"
    )
    
    parser.add_argument(
        "--binary",
        default="./build/typechecker/cli/typechecker",
        help="Путь к бинарнику типчекера (default: ./build/typechecker/cli/typechecker)"
    )
    
    parser.add_argument(
        "--stella-dir",
        default="./stella-examples",
        help="Директория со .stella файлами (default: ./stella-examples)"
    )
    
    parser.add_argument(
        "--golden-dir",
        default="./tests/golden",
        help="Директория с .golden файлами (default: ./tests/golden)"
    )
    
    parser.add_argument(
        "--update",
        action="store_true",
        help="Обновить все золотые файлы (используйте с осторожностью!)"
    )
    
    parser.add_argument(
        "--filter",
        type=str,
        help="Фильтр для тестов (например: 'simple' будет запускать только simple.stella)"
    )
    
    args = parser.parse_args()
    
    # Проверяем наличие бинарника
    if not os.path.exists(args.binary):
        print(f"❌ Типчекер не найден: {args.binary}")
        print("Пожалуйста, сначала соберите проект:")
        print("  cd /path/to/project && cmake --build build")
        return 1
    
    # Проверяем наличие директории со стелла файлами
    if not os.path.isdir(args.stella_dir):
        print(f"❌ Директория со Stella файлами не найдена: {args.stella_dir}")
        return 1
    
    return run_tests(
        args.binary,
        args.stella_dir,
        args.golden_dir,
        update_golden=args.update,
        filter_pattern=args.filter
    )


if __name__ == "__main__":
    sys.exit(main())
