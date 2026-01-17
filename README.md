# Kompresor Huffmana

Program do kompresji i dekompresji plikow algorytmem Huffmana.

## Kompilacja

```
build.bat
```

Lub recznie:
```
g++ -std=c++11 -o huffman.exe huffman.cpp
```

## Uzycie

```
huffman.exe
```

Menu:
- **[1] Kompresuj** - plik tekstowy → plik .huff
- **[2] Dekompresuj** - plik .huff → plik tekstowy
- **[3] Test kolejki** - demonstracja kolejki priorytetowej

## Struktura kodu

| Funkcja | Opis |
|---------|------|
| `naprawGore/Dol` | Naprawa kopca w gore/dol |
| `dodaj` | Wstaw element do kopca |
| `pobierzMin` | Pobierz minimum z kopca |
| `budujDrzewo` | Zbuduj drzewo Huffmana |
| `generujKody` | Wygeneruj kody binarne |
| `kompresuj` | Kompresja pliku |
| `dekompresuj` | Dekompresja pliku |

## Format pliku .huff

```
ASCII:czestotliwosc;ASCII:czestotliwosc;...
zakodowane_bity
```

## Zlozonosc

| Operacja | Zlozonosc |
|----------|-----------|
| Wstaw/Pobierz z kopca | O(log n) |
| Budowanie drzewa | O(n log n) |
| Kompresja/Dekompresja | O(m) |

---
Projekt AiSD
