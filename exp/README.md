Experiments
===========
In this directory you will find the scripts needed to reproduce the results reported in the paper.

Run the experiment
-------------------

In order to run the experiments, enter the following

```sh
cd exp
./run-exp.sh
```

Process the experiment results
-------------------

The results are processed using R. First, if needed, install the dependencies:
```sh
install.packages(c("tables", "plyr", "dplyr", "tidyr"))
```
To run the script to process the results:
```sh
Rscript results.R
```
Once the R script has finished, you will see a directory named *tables* where the tables reported in the paper are stored.
