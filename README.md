# PORR_JPEG
Kompresja Jpeg realizowana na przedmiot PORR (wersja CUDA)

## Porówywanie ze wzorcem
lena_ref.jpg jest "wzorcowo" skompresowanym lena.bmp przy użyciu obecnych współczynników. Do sprawdzenia czy nowy JPG różni się od wzorcowego
można użyć:

```commandline
cmp -bl lena_ref.jpg <nowy jpeg>.jpg | wc -l
```

## Budowanie (Linux)

```commandline
./build.sh
```

Skrypt nie jest jeszcze uniwersalny i może wymagać modyfikacji wybranych stałych/ścieżek w zależności od systemu.
