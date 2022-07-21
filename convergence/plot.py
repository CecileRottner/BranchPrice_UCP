import sys
import matplotlib.pyplot as plt
import csv
import pandas
import numpy as np

nom = str(sys.argv[1]) + "_" + str(sys.argv[2]) + "_" + str(sys.argv[3])

df = pandas.read_csv("convergence/" + nom + ".csv")

fig, ax = plt.subplots()

upper = df.dualValue.max()

ax.set_ylim([-0.5 * upper, 1.1*upper])

df.lowerBound.plot(ax=ax, x='iter')
df.dualValue.plot(ax=ax, x='iter')

plt.savefig("convergence/plots/" + str(sys.argv[4]) + "_" + nom + ".png")