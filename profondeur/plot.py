import sys
import matplotlib.pyplot as plt
import csv
import pandas
import numpy as np

nom = str(sys.argv[1]) + "_" + str(sys.argv[2]) + "_" + str(sys.argv[3])

df = pandas.read_csv("profondeur/" + nom + ".csv")

fig, ax = plt.subplots()

df = df[df.iter > 0.5]

df = df[df.profondeur > 0.5]

df.plot(x = 'profondeur', y = 'iter', kind = 'scatter')

plt.savefig("profondeur/plots/scatter/" + str(sys.argv[4]) + "_" + nom + ".png")

df = df.groupby('profondeur').mean()

df.plot(y = 'iter')

plt.savefig("profondeur/plots/moyenne/" + str(sys.argv[4]) + "_" + nom + ".png")