CPSRKSEC
=======

This is a C library to use in Branch-and-Cut algorithms for cycle problems. It provides shrinking strategies to simplify the support graph and exact separation algorithms for the subcycle elimination separation problem.

A Cycle Problem (CP) is an optimization problem whose solution is a simple cycle. The most well-known CP is the Travelling Salesman Problem (TSP), whose solution is a Hamiltonian cycle. Other cycle problems are the so-called TSP with Profits, such us the Orienteering Problem (OP). The solutions of these problems are cycles not necessarily Hamiltonian. There are many problems that can be considered a CP.

Instructions to build
---------------------

The library uses GNU Autotools to generate the Makefiles.
```sh
sudo apt-get install libtool m4
```

Obtain the source code by:
```sh
git clone https://github.com/gkobeaga/cpsrksec
cd cpsrksec
```

Use GNU Autotools to generate the *configure* scricpt:
```sh
./autogen.sh
```

To build and install the binary type:
```sh
mkdir -p build && cd build
../configure
make
make install
```

How to use it
-------------
For a use of case of the cpsrksec library, see the [example](example/) folder.

Reproducibility
---------------
We provide in the [experiments](exp/) folder the code used to run the experiments.

Acknowledgments
---------------
The implementation of the shrinking strategies, the Hong separation algorithms for the Subcycle Elimination Problem, the push-relabel algorithm for Maximum Flow Problem, the Gomory-Hu tree construction and the hashing functions have been inspired in the [Concorde](http://www.math.uwaterloo.ca/tsp/concorde.html) solver for the TSP.
