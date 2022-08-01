import sys
import matplotlib.pyplot as plt
import csv
import pandas
import numpy as np
from os.path import exists

nom = str(sys.argv[1]) + "_" + str(sys.argv[2])

df = pandas.read_csv("profondeur/" + nom + "_1" + ".csv")

for i in range(1,11):
    if (exists("profondeur/" + nom + "_" + str(i) + ".csv")):
        df = pandas.concat([df, pandas.read_csv("profondeur/" + nom + "_" + str(i) + ".csv")])


fig, ax = plt.subplots()

df = df[df.iter > 0.5]

df = df[df.profondeur > 0.5]

df = df.groupby('profondeur').mean()

df.plot(y = 'iter')

plt.savefig("profondeur/plots/moyennegenerale_" + str(sys.argv[3]) + "_" + nom + ".png")