# PORR_JPEG
Kompresja Jpeg realizowana na przedmiot PORR

## Porówywanie ze wzorcem
lena_ref.jpg jest "wzorcowo" skompresowanym lena.bmp przy użyciu obecnych współczynników. Do sprawdzenia czy nowy JPG różni się od wzorcowego
można użyć:

cmp -bl lena_ref.jpg <nowy jpeg>.jpg | wc -l
