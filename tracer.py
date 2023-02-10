import sys
import matplotlib.pyplot as plt
import csv
import pandas
import numpy as np

df = pandas.read_csv("colonnes.csv")

fig, ax = plt.subplots()

df.Demande.plot(ax=ax)
df.colsT.plot(ax=ax, secondary_y=True)

plt.savefig("plots/juxtaposition/colonnes_" + str(sys.argv[1]) + "_" + str(sys.argv[2]) + "_" + str(sys.argv[3]) + "_" + str(sys.argv[4]) + ".png")



fig, ax = plt.subplots()

df['diff'] = df.Demande - df.Demande.shift(1)
df.plot.scatter(ax=ax, x='diff', y='colsT')

plt.savefig("plots/correlation1/colonnes_" + str(sys.argv[1]) + "_" + str(sys.argv[2]) + "_" + str(sys.argv[3]) + "_" + str(sys.argv[4]) + ".png")


fig, ax = plt.subplots()

df['diff'] = df.Demande.shift(-1) - df.Demande 
df.plot.scatter(ax=ax, x='diff', y='colsT')

plt.savefig("plots/correlation2/colonnes_" + str(sys.argv[1]) + "_" + str(sys.argv[2]) + "_" + str(sys.argv[3]) + "_" + str(sys.argv[4]) + ".png")


fig, ax = plt.subplots()

df['diff'] = df.Demande.shift(-1) + df.Demande.shift(1) - 2*df.Demande 
df.plot.scatter(ax=ax, x='diff', y='colsT')

plt.savefig("plots/correlation3/colonnes_" + str(sys.argv[1]) + "_" + str(sys.argv[2]) + "_" + str(sys.argv[3]) + "_" + str(sys.argv[4]) + ".png")



fig, ax = plt.subplots()

df.plot.scatter(ax=ax, x='Demande', y='colsT')

plt.savefig("plots/correlation0/colonnes_" + str(sys.argv[1]) + "_" + str(sys.argv[2]) + "_" + str(sys.argv[3]) + "_" + str(sys.argv[4]) + ".png")
