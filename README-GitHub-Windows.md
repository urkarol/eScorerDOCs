# eScorerDOCs 1.6.2 — Windows 10/11 x64

Dodano pusty plik **SK-10** do standardowego kompletu:
PE, PL, RM1, RM2, VC1, VC2, SD, SR, SK-10.
Jest tworzony w tym samym folderze i zgodnie z ustawionym formatem nazw.
Przykład: `2026-10-17_TLM-030_SK-10_BEL-GDA.pdf`.

## Wgranie do repozytorium i pobranie EXE

1. Rozpakuj paczkę źródeł.
2. Otwórz https://github.com/urkarol/eScorerDOCs.
3. Wybierz **Add file → Upload files** i przeciągnij **zawartość** folderu
   `eScorerDOCs`, nie sam folder ani archiwum ZIP. Zastąp istniejące pliki nowymi.
4. Upewnij się, że został wgrany również
   **`.github/workflows/build-windows.yml`**. W razie potrzeby użyj
   **Add file → Create new file**, wpisz tę ścieżkę i wklej treść pliku YML.
5. Zatwierdź zmiany przyciskiem **Commit changes**.
6. Wejdź w **Actions → Windows – eScorerDOCs**. Budowanie uruchamia się po zapisie
   zmian na `main` lub `master`. Możesz też użyć **Run workflow**.
7. Po zielonym zakończeniu otwórz wykonanie i pobierz z sekcji **Artifacts**
   paczkę **eScorerDOCs-Windows-x64**.
8. Rozpakuj pobrany ZIP i uruchom **eScorerDOCs.exe**. Instalacja nie jest potrzebna.

Artefakt zawiera jeden EXE oraz plik z jego sumą SHA-256. Jest udostępniany po
powodzeniu testów i budowania. Kompilator jest instalowany na serwerze GitHuba;
użytkownik gotowego programu nie musi go instalować.

## Co zostało zachowane

Główne okno, przyciski, ustawienia, edycja zespołów, format daty, nazwy folderów,
kartki R5 i usuwanie ich plików tymczasowych korzystają z dotychczasowego kodu.
Nowy dokument jest pustym A4, tak jak PE i pozostałe dokumenty meczowe.
Zapisane wcześniej ustawienia nie wymagają migracji.

## Budowanie lokalnie

W środowisku **MSYS2 MINGW64** z pakietem `mingw-w64-x86_64-gcc`:

```sh
bash scripts/test-common.sh
bash scripts/build-windows.sh
bash scripts/test-windows.sh
```

Wynik: `dist/eScorerDOCs.exe`.
Skrypt używa statycznego linkowania bibliotek kompilatora i systemowych bibliotek
Windows. Nie jest potrzebny osobny plik DLL z biblioteką C++.

## Testy

- 54 kody zespołów, osiem filtrów rozgrywek i 72 nazwy dokumentów, w tym SK-10.
- 720 układów nazw plików i 120 układów nazw folderów, formaty dat i separatory.
- Rzeczywisty zapis PDF, odmowa nadpisania i wycofywanie nieudanego zapisu.
- Generator R5 i nazwy `SKRÓT-R5.pdf`.
- Na Windows: systemowe renderowanie R5, usuwanie po zamknięciu uchwytu oraz
  zapis uruchomiony przez właściwy plik EXE (`--self-test`).

Przygotowana lokalnie paczka jest sprawdzana kompilacją i testami logiki na Linuxie.
Testy natywne Windows uruchamia workflow; interfejs należy sprawdzić na Windows.

Dokumentacja: [MSYS2 w CI](https://www.msys2.org/docs/ci/).
