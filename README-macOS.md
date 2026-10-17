# eScorerDOCs dla macOS — 1.7.0

Natywna aplikacja dla Apple Silicon (M1/M2/M3/M4 i nowszych), macOS 13 lub nowszy.
Docelowy komputer: MacBook Air M1 (2020). Główne okno ma 480 × 536 punktów,
tak jak dotychczasowa wersja Windows. macOS sam obsługuje ekrany Retina.

## Jak zbudować przez GitHub — bez instalowania narzędzi na swoim Macu

1. Rozpakuj paczkę źródeł.
2. Otwórz https://github.com/urkarol/eScorerDOCs.
3. Wybierz **Add file → Upload files**. Przeciągnij zawartość folderu
   `eScorerDOCs`, nie sam folder i nie ZIP. Pliki z obecnej wersji zostaną
   uzupełnione o `src/macos`, `scripts`, `README-macOS.md` i konfigurację Actions.
4. Upewnij się, że trafił też plik **`.github/workflows/build-macos.yml`**.
   Folder `.github` jest ukryty w Finderze; pokaż go skrótem **⌘⇧.**.
   Jeśli nie został przesłany, otwórz **Add file → Create new file**, wpisz
   nazwę `.github/workflows/build-macos.yml`, wklej zawartość tego pliku i zapisz.
5. Zatwierdź przyciskiem **Commit changes** na gałęzi `main`.
6. Otwórz **Actions → macOS – eScorerDOCs**. Budowanie uruchomi się automatycznie.
   Można też wybrać **Run workflow**.
7. Po zielonym zakończeniu otwórz wykonanie i pobierz z **Artifacts**
   `eScorerDOCs-macOS-M1-M2-M3-M4`.
8. Rozpakuj pobrany artefakt, a następnie znajdujący się w nim ZIP aplikacji.
   Otrzymasz **eScorerDOCs.app**. Przenieś ją np. do folderu Aplikacje i uruchom.

Do uruchamiania gotowej aplikacji nie są potrzebne Python, Homebrew ani Xcode.
`.app` jest pojedynczym pakietem aplikacji rozpoznawanym przez Finder.
Wewnątrz ma swój plik wykonawczy, ikonę i metadane. Nie jest plikiem `.exe`.

## Pierwsze uruchomienie pobranej aplikacji

Skrypt tworzy podpis lokalny (ad-hoc). Nie zastępuje on podpisu Developer ID ani
notaryzacji Apple. System może dlatego poprosić o zgodę na uruchomienie.
Po próbie uruchomienia sprawdź **Ustawienia systemowe → Prywatność i ochrona →
Otwórz mimo to**, o ile taka opcja jest dostępna. Nie wyłączaj globalnie ochrony.
Do dystrybucji bez tego komunikatu potrzebny jest podpis Developer ID i notaryzacja.

## Co robi aplikacja

- Automatycznie ustawia dzisiejszą datę; datę można poprawić.
- Wybór rozgrywek ogranicza listy zespołów według tych samych reguł co w Windows.
- Przycisk **Zespoły…** otwiera edytor nazw, trzyznakowych kodów i przypisania
  do ligi. Można dodawać i usuwać drużyny oraz importować/eksportować listę UTF-8.
- **Format nazw…** pozwala przeciągać elementy, edytować separatory i niezależnie
  ustawić maskę daty dla dokumentów oraz folderu, np. `dd.MM.yyyy`, `yyyyMMdd`.
  `yyyy` oznacza rok, `MM` miesiąc, `dd` dzień; pojedyncze `M` i `d` pomijają zero.
- **Generuj PDF** tworzy puste A4: PE, PL, RM1, RM2, VC1, VC2, SD i SR.
- Domyślny folder meczu: `DATA_SKRÓT LIGI_NUMER MECZU_GOSPODARZ-GOŚĆ`
  (przykład: `2026-10-10_TLM_030_BEL-GDA`).
- Domyślny plik: `2026-10-10_TLM-030_PE_BEL-GDA.pdf`.
- Ikony: zamiana drużyn na środku, otwarcie folderu, kartki na ustawienie.
- Kartki R5 korzystają z dokładnie tego samego szablonu i generatora co Windows:
  wyśrodkowane skróty, usunięte wystające dolne kreski, 10 kartek na jednej stronie.
