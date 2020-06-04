CPSRKSEC
=======

[![DOI](https://zenodo.org/badge/259394517.svg)](https://zenodo.org/badge/latestdoi/259394517)
[![arXiv](http://img.shields.io/badge/cs.DS-arXiv%3A2004.14574-B31B1B.svg)](https://arxiv.org/abs/2004.14574)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://github.com/gkobeaga/cpsrksec/blob/master/LICENSE)

This is a C library for Branch-and-Cut algorithms that deal with cycle problems. It provides shrinking strategies to simplify the support graph and exact separation algorithms for the subcycle elimination separation problem.

A Cycle Problem (CP) is an optimization problem whose solution is a simple cycle. The Travelling Salesman Problem (TSP), whose solutions are Hamiltonian cycles, is the most well-known CP. Other cycle problems are the so-called TSP with Profits, such us the Orienteering Problem (OP), but there are many problems that can be considered a CP.

This library has been developed to perform the experiments reported in the paper "[On Solving Cycle Problems with Branch-and-Cut: Extending Shrinking and Exact Subcycle Elimination Separation Algorithms](https://arxiv.org/pdf/2004.14574.pdf)" by G. Kobeaga, M. Merino and J.A Lozano.

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

Use GNU Autotools to generate the *configure* script:
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
For a use case of the cpsrksec library, see the [example](example/) folder.

Reproducibility
---------------
The code used to run the experiments is provided in the [experiments](exp/) folder.

Citation
---------
Currently the paper is in revision process. Meanwhile, please use the following citation when using this library or comparing against it:

<pre>
@misc{kobeaga2020,
    title  = {On Solving Cycle Problems with Branch-and-Cut:
              Extending Shrinking and Exact Subcycle Elimination Separation Algorithms},
    author = {Gorka Kobeaga and Mar\'ia Merino and Jose A. Lozano},
    year   = {2020},
    eprint = {2004.14574},
    archivePrefix={arXiv},
    primaryClass={cs.DS}
}
</pre>

Acknowledgments
---------------
We gratefully acknowledge the authors of the TSP solver [Concorde](http://www.math.uwaterloo.ca/tsp/concorde.html) for making their code available to the public, since it has been the working basis of our implementations of the shrinking strategies, the Hong separation algorithms for the Subcycle Elimination Problem, the push-relabel algorithm for Maximum Flow Problem, the Gomory-Hu tree construction and the hashing functions.

