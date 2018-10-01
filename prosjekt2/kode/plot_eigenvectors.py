import os
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import sys

def remove_file(filename):
    if os.path.exists(filename):
        os.system("rm " + filename)

def make_plot(ax, problem, n, omega_r, interact, which_to_plot):

    #os.system("./main.exe {} {} {}".format(n, problem, omega_r))
    os.system("./main.exe {} {} 1 {} {}".format(n, problem, omega_r, interact))

    filename = "../data/jacobi_eig{}_{}.dat".format(problem, n)
    data = np.loadtxt(filename)
    remove_file(filename)
    x = np.linspace(1./n,1-1./n,n)
    eigenvalues = data[:,0]
    eigenvectors = data[:,1:]
    inds = eigenvalues.argsort()
    sorted = eigenvectors[inds]

    if problem in [1,2]:
        normalised = sorted*sorted*n/10.
    else:
        normalised = sorted*sorted*n #FLAGG

    for i in which_to_plot:
        ax.plot(x, normalised,label=r"n={}, $\omega_r = {}$, interact = {}".format(n, omega_r, interact))

fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)

"""
for j in np.arange(10,201,10):
    make_plot(ax, 2, j, 0.01,[0])
"""
omegalist = [0.01, 0.5, 1.0, 5.0]

make_plot(ax, 2, 200, 0.01, 0, [0])
make_plot(ax, 2, 200, 0.01, 1, [0])
ax.legend()
plt.show()
