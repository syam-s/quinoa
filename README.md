## What is Quinoa?

<img src="https://quinoacomputing.github.io/quinoa/quinoa.svg" align="right" width="25%" background=transparent>
Quinoa is a set of computational tools that enables research and numerical analysis in fluid dynamics. At this time it is a test-bed to experiment with various algorithms using fully asynchronous runtime systems.

## Organization

Currently, Quinoa consists of the following tools:
  - [<B>inciter</B>](https://quinoacomputing.github.io/quinoa/inciter_doc.html) - Solve a PDE on an unstructured mesh using overdecomposition
  - [<B>walker</B>](https://quinoacomputing.github.io/quinoa/walker_doc.html) - Random walker for stochastic differential equations
  - [<B>rngtest</B>](https://quinoacomputing.github.io/quinoa/rngtest_doc.html) - Random number generator test suite
  - [<B>unittest</B>](https://quinoacomputing.github.io/quinoa/unittest_doc.html) - Unit test suite
  - [<B>meshconv</B>](https://quinoacomputing.github.io/quinoa/meshconv_doc.html) - Mesh file converter

## Goals

  - [<B>Designed for the exascale era</B>](https://quinoacomputing.github.io/quinoa/why.html#exascale)
  - [<B>Verified and proven to be correct</B>](https://quinoacomputing.github.io/quinoa/why.html#correct)
  - [<B>Optimized for performance, power, and reliability</B>](https://quinoacomputing.github.io/quinoa/why.html#optimized)
  - [<B>Advanced computer science outsourced to experts</B>](https://quinoacomputing.github.io/quinoa/why.html#outsource)
  - [<B>Using a language that can cope with complexity</B>](https://quinoacomputing.github.io/quinoa/why.html#language)
  - [<B>Highly-valued programmer productivity</B>](https://quinoacomputing.github.io/quinoa/why.html#productivity)
  - [<B>User and developer friendly</B>](https://quinoacomputing.github.io/quinoa/why.html#friendly)
  - [<B>Well documented</B>](https://quinoacomputing.github.io/quinoa/why.html#documented)
  - [<B>Fun to work on</B>](https://quinoacomputing.github.io/quinoa/why.html#fun)

## License

See the [license](https://github.com/quinoacomputing/quinoa/blob/master/LICENSE)

## Authors

Jozsef Bakosi (jbakosi@lanl.gov)

## Build

### 1. Install prerequisites

- Debian/Ubuntu linux: (line 1: required, line 2: recommended)

   ```
   apt-get install cmake gfortran gcc g++ openmpi-bin libopenmpi-dev
   apt-get install gmsh libpugixml-dev libpstreams-dev libboost-all-dev liblapack-dev liblapacke-dev libhdf5-dev libhdf5-openmpi-dev libhypre-dev
   ```

- Mac OS X: (line 1: required, line 2: recommended)

   ```
   port install cmake openmpi-clang38 && port select clang mp-clang-3.8 && port select mpi openmpi-clang38-fortran
   port install gmsh pugixml boost hdf5 +hl +openmpi hypre +openmpi
   ```

### 2. Clone, build third-party libraries, build & test

   ```
   git clone https://github.com/quinoacomputing/quinoa.git; cd quinoa
   mkdir tpl/build; cd tpl/build; cmake ..; make; cd -
   mkdir build; cd build; cmake ../src; make; ../script/run_tests.sh
   ```

   - All executables will be in <tt>quinoa/build/Main</tt>
   - Browse the [documentation](http://quinoacomputing.github.io/quinoa/index.html)
