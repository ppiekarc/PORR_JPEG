# PORR_JPEG
Kompresja Jpeg realizowana na przedmiot PORR

## Porówywanie ze wzorcem
lena_ref.jpg jest "wzorcowo" skompresowanym lena.bmp przy użyciu obecnych współczynników. Do sprawdzenia czy nowy JPG różni się od wzorcowego
można użyć:

```commandline
cmp -bl lena_ref.jpg <nowy jpeg>.jpg | wc -l
```

## Budowanie

```commandline
mkdir build
cmake -G "Unix Makefiles" ..
make
```

Utworzą się następujące binarki:
```text
jpeg_compressor - kompresja i zapisywanie wynikowego JPEG
jpeg_sequential_benchmark - benchmark sekwencyjnej wersji kompresora
jpeg_parallel_benchmark - benchmark zrównoleglonej (pthreads) wersji kompresora
jpeg_vectorized_benchmark - benchmark zwektoryzowanej (-03, -ftree-vectorize) wersji kompresora
jpeg_parallel_and_vectorized_benchmark - benchmark zrównoleglonej i zwektoryzowanej wersji kompresora
```
