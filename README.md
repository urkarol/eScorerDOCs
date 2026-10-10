# eScorerDOCs, wersja 1.6.1

Program: eScorerDOCs.exe. Jeden plik dla Windows 10/11 x64, bez instalacji i bez dodatkowych bibliotek. Uruchom dwuklikiem.

## Dokumenty meczowe

1. Wybierz datę (przy uruchomieniu jest ustawiana na aktualną datę komputera).
2. Wpisz numer meczu, np. 30 — w nazwach będzie 030.
3. Wybierz rozgrywki, gospodarza i gościa.
4. Wybierz folder zapisu.
5. Sprawdź podgląd nazw i kliknij „Generuj PDF”.

Powstają dokumenty PE, PL, RM1, RM2, VC1, VC2, SD i SR. Każdy jest poprawnym plikiem PDF 1.4 z jedną pustą stroną A4.
Przykład dokumentu: 2026-10-09_TLM-030_PE_BDM-JAS.pdf

W wybranym folderze zapisu program tworzy podfolder meczu. Jego domyślna nazwa ma format DATA_LIGA_NUMER_GOSPODARZ-GOŚĆ, np. 2026-10-09_TLM_030_BDM-JAS. Wszystkie dokumenty meczu powstają wewnątrz tego podfolderu. Przycisk „Otwórz folder” otwiera folder bieżącego meczu, jeśli już istnieje, a w przeciwnym razie folder bazowy.

Program nie nadpisuje istniejących plików. Jeżeli którykolwiek docelowy plik już istnieje, nie zapisuje całego kompletu. W razie błędu zapisu próbuje usunąć wyłącznie pliki utworzone podczas bieżącego generowania.

Rozgrywki, folder i lista zespołów są zapamiętywane w profilu użytkownika Windows. Data przy następnym uruchomieniu znów jest dzisiejsza, numer meczu należy wpisać ponownie. Zapisane ustawienia poprzedniej wersji programu są zachowane. Kopiowanie samego EXE na inny komputer nie przenosi ustawień i zmienionej listy zespołów.

## Kartki na ustawienie — wersja 1.6

Przycisk z ikoną kartki obok „Generuj PDF” tworzy dwa osobne pliki: `BEL-R5.pdf` i `GDA-R5.pdf` (skrót według wybranych drużyn). Po najechaniu myszą pokazuje podpowiedź. Każdy plik zawiera jedną stronę A4 poziomo z 10 kartkami (sety 1–5 w dwóch rzędach). Skrót PLS jest wyśrodkowany w obu osiach w prostokącie, a dwie krótkie kreski zostały usunięte. Pozostałe pola pozostają puste.

- **Z numerem meczu:** wypełnij dane i wybierz folder zapisu. Oba PDF-y powstają w folderze meczu według ustawionego formatu folderu i daty, a następnie otwierają się w domyślnym czytniku PDF Windows. Pliki pozostają na dysku. Nazwy kartek zawsze mają postać `SKRÓT-R5.pdf` i nie zależą od edytora nazw dokumentów. Program nie nadpisuje istniejących plików.
- **Bez numeru meczu:** wystarczy wybrać dwie różne drużyny. Folder zapisu nie jest potrzebny. Pliki powstają w osobnym podfolderze systemowego TEMP i otwierają się w dwóch własnych oknach podglądu PDF. Każde okno ma przyciski „Drukuj…” i „Zamknij” oraz obsługuje Ctrl+P i Escape. Zamknięcie danego podglądu usuwa jego PDF; zamknięcie ostatniego usuwa też podfolder. Podglądy pozostają otwarte po zamknięciu głównego programu.

Pliki tymczasowe są trzymane przez własny proces podglądu z flagą Windows DELETE_ON_CLOSE. Usuwają się również w razie zakończenia tego procesu przez system. Po takim awaryjnym zakończeniu może pozostać pusty podfolder. Usuwanie dotyczy tylko osobnych plików tymczasowych programu; dokumenty w folderach meczów nie są usuwane.

Podgląd odczytuje rzeczywisty wygenerowany PDF przez systemowy Windows.Data.Pdf i renderuje go do bitmapy 3508 × 2480 (około 300 dpi dla A4). Także drukowanie z podglądu wykorzystuje ten obraz. Nie potrzebuje dodatkowego czytnika, internetu, Pythona ani instalowania bibliotek. Wzór i czcionka są wbudowane w pojedynczy EXE.