- Z numerem meczu kartki zapisują się w folderze meczu jako `BEL-R5.pdf` i
  `GDA-R5.pdf`, po czym otwierają w domyślnym czytniku PDF.
- Bez numeru i bez folderu zapisu aplikacja tworzy pliki w systemowym katalogu
  tymczasowym i otwiera własne dwa podglądy PDFKit z przyciskiem **Drukuj…** (⌘P).
  Zamknięcie każdego podglądu usuwa jego PDF; zamknięcie ostatniego usuwa katalog.
  Zamknięcie głównego okna pozostawia otwarte podglądy. Zakończenie programu przez
  ⌘Q również usuwa pliki tymczasowe. Awaria systemu lub wymuszone zabicie procesu
  może pozostawić katalog tymczasowy.
- Program nie nadpisuje istniejących dokumentów.

## Ustawienia

Zespoły i formaty są zapisywane atomowo w:
`~/Library/Application Support/eScorerDOCs/teams.txt` oraz `formats.txt`.
Folder i ostatnio wybrane drużyny/rozgrywki są przechowywane w preferencjach macOS.
Pliki `teams.txt` i `formats.txt` mają ten sam format tekstowy UTF-8 co dane
obsługiwane przez wspólną logikę Windows; można je zachować jako kopię ustawień.

## Budowanie na własnym Macu

Jednorazowo zainstaluj narzędzia Apple, jeśli jeszcze ich nie ma:

```sh
xcode-select --install
```

W Terminalu, w katalogu repozytorium:

```sh
bash scripts/test-common.sh
bash scripts/build-macos.sh arm64
bash scripts/test-macos.sh
```

Wynik: `dist/eScorerDOCs-macOS-arm64.zip`.
Opcjonalnie `bash scripts/build-macos.sh x86_64` buduje osobną wersję dla Intela;
workflow jest ustawiony na Apple Silicon.

## Zakres sprawdzenia tej paczki

Na Linuxie przeszły rzeczywiste testy wspólnej logiki: 54 kody zespołów i 8 filtrów,
walidacja, zapisy i ochrona przed nadpisaniem, polskie znaki, 720 układów nazw
dokumentów, 120 układów nazw folderów, maski dat i generowanie R5.
Skrypty mają poprawną składnię, a metadane aplikacji i YAML zostały sprawdzone.

W środowisku przygotowania paczki nie ma macOS ani SDK Apple. **Nie wykonano
jeszcze kompilacji ani testów interfejsu na macOS**. Workflow najpierw kompiluje
aplikację, następnie uruchamia test integracyjny z prawdziwym AppKit/PDFKit:
rozmiar okna, filtry zespołów, Unicode, puste PDF-y, podglądy tymczasowe,
usuwanie po zamknięciu i utworzenie operacji drukowania. Artefakt jest udostępniany
tylko po powodzeniu wszystkich kroków. Test nie drukuje fizycznie na drukarce.
Ostateczny wygląd i drukowanie należy jeszcze sprawdzić na docelowym Macu.

## Układ projektu

- `src/main.cpp` i pozostałe pliki Win32 — dotychczasowa wersja Windows.
- `src/core.hpp`, `naming.hpp`, `batch.hpp`, `lineup.hpp`, `lineup_data.hpp` —
  wspólne reguły, zapis i generator PDF.
- `src/macos/main.mm` — natywny interfejs AppKit w Objective-C++.
- `src/macos/icon.mm`, `Info.plist` — ikona i metadane aplikacji.
- `src/macos/test_macos.mm` — test integracyjny macOS.
- `scripts/` — kompilacja i testy.
- `.github/workflows/build-macos.yml` — budowanie na runnerze macOS arm64.

Szablon i osadzony font pochodzą z istniejącego projektu. Licencja fontu jest
dołączona w `DejaVu-LICENSE.txt` i kopiowana do gotowej aplikacji.

## Dokumentacja użytych mechanizmów

- [Runner macOS arm64](https://docs.github.com/en/actions/reference/runners/github-hosted-runners).
- [Podgląd i skalowanie PDFKit](https://developer.apple.com/documentation/pdfkit/pdfview/autoscales).
- [Pierwsze uruchomienie aplikacji pobranej z internetu](https://support.apple.com/102445).
