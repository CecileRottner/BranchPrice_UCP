import sys
import matplotlib.pyplot as plt
import csv
import pandas
import numpy as np

df = pandas.read_csv(str(sys.argv[1]) + ".csv")

df = df[df.n == int(sys.argv[2])]

df = df[df['T'] == int(sys.argv[3])]


df = df[df.threshold <= float(sys.argv[4])]


df = df.groupby('threshold', as_index=False).mean()


fig, ax = plt.subplots()

df.plot(ax=ax, x='threshold', y='pricingTime')

plt.savefig("moyennes/" + str(sys.argv[1]) + "_" + str(sys.argv[2]) + "_" + str(sys.argv[3]) + "_" + str(sys.argv[4]) + ".png")