Przyciski zamiany drużyn, folderu i kartek mają ikony oraz jednakowy rozmiar 30 × 30 przy skalowaniu 100%. Zamiana jest na środku pomiędzy polami drużyn. Każdy przycisk ma podpowiedź oraz nazwę dla czytników ekranu. Wymiary głównego okna nadal wynoszą 480 × 536.

Sprawdzono generowanie obu PDF-ów, format nazw SKRÓT-R5, oba tryby z numerem i bez, własne formaty folderów, błędne kody, identyczne drużyny, kolizje plików i wycofanie nieudanego zapisu. Przeszły też dotychczasowe testy nazw, dat, zespołów i PDF-ów. Obraz PDF jest identyczny z zatwierdzonym wyśrodkowanym wzorem. Gotowy plik jest Windows x64 i importuje tylko biblioteki systemowe. Podglądu, drukowania i usuwania po zamknięciu okna nie uruchamiano pod Windows w tym środowisku.

## Edycja listy zespołów

Kliknij „Zespoły…” obok pola rozgrywek.

- Wybierz ligę i zespół, następnie zmień jego nazwę lub trzyznakowy skrót PLS i kliknij „Zapisz zespół”.
- Aby przenieść zespół do innej ligi, zmień pole „Liga drużyny” i kliknij „Zapisz zespół”.
- Aby dodać zespół, kliknij „Nowy zespół”, wypełnij nazwę, skrót i ligę, następnie kliknij „Zapisz zespół”.
- Aby usunąć zespół, wybierz go na liście, kliknij „Usuń” i potwierdź w oknie programu.
- „Zamknij” wraca do głównego okna. Tekst wpisany w formularzu bez kliknięcia „Zapisz zespół” nie jest zapisywany.

Zmiany są widoczne od razu w głównym oknie i są zachowane po ponownym uruchomieniu. Kod drużyny musi być unikalny i zawierać trzy litery A-Z lub cyfry.

Początkowe listy zawierają 54 drużyny z tabeli użytkownika i korzystają ze skrótów PLS. Cztery ligi mają osobne listy. Puchar i Superpuchar pokazują odpowiednio drużyny z obu lig męskich albo obu kobiecych. Można również wpisać kod drużyny spoza listy z klawiatury w głównym oknie.

## Edycja formatów nazw

Kliknij „Format nazw…” nad podglądem. Wybierz „Nazwy dokumentów” albo „Nazwa folderu meczu”.

- Przeciągnij element na inne miejsce na liście. Możesz też użyć „W górę” i „W dół”.
- Wybierz element i zmień „Separator po wybranym elemencie”. Separatory są przypisane do przerw pomiędzy kolejnymi pozycjami; przesuwanie elementów nie przesuwa separatorów.
- Możesz użyć np. `_`, `-`, spacji lub pozostawić separator pusty.
- W polu „Format daty” wybierz gotowy układ albo wpisz własny. Ustawienie jest osobne dla nazw dokumentów i folderu. `yyyy` oznacza czterocyfrowy rok, `yy` dwucyfrowy rok, `MM` i `dd` miesiąc i dzień z zerem na początku, a `M` i `d` bez zera. Każdy składnik musi wystąpić raz. Możesz zmieniać ich kolejność i używać kropek, myślników, podkreśleń, spacji lub zapisu bez separatorów, np. `yyyyMMdd` → `20261009`, `dd.MM.yyyy` → `09.10.2026`.
- Podgląd aktualizuje się od razu. Gdy dane bieżącego meczu są niekompletne, używany jest przykładowy mecz.
- „Układ domyślny” przywraca domyślny format aktualnie wybranego typu nazwy.
- „Zapisz” zachowuje oba formaty. „Anuluj” zamyka edytor bez zastosowania zmian.

Rozszerzenie .pdf jest dodawane automatycznie na końcu dokumentu. Wszystkie elementy pozostają w nazwie, a użytkownik zmienia ich kolejność. Niedozwolone w Windows znaki separatorów są odrzucane. Formaty są zapamiętywane po zamknięciu programu.

## Zmiany w wersji 1.3

Dodano podfoldery meczów i edytor formatów nazw z przeciąganiem, edycją separatorów oraz podglądem. Szerokość i wysokość głównego okna pozostają takie same.

