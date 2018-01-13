import matplotlib.pyplot as plt

seq = [67.64, 182.38, 749.96, 3174.93, 11533.70]
vect = [15.15, 47.54, 200.07, 917.46, 2605.71]
openmp = [32.91, 99.257, 420.86, 1940.30, 5942.29]
openmpvect = [16.22, 48.84, 219.12, 1078.77, 3508.84]
cuda = [112.15, 127.29, 182.89, 398.95, 1305.37]
cudafull = [111.83, 114.73, 135.18, 190.76, 415.54]

t = [2.4, 6.2, 24.9, 96, 414.7]

plt.xscale('log')
p1 = plt.plot(t, seq, 'ro', label = 'sekwencyjna')
p2 = plt.plot(t, vect, 'go', label = 'zwektoryzowana')
p2 = plt.plot(t, openmp, 'bo', label = 'OpenMP')
p2 = plt.plot(t, openmpvect, 'yo', label = 'OpenMP zwektoryzowana')
p2 = plt.plot(t, cuda, 'co', label = 'CUDA (dct)')
p2 = plt.plot(t, cudafull, 'ko', label = 'CUDA (YCbCr + dct)')
plt.legend()
plt.xlabel('Rozmiar bitmapy [MB]')
plt.ylabel('Czas kompresji [ms]')
plt.show()