## Zmiany w wersji 1.4

Dodano edycję formatu daty z gotowymi układami i możliwością wpisania własnego. Osobne ustawienia dla dokumentów i folderu są zapamiętywane. Dotychczas zapisane układy i separatory są zachowane. Wybór daty meczu w głównym oknie nadal pokazuje datę w stałym formacie `yyyy-MM-dd`; format nazw zmienia tylko zapis w folderach i plikach. „Układ domyślny” przywraca również domyślny format daty wybranego typu nazwy.

## Zmiany w wersji 1.2

Nazwa programu i pliku to eScorerDOCs. Ikona przedstawia dokumenty. Główne okno ma 480 × 536 jednostek przy skalowaniu 100%; rozwijane listy pokazują pełne nazwy zespołów. Dodano edytor listy zespołów.

## Źródła i kompilacja

Folder src zawiera kod C++17, zasoby, manifest i testy. Do uruchamiania gotowego EXE kompilator nie jest potrzebny.
Przykład kompilacji dla Windows za pomocą MinGW-w64, z folderu src:

    windres -i app.rc -O coff -o app.res
    g++ main.cpp app.res -std=c++17 -Os -s -static -static-libgcc -static-libstdc++ -municode -mwindows -o eScorerDOCs.exe -lcomctl32 -lshell32 -lole32 -ladvapi32 -lgdi32 -luser32 -lruntimeobject -lshcore -lshlwapi -lwindowscodecs -luuid -lcomdlg32

## Weryfikacja

Przeszły testy nazw, filtrowania i zapisu PDF-ów oraz testy zapisu/odczytu danych zespołów, polskich znaków, zmian nazw i kodów, przenoszenia zespołu do innej ligi, dodawania i usuwania zespołów, pustej listy oraz odrzucania nieprawidłowych i powielonych kodów. PDF został sprawdzony parserem i renderowaniem. W wersji 1.3 przeszły także testy wszystkich 720 układów nazw dokumentów, 120 układów nazw folderów, zmiany separatorów, zachowywania formatów, tworzenia podfolderu i ochrony istniejących plików.
W wersji 1.4 sprawdzono także formaty daty, daty przestępne, różne formaty folderów i dokumentów, zapamiętywanie i migrację ustawień poprzedniej wersji oraz generowanie kompletu w folderze z nowym formatem daty.
Plik jest skompilowany dla Windows x64 i importuje tylko systemowe biblioteki Windows. Uruchomienie okna aplikacji Windows w tym środowisku nie było możliwe.

## Poprawka 1.3.1

Pole separatora w edytorze formatów nazw jest krótsze, a jego wysokość wynika z wymiarów aktualnie używanej czcionki.

## Wbudowany wzór kartek

`src/lineup_source.pdf` jest wzorem dostarczonym przez użytkownika. `src/build_lineup_template.py` usuwa etykiety i krótkie kreski oraz buduje `lineup_template.pdf` i `lineup_data.hpp`. Przebudowanie wzoru wymaga wyłącznie na etapie przygotowania: Python, PyMuPDF, fontTools i czcionki DejaVu Sans. Kompilacja C++ korzysta z gotowego `lineup_data.hpp` i nie wymaga Pythona. Licencja czcionki jest w DejaVu-LICENSE.txt oraz wewnątrz zasobów EXE.

## Test podglądu pod Windows

`src/test_preview.cpp` jest testem integracyjnym dla Windows 10/11. Sprawdza systemowe renderowanie rzeczywistego PDF i usuwanie plików tymczasowych po zamknięciu uchwytu, zachowując drugi plik. Test został skompilowany, ale nie uruchomiony w środowisku Linux. Przykładowa kompilacja z folderu src:

    g++ test_preview.cpp -std=c++17 -O2 -static -municode -o test_preview.exe -lole32 -lruntimeobject -lshcore -lshlwapi -lwindowscodecs -luuid -lcomdlg32 -lgdi32 -luser32

## Poprawka 1.6.1

Dodano SD i SR do kompletu pustych dokumentów meczowych i do podglądu nazw. Powstaje osiem dokumentów. Oba nowe rodzaje korzystają z ustawionych formatów nazw i daty oraz tego samego folderu meczu. Wymiary okna i obsługa kartek R5 pozostają bez zmian.
